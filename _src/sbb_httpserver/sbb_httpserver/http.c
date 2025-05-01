#include "sbb_httpserver.h"
#include <errno.h>

#define LF                  (u_char) '\n'
#define CR                  (u_char) '\r'
#define CRLF                "\r\n"

typedef struct  
{
    char                *key;
    char                *value;
} request_fields_t;

typedef struct
{
    char                *method;
    char                *uri;
    char                *version;
    uint8_t             fields_count;
    request_fields_t    *fields;
} request_header_t;

static void CallbackAccept(event_t *ev);
static void CallbackRead(event_t *ev);
static void CallbackWrite(event_t *ev);

static int   RequestHeaderRead(event_t *ev, char **buf, int *size);
static int   RequestHeaderParse(char *data, request_header_t *header);
static void  RequestHeaderRelease(request_header_t *header);
static void  EventRelease(event_t *ev);
static event_data_t *EventDataCreate(const char *header, const char *html);
static event_data_t *EventDataCreateFP(const char *header, FILE *fp, int read_len, int total_len);
static void  EventDataRelease(event_t *ev);
static void  DecodeURI(char* uri);
static uint8_t IsHex(uint8_t x);
static char *LocalFileList(char *path);

static const char *ResponseFileType(char *file_name);
static const char *ResponseHeaderFormat();
static const char *ResponseBodyFormat();
static void ResponseFileIndex(event_t* ev, char* path);
static void ResponseSendFilePage(event_t *ev, char *file_name);
static void ResponseHTTP400(event_t *ev);
static void ResponseHTTP404(event_t *ev);
static void ResponseHTTP500(event_t *ev);
static void ResponseHTTP501(event_t *ev);
static void SendResponse(event_t *ev, char* title, char *status);

char* UTF8toANSI(char* str)
{
    wchar_t* u8toaresult;
    int u8toalen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    u8toaresult = (wchar_t*)malloc((u8toalen + 1) * sizeof(wchar_t));
    memset(u8toaresult, 0, (u8toalen + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)u8toaresult, u8toalen);
    wchar_t* uni = u8toaresult;

    char* utoaresult;
    int utoalen = WideCharToMultiByte(CP_ACP, 0, uni, -1, NULL, 0, NULL, NULL);
    utoaresult = (char*)malloc((utoalen + 1) * sizeof(char));
    memset(utoaresult, 0, (utoalen + 1) * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, uni, -1, utoaresult, utoalen, NULL, NULL);
    char* ansi = utoaresult;

    free(uni);
    return ansi;
}

char* ANSItoUTF8(char* str)
{
    wchar_t* atouresult;
    int atoulen = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    atouresult = (wchar_t*)malloc((atoulen + 1) * sizeof(wchar_t));
    memset(atouresult, 0, (atoulen + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)atouresult, atoulen);
    wchar_t* uni = atouresult;

    char* utou8result;
    int utou8len = WideCharToMultiByte(CP_UTF8, 0, uni, -1, NULL, 0, NULL, NULL);
    utou8result = (char*)malloc((utou8len + 1) * sizeof(char));
    memset(utou8result, 0, (utou8len + 1) * sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, uni, -1, utou8result, utou8len, NULL, NULL);
    char* utf8 = utou8result;

    free(uni);
    return utf8;
}

char* GetWorkingPath()
{
    static char root[MAX_PATH] = { 0 };
    uint32_t i;

    if (!root[0])
    {
        GetCurrentDirectoryA(MAX_PATH, root);
        for (i = 0; i < strlen(root); i++)
        {
            if (root[i] == '\\')
            {
                root[i] = '/';
            }
        }
        if (root[strlen(root) - 1] != '/')
        {
            root[strlen(root)] = '/';
        }
    }
    return root;
}

int HTTPServe(uint16_t *port)
{
    SOCKET fd;
    event_t ev = {0};

    LogToFile("%s:%d] - STLBitzBox HTTP server starting...", __FUNCTION__, __LINE__);
    NetInit();
    EventInit();
    NetListen(port, &fd);

    ev.fd = fd;
    ev.ip = htonl(INADDR_ANY);
    ev.type = EV_READ | EV_PERSIST;
    ev.callback = CallbackAccept;
    EventAdd(&ev);

    EventDispatch();

    closesocket(fd);
    EventUninit();
    NetCleanup();
    LogToFile("%s:%d] - STLBitzBox HTTP server stopping...", __FUNCTION__, __LINE__);
    return SUCC;
}

static void CallbackAccept(event_t *ev)
{
    SOCKET fd;
    struct in_addr addr;
    event_t ev_ = {0};

    if (SUCC == NetAccept(ev->fd, &addr, &fd))
    {   
        ev_.fd = fd;
        ev_.ip = addr.s_addr;
        ev_.type = EV_READ | EV_PERSIST;
        ev_.callback = CallbackRead;
        EventAdd(&ev_);

        LogToFile("%s:%d] - Callback accepted from client (IP: %s) connected on socket: %d", __FUNCTION__, __LINE__, inet_ntoa(addr), fd);
    }
    else
    {
        LogToFile("%s:%d] - Callback accept falire. Error: %d", __FUNCTION__, __LINE__, WSAGetLastError());
    }
}

static void CallbackRead(event_t *ev)
{
    char *buf = NULL;
    int   size;
    request_header_t header;
    int   i;
    int   content_length = 0;
    char *temp = NULL;
    char  file_path[MAX_PATH] = {0};

    if (ev->status == EV_IDLE)
    {
        if (SUCC != RequestHeaderRead(ev, &buf, &size))
        {
            ResponseHTTP400(ev);
            free(buf);
            return;
        }
        if (!buf)
            return;
        RequestHeaderParse(buf, &header);
        DecodeURI(header.uri);
        header.uri = UTF8toANSI(header.uri);

        LogToFile("%s:%d] - Callback read. Entry point received: uri=%s", __FUNCTION__, __LINE__, header.uri);
        if (strcmp(header.method, "GET") && strcmp(header.method, "POST"))
        {
            // 501 Not Implemented
            ResponseHTTP501(ev);
            RequestHeaderRelease(&header);
            free(buf);
            return;
        }
        else if (header.uri[strlen(header.uri) - 1] == '/' && strlen(header.uri) >= 2)
        {
            ResponseFileIndex(ev, header.uri + 1);
            free(buf);
            RequestHeaderRelease(&header);
            return;
        }
        else if (header.uri[strlen(header.uri)-1] == '/')
        {
            header.uri = "/index.html";
            header.uri = UTF8toANSI(header.uri);

            memset(file_path, 0, sizeof(file_path));
            memcpy(file_path, GetWorkingPath(), strlen(GetWorkingPath()));
            memcpy(file_path + strlen(file_path), header.uri + 1, strlen(header.uri + 1));

            ResponseSendFilePage(ev, file_path);
            free(buf);
            RequestHeaderRelease(&header);
            return;
        }
        else
        {
            memset(file_path, 0, sizeof(file_path));
            memcpy(file_path, GetWorkingPath(), strlen(GetWorkingPath()));
            memcpy(file_path+strlen(file_path), header.uri+1, strlen(header.uri+1));

            ResponseSendFilePage(ev, file_path);
            free(buf);
            RequestHeaderRelease(&header);
            return;
        }
    }
}

static void CallbackWrite(event_t *ev)
{
    if (!ev->data)
        return;

    if (ev->data->size != send(ev->fd, ev->data->data, ev->data->size, 0))
    {
        LogToFile("%s:%d] - Send response failed on socket: %d, Error: %d", __FUNCTION__, __LINE__, ev->fd, WSAGetLastError());
        shutdown(ev->fd, SD_SEND);
        EventDataRelease(ev);
        return;
    }
    //LogToFile("%s:%d] - Send response progress %d%% on socket: %d", __FUNCTION__, __LINE__, ev->data->total ? ev->data->offset*100/ev->data->total : 100, ev->fd);
    if (ev->data->total == ev->data->offset)
    {
        LogToFile("%s:%d] - Send response completed on socket: %d", __FUNCTION__, __LINE__, ev->fd);
        EventDataRelease(ev);
    }
    else
    {
        ResponseSendFilePage(ev, ev->data->file);
    }
}

static int RequestHeaderRead(event_t *ev, char **buf, int *size)
{
    char c;
    int len = 1;
    int idx = 0;
    int ret;

    while (TRUE)
    {
        ret = NetRead(ev->fd, &c, len);
        if (ret == DISC)
        {
            EventRelease(ev);
            return SUCC;
        }
        else if (ret == SUCC)
        {
            if (*buf == NULL)
            {
                *size = BUFFER_UNIT;
                *buf = (char*)malloc(*size);
                if (!(*buf))
                {
                    LogToFile("%s:%d] - CRITICAL ERROR! Memory allocation failure.", __FUNCTION__, __LINE__);
                    return FAIL;
                }
            }
            (*buf)[idx++] = c;
            if (idx >= *size - 1)
            {
                *size += BUFFER_UNIT;
                *buf = (char*)realloc(*buf, *size);
                if (!(*buf))
                {
                    LogToFile("%s:%d] - CRITICAL ERROR! Newly allocated memory failure.", __FUNCTION__, __LINE__);
                    return FAIL;
                }
            }
            if (idx >= 4 && (*buf)[idx - 1] == LF && (*buf)[idx - 2] == CR
                && (*buf)[idx - 3] == LF && (*buf)[idx - 4] == CR)
            {
                (*buf)[idx] = 0;
                return SUCC;
            }
        }
        else
        {
            LogToFile("%s:%d] - CRITICAL ERROR! Unknown or malformed header receive failure.", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }

    LogToFile("%s:%d] - CRITICAL ERROR! Header not found.", __FUNCTION__, __LINE__);
    return FAIL;
}

static int RequestHeaderParse(char *data, request_header_t *header)
{
#define move_next_line(x)   while (*x && *(x + 1) && *x != CR && *(x + 1) != LF) x++;
#define next_header_line(x) while (*x && *(x + 1) && *x != CR && *(x + 1) != LF) x++; *x=0;
#define next_header_word(x) while (*x && *x != ' ' && *x != ':' && *(x + 1) && *x != CR && *(x + 1) != LF) x++; *x=0;

    char *p = data;
    char *q;
    int idx = 0;

    memset(header, 0, sizeof(request_header_t));

    next_header_word(p);
    header->method = data;
    data = ++p;
    next_header_word(p);
    header->uri = data;
    data = ++p;
    next_header_word(p);
    header->version = data;
    next_header_line(p);
    data = ++p + 1;
    p++;
    q = p;

    while (*p)
    {
        move_next_line(p);
        data = ++p + 1;
        p++;
        header->fields_count++;
        if (*data && *(data + 1) && *data == CR && *(data + 1) == LF)
            break;
    }

    header->fields = (request_fields_t*)malloc(sizeof(request_fields_t)*header->fields_count);
    if (!header->fields)
    {
        LogToFile("%s:%d] - CRITICAL ERROR! Memory allocation failure.", __FUNCTION__, __LINE__);
        return FAIL;
    }

    data = p = q;
    while (*p)
    {
        next_header_word(p);
        header->fields[idx].key = data;
        data = ++p;
        if (*data == ' ')
        {
            data++;
            p++;
        }
        next_header_line(p);
        header->fields[idx++].value = data;
        data = ++p + 1;
        p++;
        if (*data && *(data + 1) && *data == CR && *(data + 1) == LF)
            break;
    }
    ASSERT(idx == header->fields_count);
    return SUCC;
}

static void RequestHeaderRelease(request_header_t *header)
{
    if (header->uri)
    {
        free(header->uri);
    }
    if (header->fields)
    {
        free(header->fields);
    }
}

static void EventRelease(event_t *ev)
{
    closesocket(ev->fd);
    EventDataRelease(ev);
    EventDelete(ev);
}

static event_data_t *EventDataCreate(const char *header, const char *html)
{
    event_data_t* ev_data = NULL;
    int header_length = 0;
    int html_length = 0;
    int data_length = 0;

    if (header)
        header_length = strlen(header);
    if (html)
        html_length = strlen(html);

    data_length = sizeof(event_data_t) - sizeof(char) + header_length + html_length;
    ev_data = (event_data_t*)malloc(data_length);
    if (!ev_data)
    {
        LogToFile("%s:%d] - CRITICAL ERROR! Memory allocation failure.", __FUNCTION__, __LINE__);
        return ev_data;
    }
    memset(ev_data, 0, data_length);
    ev_data->total = header_length + html_length;
    ev_data->offset = header_length + html_length;
    ev_data->size = header_length + html_length;
    if (header)
        memcpy(ev_data->data, header, header_length);
    if (html)
        memcpy(ev_data->data + header_length, html, html_length);
    return ev_data;
}

static event_data_t *EventDataCreateFP(const char *header, FILE *fp, int read_len, int total_len)
{
    event_data_t* ev_data = NULL;
    int header_length = 0;
    int data_length = 0;

    if (header)
        header_length = strlen(header);

    data_length = sizeof(event_data_t) - sizeof(char) + header_length + read_len;
    ev_data = (event_data_t*)malloc(data_length);
    if (!ev_data)
    {
        LogToFile("%s:%d] - CRITICAL ERROR! Memory allocation failure.", __FUNCTION__, __LINE__);
        return ev_data;
    }
    memset(ev_data, 0, data_length);
    ev_data->total = total_len;
    ev_data->size = read_len + header_length;
    if (header)
        memcpy(ev_data->data, header, header_length);
    if (read_len != fread(ev_data->data + header_length, 1, read_len, fp))
    {
        LogToFile("%s:%d] - CRITICAL ERROR! Failure reading file.", __FUNCTION__, __LINE__);
        free(ev_data);
        ev_data = NULL;
    }
    return ev_data;
}

static void EventDataRelease(event_t *ev)
{
    if (ev->data)
    {
        if (ev->data->fp)
        {
            fclose(ev->data->fp);
            ev->data->fp = NULL;
        }
        free(ev->data);
        ev->data = NULL;
    }
}

static void DecodeURI(char* uri)
{
    int len = strlen(uri);
    char *out = NULL;
    char *o = out;
    char *s = uri;
    char *end = uri + strlen(uri);
    int c;

    out = (char*)malloc(len+1);
    if (!out)
    {
        LogToFile("%s:%d] - CRITICAL ERROR! Memory allocation failure.", __FUNCTION__, __LINE__);
        return;
    }

    for (o = out; s <= end; o++)
    {
        c = *s++;
        if (c == '+')
        {
            c = ' ';
        }
        else if (c == '%' 
            && (!IsHex(*s++) || !IsHex(*s++) || !sscanf(s - 2, "%2x", &c)))
        {
            LogToFile("%s:%d] - Malformed URI: %s", __FUNCTION__, __LINE__, uri);
            free(out);
            return;
        }

        if (out)
        {
            *o = c;
        }
    }

    memcpy(uri, out, strlen(out));
    uri[strlen(out)] = 0;
    free(out);
}

static uint8_t IsHex(uint8_t x)
{
    return	(x >= '0' && x <= '9')	||
        (x >= 'a' && x <= 'f')	||
        (x >= 'A' && x <= 'F');
}

static char* LocalFileList(char *path)
{
    const char* format_dir = "<a href=\"%s/\">%s/</a>" CRLF;
    const char* format_file = "<a href=\"%s\">%s</a>";
    WIN32_FIND_DATAA FindFileData;
    HANDLE hFind;
    char filter[MAX_PATH] = {0};
    char *result = NULL;
    char line[BUFFER_UNIT] = {0};
    int line_length;
    int size = BUFFER_UNIT;
    int offset = 0;
    char *size_str = NULL;
    char *utf8 = NULL;
    int i;

    sprintf(filter, "%s*", path);

    hFind = FindFirstFileA(filter, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) 
    {
        LogToFile("%s:%d] - Invalid file handle. Error: %d", __FUNCTION__, __LINE__, GetLastError());
        return NULL;
    }
    do 
    {
        if (FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)
        {
            if (!result)
            {
                result = (char*)malloc(size);
            }
            utf8 = ANSItoUTF8(FindFileData.cFileName);
            sprintf(line, format_dir, utf8, utf8);
            line_length = strlen(line);
            line[line_length] = 0;
            if (offset+line_length > size-1)
            {
                size += BUFFER_UNIT;
                result = (char*)realloc(result, size);
            }
            memcpy(result+offset, line, line_length);
            offset += line_length;
            free(utf8);
        }
    } while (FindNextFileA(hFind, &FindFileData));
    FindClose(hFind);

    hFind = FindFirstFileA(filter, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) 
    {
        LogToFile("%s:%d] - Invalid file handle. Error: %d", __FUNCTION__, __LINE__, GetLastError());
        return NULL;
    }
    do 
    {
        if (!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes))
        {
            if (!result)
            {
                result = (char*)malloc(size);
            }
            utf8 = ANSItoUTF8(FindFileData.cFileName);
            sprintf(line, format_file, utf8, utf8);
            line_length = strlen(line);
            for (i=strlen(FindFileData.cFileName); i<60; i++)
            {
                line[line_length++] = ' ';
            }
            static char buf[16] = { 0 };
            memset(buf, 0, sizeof(buf));
            _itoa(FindFileData.nFileSizeLow, buf, 10);
            size_str = buf;

            memcpy(line+line_length, size_str, strlen(size_str));
            line_length += strlen(size_str);
            line[line_length++] = CR;
            line[line_length++] = LF;
            line[line_length] = 0;

            if (offset+line_length > size-1)
            {
                size += BUFFER_UNIT;
                result = (char*)realloc(result, size);
            }
            memcpy(result+offset, line, line_length);
            offset += line_length;
            free(utf8);
        }
    } while (FindNextFileA(hFind, &FindFileData));
    FindClose(hFind);
    result[offset] = 0;

    return result;
}

static const char *ResponseFileType(char *file_name)
{
    const request_fields_t content_type[] =     {
        { "html",   "text/html"                 },
        { "css",    "text/css"                  },
        { "txt",    "text/plain"                },
        { "log",    "text/plain"                },
        { "cpp",    "text/plain"                },
        { "c",      "text/plain"                },
        { "h",      "text/plain"                },
        { "js",     "application/x-javascript"  },
        { "png",    "application/x-png"         },
        { "jpg",    "image/jpeg"                },
        { "jpeg",   "image/jpeg"                },
        { "jpe",    "image/jpeg"                },
        { "gif",    "image/gif"                 },
        { "png",    "image/png"                 },
        { "ico",    "image/x-icon"              },
        { "doc",    "application/msword"        },
        { "docx",   "application/msword"        },
        { "ppt",    "application/x-ppt"         },
        { "pptx",   "application/x-ppt"         },
        { "xls",    "application/x-xls"         },
        { "xlsx",   "application/x-xls"         },
        { "mp4",    "video/mpeg4"               },
        { "mp3",    "audio/mp3"                 },
        { "stl",    "model/stl"                 },
        { "zip",    "application/zip"           }
    };

    char *ext = NULL;
    int i;
    int j;

    if (!file_name)
    {
        return "text/html";
    }

    for (i = strlen(file_name) - 1; i >= 0; i--)
    {
        if (file_name[i] == '.')
        {
            ext = file_name + i + 1;
            break;
        }
    }

    if (ext)
    {
        for (j=0; j<sizeof(content_type)/sizeof(content_type[0]); j++)
        {
            if (0 == strcmp(content_type[j].key, ext))
            {
                return content_type[j].value;
            }
        }
    }
    return "application/octet-stream";
}

static const char *ResponseHeaderFormat()
{
    const char *http_header_format =
        "HTTP/1.1 %s" CRLF
        "Content-Type: %s" CRLF
        "Content-Length: %d" CRLF
        CRLF;

    return http_header_format;
}

static const char *ResponseBodyFormat()
{
    const char *http_body_format =
        "<html>" CRLF
        "<head><title>%s</title></head>" CRLF
        "<body bgcolor=\"white\">" CRLF
        "<center><h1>%s</h1></center>" CRLF
        "</body></html>";

    return http_body_format;
}

static void ResponseFileIndex(event_t* ev, char* path)
{
    const char* html_format =
        "<html>" CRLF
        "<head>" CRLF
        "<meta charset=\"utf-8\">" CRLF
        "<title>STLBitzBox - Index of /%s</title>" CRLF
        "</head>" CRLF
        "<header style=\"font-family: 'Nunito'; font-weight: bold; background-color: #545a70; padding-left: 8px\">" CRLF
        "<h1 style=\"text-align: center; font-size: 36;\">STLBitzBox</h1>" CRLF
        "<h1>Index of:<br>/%s</h1>" CRLF
        "</header>" CRLF
        "<body style=\"font-family: 'Nunito'; font-size: 12; font-weight: bold; background-color: #9d9eab; padding-left: 16px\">" CRLF
        "<form action=\"/upload?path=%s\" method=\"post\" enctype=\"multipart/form-data\">" CRLF
        "<hr><pre>" CRLF
        "%s" CRLF
        "</body></html>";

    char header[BUFFER_UNIT] = { 0 };
    event_data_t* ev_data = NULL;
    int length;
    char* file_list = NULL;
    char* html = NULL;
    event_t ev_ = { 0 };
    char* utf8 = NULL;

    utf8 = ANSItoUTF8(path);
    file_list = LocalFileList(path);
    if (!file_list)
        return;

    length = strlen(html_format) + strlen(file_list) + (strlen(utf8) - strlen("%s")) * 3 + 1;
    html = (char*)malloc(length);
    if (!html)
    {
        LogToFile("%s:%d] - CRITICAL ERROR! Memory allocation failure.", __FUNCTION__, __LINE__);
        return;
    }
    sprintf(html, html_format, utf8, utf8, utf8, file_list);
    free(utf8);
    free(file_list);
    sprintf(header, ResponseHeaderFormat(), "200 OK", ResponseFileType(NULL), strlen(html));
    ev_data = EventDataCreate(header, html);
    free(html);

    ev_.fd = ev->fd;
    ev_.ip = ev->ip;
    ev_.type = EV_WRITE;
    ev_.param = ev->param;
    ev_.data = ev_data;
    ev_.callback = CallbackWrite;
    EventAdd(&ev_);
}

static void ResponseSendFilePage(event_t *ev, char *file_name)
{
    char header[BUFFER_UNIT] = { 0 };
    FILE* fp = NULL;
    int total;
    int len;
    event_data_t* ev_data = NULL;
    event_t ev_ = {0};

    fp = fopen(file_name, "rb");
    if (!fp)
    {
        LogToFile("%s:%d] - Failure opening file (%s). Error number: %d", __FUNCTION__, __LINE__, file_name, errno);
        EventDataRelease(ev);
        ResponseHTTP404(ev);
        goto end;
    }
    if (ev->data == NULL)
    {
        fseek(fp, 0, SEEK_END);
        total = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        len = total > BUFFER_UNIT ? BUFFER_UNIT : total;
        sprintf(header, ResponseHeaderFormat(), "200 OK", ResponseFileType(file_name), total);
        ev_data = EventDataCreateFP(header, fp, len, total);
        memcpy(ev_data->file, file_name, strlen(file_name));
        ev_data->offset += len;
    }
    else
    {
        fseek(fp, ev->data->offset, SEEK_SET);
        len = ev->data->total - ev->data->offset > BUFFER_UNIT ? BUFFER_UNIT : ev->data->total - ev->data->offset;
        ev_data = EventDataCreateFP(NULL, fp, len, ev->data->total);
        memcpy(ev_data->file, file_name, strlen(file_name));
        ev_data->offset = ev->data->offset + len;
        EventDataRelease(ev);
    }

    ev_.fd = ev->fd;
    ev_.ip = ev->ip;
    ev_.type = EV_WRITE;
    ev_.param = ev->param;
    ev_.data = ev_data;
    ev_.callback = CallbackWrite;
    EventAdd(&ev_);

end:
    if (fp)
        fclose(fp);
}

static void ResponseHTTP400(event_t *ev)
{
    SendResponse(ev, "400 Bad Request", NULL);
}

static void ResponseHTTP404(event_t *ev)
{
    SendResponse(ev, "404 Not Found", NULL);
}

static void ResponseHTTP500(event_t *ev)
{
    SendResponse(ev, "500 Internal Server Error", NULL);
}

static void ResponseHTTP501(event_t *ev)
{
    SendResponse(ev, "501 Not Implemented", NULL);
}

static void SendResponse(event_t *ev, char *title, char *status)
{
    char header[BUFFER_UNIT] = { 0 };
    char body[BUFFER_UNIT]   = { 0 };
    event_data_t* ev_data = NULL;
    event_t ev_ = {0};

    sprintf(body, ResponseBodyFormat(), title, title);
    sprintf(header, ResponseHeaderFormat(), status ? status : title, ResponseFileType(NULL), strlen(body));
    ev_data = EventDataCreate(header, body);

    ev_.fd = ev->fd;
    ev_.ip = ev->ip;
    ev_.type = EV_WRITE;
    ev_.param = ev->param;
    ev_.data = ev_data;
    ev_.callback = CallbackWrite;
    EventAdd(&ev_);
}