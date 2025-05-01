#define WIN32_LEAN_AND_MEAN
#define WINDOWS_IGNORE_PACKING_MISMATCH
#include <Windows.h>
#include <shellapi.h>
#include <Thumbcache.h>
#include <gdiplus.h>
#include <gdiplusflat.h>
#include "resource.h"
#pragma comment(lib, "Gdiplus.lib")

constexpr UUID CLSID_PapasBestSTLThumbnails = { 0x2e2f83c0, 0xd8, 0x4504, { 0xb8, 0x4a, 0x31, 0xd6, 0xa2, 0x9b, 0xfd, 0x80 } };

constexpr UUID CLSID_gdiPlusPngEncoder = { 0x557cf406, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e } };

static void print(HANDLE const channel, char const text[]) {
	auto const l = ::strlen(text);
	DWORD dummy;
	WriteFile(channel, text, DWORD(l), &dummy, nullptr);
}

static void printHexError(HANDLE const channel, unsigned int x) {
	char string[15];
	string[0] = 'E';
	string[1] = 'r';
	string[2] = 'r';
	string[3] = 'o';
	string[4] = 'r';
	string[5] = ' ';
	for (int i = 0; i < 8; ++i) {
		auto const hexDigit = x & 0xF;
		char c;
		if (hexDigit < 10) {
			c = char('0' + hexDigit);
		}
		else {
			c = char('A' + hexDigit);
		}
		string[13 - i] = c;
		x >>= 4;
	}
	string[14] = '\n';
	DWORD dummy;
	WriteFile(channel, string, sizeof string, &dummy, nullptr);
}

static int intFromDecimal(wchar_t const* digits) {
	int result = 0;
	while (L'0' <= *digits && *digits <= L'9') {
		result = 10 * result + int(*digits++ - '0');
	}
	if (L'\0' == *digits) {
		return result;
	}
	return -1;
}

static void printErrorAndExit(HANDLE const channel, char const message[], UINT const code) {
	print(channel, message);
	ExitProcess(code);
}

#	pragma comment(linker, "/ENTRY:crtMain")
void crtMain() {
	auto const stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	auto const stderr = GetStdHandle(STD_ERROR_HANDLE);

	wchar_t const* stlPath = nullptr;
	wchar_t const* outPath = nullptr;
	int             sidelength = 1024;
	int             numberOfParameters = 0;
	auto            parameters = CommandLineToArgvW(GetCommandLineW(), &numberOfParameters);

	print(stdout, "\n");

	if (parameters[0][0] != L'-') {
		--numberOfParameters;
		++parameters;
	}

	while (numberOfParameters > 0) {
		if (0 == wcscmp(parameters[0], L"-o")) {

			--numberOfParameters;
			++parameters;
			if (0 == numberOfParameters) {
				printErrorAndExit(stderr, "Usage error: Bad parameter; expecting -o <path>", 1);
			}
			--numberOfParameters;
			outPath = *parameters++;

		}
		else {

			if (nullptr == stlPath) {
				--numberOfParameters;
				stlPath = *parameters++;
			}
			else {
				printErrorAndExit(stderr, "Usage error: Bad parameter.", 1);
			}

		}
	}

	if (nullptr == stlPath) {
		print(stdout, "STLBitzBox - STLtoPNG\n"
					  "\n"
					  " Creates a .PNG image of a .STL file.\n"
			          " Uses Papa's Best STL Thumbnails for thumbnail support.\n"
			          "\n"
			          "  Usage: STLtoPNG <stl path> -o <png path>\n"
			          "\n"
			          "   -o Defaults to \"stl.png\".\n");
		ExitProcess(1);
	}

	if (nullptr == outPath) {
		outPath = L"stl.png";
	}

	auto isSuccessful = false;

	(void)CoInitialize(nullptr);

	print(stdout, "Loading .STL thumbnail handler ...  ");
	IThumbnailProvider* thumbnailProvider = nullptr;
	{
		auto const status = CoCreateInstance(CLSID_PapasBestSTLThumbnails, nullptr, CLSCTX_INPROC_SERVER, IID_IThumbnailProvider, reinterpret_cast<void**>(&thumbnailProvider));
		if (FAILED(status)) {
			printHexError(stderr, status);
		}
		else {
			print(stdout, " loaded handler!\n");
		}
	}

	if (nullptr != thumbnailProvider) {
		IInitializeWithFile* initializeWithFile = nullptr;
		{
			auto const status = thumbnailProvider->QueryInterface(IID_IInitializeWithFile, reinterpret_cast<void**>(&initializeWithFile));
			if (FAILED(status)) {
				printHexError(stderr, status);
			}
		}

		if (nullptr != initializeWithFile) {

			print(stdout, "Loading .STL ...                    ");
			auto stlLoaded = false;
			{
				auto const status = initializeWithFile->Initialize(stlPath, STGM_READ);
				if (FAILED(status)) {
					printHexError(stderr, status);
				}
				else {
					print(stdout, " loaded .STL!\n");
					stlLoaded = true;
				}
			}

			if (stlLoaded) {
				HBITMAP thumbnail = nullptr;
				WTS_ALPHATYPE alpha = WTSAT_UNKNOWN;

				{
					auto const status = thumbnailProvider->GetThumbnail(sidelength, &thumbnail, &alpha);
					if (FAILED(status)) {
						printHexError(stderr, status);
					}
				}

				if (nullptr != thumbnail && (WTSAT_RGB == alpha || WTSAT_ARGB == alpha)) {

					Gdiplus::GdiplusStartupInput input;
					ULONG_PTR gdiplusToken;
					auto const gdiPlusStatus = GdiplusStartup(&gdiplusToken, &input, nullptr);
					if (Gdiplus::Status::Ok == gdiPlusStatus) {
						Gdiplus::GpBitmap* bmp;
						auto const bmpStatus = Gdiplus::DllExports::GdipCreateBitmapFromHBITMAP(thumbnail, nullptr, &bmp);
						if (Gdiplus::Status::Ok == bmpStatus) {
							print(stdout, "Rendering .PNG ...                  ");
							auto const saveStatus = Gdiplus::DllExports::GdipSaveImageToFile(bmp, outPath, &CLSID_gdiPlusPngEncoder, nullptr);
							if (Gdiplus::Status::Ok == saveStatus) {
								print(stdout, " successfully rendered .PNG!\n");
								isSuccessful = true;

							}
							else {
								printHexError(stderr, saveStatus);
							}

						}
						else {
							printHexError(stderr, bmpStatus);
						}
					}
					else {
						printHexError(stderr, gdiPlusStatus);
					}

				}

			}
		}
	}

	ExitProcess(isSuccessful ? 0 : 1);
}