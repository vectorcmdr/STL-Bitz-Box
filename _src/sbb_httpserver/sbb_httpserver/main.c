#include "sbb_httpserver.h"

#pragma comment(lib, "Dbghelp.lib")

int WINAPI WinMain(_In_       HINSTANCE hInstance,
                    _In_opt_  HINSTANCE hPrevInstance,
                    _In_      LPWSTR    lpCmdLine,
                    _In_      int       nCmdShow)
{
    UINT16 port = 80;

    system("@mkdir logs >nul 2>&1");
    
    HTTPServe(&port);
    system("pause");
    return 0;
}

void LogToFile(const char* fmt, ...)
{
    char buffer[BUFFER_UNIT] = { 0 };
    va_list argslist;

    va_start(argslist, fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, argslist);
    va_end(argslist);

    const char* strstart = NULL;
    const char* filename = NULL;
    FILE* filepath = NULL;
    strstart = "[LOG";

    static char filenamefw[MAX_PATH] = { 0 };
    static struct tm last = { 0 };

    time_t t = time(0);
    struct tm now;
    now = *localtime(&t);

    if (last.tm_year == now.tm_year
        && last.tm_mon == now.tm_mon
        && last.tm_mday == now.tm_mday)
    {
        filename = filenamefw;
    }
    memcpy(&last, &now, sizeof(struct tm));
    memset(filenamefw, 0, sizeof(filenamefw));
    memcpy(filenamefw, GetWorkingPath(), strlen(GetWorkingPath()));
    strftime(filenamefw + strlen(GetWorkingPath()), sizeof(filenamefw) - strlen(GetWorkingPath()), "logs\\STLBitzBoxServer_%Y-%m-%d.log", &now);

    filename = filenamefw;

    filepath = fopen(filename, "ab");
    if (filepath)
    {
        fwrite(strstart, 1, strlen(strstart), filepath);
        fwrite(" ", 1, 1, filepath);
        fwrite(buffer, 1, strlen(buffer), filepath);
        fwrite("\n", 1, 1, filepath);
        fclose(filepath);
    }

    printf("%s %s\n", strstart, buffer);
}

ret_code_t NetInit()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    return SUCC;
}

ret_code_t NetCleanup()
{
    WSACleanup();
    return SUCC;
}

ret_code_t NetListen(uint16_t* port, SOCKET* fd)
{
    struct sockaddr_in address;
    BOOL option = TRUE;
    int rtn;

    *fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == *fd)
    {
        LogToFile("%s:%d] - Socket creation failed. Error: %d", __FUNCTION__, __LINE__, WSAGetLastError());
        exit(1);
    }

    rtn = setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
    if (SOCKET_ERROR == rtn)
    {
        closesocket(*fd);
        LogToFile("%s:%d] - Set socket option failed. Error: %d", __FUNCTION__, __LINE__, WSAGetLastError());
        exit(1);
    }

    do
    {
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(*port);
        address.sin_addr.s_addr = htonl(INADDR_ANY);

        rtn = bind(*fd, (struct sockaddr*)&address, sizeof(address));
        if (SOCKET_ERROR == rtn)
        {
            (*port)++;
        }
        else
        {
            break;
        }
    } while (*port < (uint16_t)(-1));

    rtn = listen(*fd, SOMAXCONN);
    if (SOCKET_ERROR == rtn)
    {
        closesocket(*fd);
        LogToFile("%s:%d] - Socket listen failure. Error: %d", __FUNCTION__, __LINE__, WSAGetLastError());
        exit(1);
    }

    LogToFile("%s:%d] - Server running on http://%s:%d/ on socket: %d", __FUNCTION__, __LINE__, inet_ntoa(address.sin_addr), *port, *fd);
    return SUCC;
}

ret_code_t NetAccept(SOCKET sfd, struct in_addr* addr, SOCKET* cfd)
{
    struct sockaddr_in addr_;
    int len = sizeof(addr_);

    *cfd = accept(sfd, (struct sockaddr*)&addr_, &len);
    if (INVALID_SOCKET == *cfd)
    {

        ("%s:%d] - Socket accept failure. Error: %d", __FUNCTION__, __LINE__, WSAGetLastError());
        return FAIL;
    }
    *addr = addr_.sin_addr;
    LogToFile("%s:%d] - Client (IP: %s) connected on socket: %d", __FUNCTION__, __LINE__, inet_ntoa(addr_.sin_addr), *cfd);
    return SUCC;
}

ret_code_t NetRead(SOCKET fd, char* buf, int32_t size)
{
    int ret = 0;
    int offset = 0;

    while (TRUE)
    {
        ret = recv(fd, buf + offset, size - offset, 0);
        if (ret == SOCKET_ERROR)
        {
            LogToFile("%s:%d] - Socket receive failure on socket: %d Error: %d", __FUNCTION__, __LINE__, fd, WSAGetLastError());
            return DISC;
        }
        else if (ret == 0)
        {
            LogToFile("%s:%d] - Disconnect on socket: %d", __FUNCTION__, __LINE__, fd);
            return DISC;
        }
        else if (ret + offset == size)
        {
            return SUCC;
        }
        else if (ret + offset < size)
        {
            offset += ret;
            LogToFile("%s:%d] - Continuing to read on socket: %d process:%d%%", __FUNCTION__, __LINE__, fd, offset * 100 / size);
            continue;
        }
        else
        {
            LogToFile("%s:%d] - Unknown failure on socket: %d", __FUNCTION__, __LINE__, fd);
            break;
        }
    }
    return FAIL;
}

ret_code_t NetWrite(SOCKET fd, void* buf, uint32_t size)
{
    if (size != send(fd, buf, size, 0))
    {
        LogToFile("%s:%d] - Send failure on socket: %d. Error: %d", __FUNCTION__, __LINE__, fd, WSAGetLastError());
        return FAIL;
    }

    return SUCC;
}