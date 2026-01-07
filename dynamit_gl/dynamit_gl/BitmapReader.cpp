#include "pch.h"
#include "BitmapReader.h"

//suggested by claude to awoid Windows SDK header conflict. 
//    The error occurs because MIDL_INTERFACE is a macro that requires COM headers,
//    but they're not included before GdiplusImaging.h gets pulled in.
//Solution1: ensure objbase.h is included before gdiplus.h
//#include <objbase.h>    // or <windows.h> — must come first
//#include <gdiplus.h>
//Solution2: WIN32_LEAN_AND_MEAN should fix preorder issue
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>    // brings back COM definitions
#include <gdiplus.h>
//end suggested by claude
#include <iostream>
#include <vector>
#include "geometry.h" //API for automatic cleaner

wchar_t* towide(const char* imgPath)
{
	int sz = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, imgPath, -1, 0, 0);
	if (sz == 0) { ShowLastError(std::cout); return 0; }
	wchar_t* wstr = (new wchar_t[sz + 1ll]);
	sz = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, imgPath, -1, wstr, sz);
	if (sz == 0) { ShowLastError(std::cout); return 0; }
	return wstr;
}

auto localdeleterw = [](wchar_t* a) {LocalFree(a); };
auto localdeletera = [](char* a)    {LocalFree(a); };

std::wostream& ShowLastError(std::wostream& os)
{
	wchar_t* pBuffer = 0;
	int ret = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, (LPWSTR)&pBuffer, 0, 0);
	if (!(ret && pBuffer))
	{
		os << L"failed to read error";
		return os;
	}
	std::unique_ptr<wchar_t, decltype(localdeleterw)> ptr (pBuffer, localdeleterw);
	os << pBuffer << std::flush;
	return os;
}

std::ostream& ShowLastError(std::ostream& os)
{
	LPSTR pBuffer = NULL;
	int ret = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, (LPSTR)&pBuffer, 0, 0);
	if (!(ret && pBuffer))
	{
		os << "failed to read error";
		return os;
	}
	std::unique_ptr<char, decltype(localdeletera)> ptr(pBuffer, localdeletera);
	os << pBuffer << std::flush;
	return os;
}
void LastErrorBox()
{
	LPWSTR pBuffer = NULL;
	int ret = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, (LPWSTR)&pBuffer, 0, 0);
	if (!(ret && pBuffer))
	{
		MessageBoxW(0, pBuffer, L"failed to read error", 0);
		return;
	}
	std::unique_ptr<wchar_t, decltype(localdeleterw)> ptr(pBuffer, localdeleterw);

	MessageBoxW(0, pBuffer, L"", 0);
}

int HeigthMapFromBmp(const char* imgPath, std::vector<std::vector<float>>& heights, int step)
{
	std::unique_ptr<wchar_t[]> wstr(towide(imgPath));
	return HeigthMapFromBmp(wstr.get(), heights, step);
}

int HeigthMapFromBmp(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step)
{
	HDC hdcMem = CreateCompatibleDC(0);
	HBITMAP hBMP = (HBITMAP)LoadImage(NULL, imgPath, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (!hBMP)
	{
		LastErrorBox();
		return -1;
	}

	HGDIOBJ hOld = SelectObject(hdcMem, hBMP);

	BITMAP bitmap;
	GetObject(hBMP, sizeof(bitmap), &bitmap);
	heights.resize(bitmap.bmHeight);

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = bitmap.bmWidth;
	bmi.biHeight = -bitmap.bmHeight;
	bmi.biCompression = BI_RGB;

	std::unique_ptr<unsigned char[]> screenData = std::make_unique<unsigned char[]>(4ll * bitmap.bmWidth * bitmap.bmHeight);

	GetDIBits(hdcMem, hBMP, 0, bitmap.bmHeight, screenData.get(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	for (long long i = 0; i < bitmap.bmHeight; i++)
	{
		std::vector<float>& widths = heights[i];
		widths.resize(bitmap.bmWidth);
		for (int j = 0; j < bitmap.bmWidth; j++)
			widths[j] = (float)screenData[4 * (i * bitmap.bmWidth + j)] / 255.0f;
	}

	SelectObject(hdcMem, hOld);
	DeleteDC(hdcMem);
	DeleteObject(hBMP);
	return 0;
}

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")



int HeigthMapFromImg(const char* imgPath, std::vector<std::vector<float>>& heights, int step)
{
	std::unique_ptr<wchar_t> wstr(towide(imgPath));
	return HeigthMapFromImg(wstr.get(), heights, step);
}

int HeightMapFromImgApi(const char* imgPath, std::vector<std::vector<float>>& heights, int step)
{
	std::unique_ptr<wchar_t> wstr (towide(imgPath));
	return HeightMapFromImgApi(wstr.get(), heights, step);
}

////Workaround the unique_ptr to call Gdiplus::GdiplusShutdown
////very unclean
//auto dtr = [](ULONG_PTR* a) {Gdiplus::GdiplusShutdown(*a); };
//std::unique_ptr<ULONG_PTR, decltype(dtr) > ptr(&gdiplusToken, dtr);

////Just shut down, clean but not automatic
//Gdiplus::GdiplusShutdown(gdiplusToken);

//
//using cleaner class, clean but implemented own
template<class T, auto deleter> struct cleaner
{
	T resource;
	cleaner(const cleaner&) = delete;
	cleaner(cleaner&&) = delete;
	void operator=(const cleaner&) = delete;
	cleaner* operator &() = delete;
	~cleaner() { deleter(resource); }
};
int HeigthMapFromImg(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	cleaner<decltype(gdiplusToken), Gdiplus::GdiplusShutdown> c { gdiplusToken };

	int x = HeightMapFromImgApi(imgPath, heights, step);
	return x;
}
int HeigthMapFromImgFlat(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	cleaner<decltype(gdiplusToken), Gdiplus::GdiplusShutdown> c { gdiplusToken };

	int x = HeightMapFromImgApiFlat(imgPath, heights, step);
	return x;
}
int HeightMapFromImgApi(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step)
{
	Gdiplus::Bitmap image(imgPath);
	//particle.jpg //crate.jpg //airplane.png //heightmap.xx.bmp //heightmap.yy.bmp
	heights.resize(image.GetHeight() / step);
	
	if (step == 1)
	{
		int imgHeigth = image.GetHeight();
		int imgWidth = image.GetWidth();

		for (size_t i = 0; i < imgHeigth; i++)
		{
			std::vector<float>& widths = heights[i];
			//int width = image.GetWidth();
			widths.resize(imgWidth);
			Gdiplus::Color color;
			for (size_t j = 0; j < imgWidth; j++)
			{
				image.GetPixel(i, j, &color);
				widths[j] = (float)color.GetR() / 255.0f;
			}
		}
	}
	else
	{
		int imgHeigth = image.GetHeight() - (image.GetHeight() % step);
		int imgWidth = image.GetWidth() - (image.GetWidth() % step);
		for (size_t i = 0; i < imgHeigth; i += step)
		{
			std::vector<float>& widths = heights[i / step];
			//int width = image.GetWidth();
			widths.resize(imgWidth / step);
			Gdiplus::Color color;
			for (size_t j = 0; j < imgWidth; j += step)
			{
				image.GetPixel(i, j, &color);
				widths[j / step] = (float)color.GetR() / 255.0f;
			}
		}
	}
	return 0;
}


int HeightMapFromImgApiFlat(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step)
{
	Gdiplus::Bitmap image(imgPath);
	//particle.jpg //crate.jpg //airplane.png //heightmap.xx.bmp //heightmap.yy.bmp
	heights.resize(image.GetHeight() / step);

	if (step == 1)
	{
		int imgHeigth = image.GetHeight();
		int imgWidth = image.GetWidth();

		for (size_t i = 0; i < imgHeigth; i++)
		{
			std::vector<float>& widths = heights[i];
			//int width = image.GetWidth();
			widths.resize(imgWidth);
			Gdiplus::Color color;
			for (size_t j = 0; j < imgWidth; j++)
			{
				image.GetPixel(i, j, &color);
				widths[j] = (float)10.f / 255.0f;
			}
		}
	}
	else
	{
		int imgHeigth = image.GetHeight() - (image.GetHeight() % step);
		int imgWidth = image.GetWidth() - (image.GetWidth() % step);
		for (size_t i = 0; i < imgHeigth; i += step)
		{
			std::vector<float>& widths = heights[i / step];
			//int width = image.GetWidth();
			widths.resize(imgWidth / step);
			Gdiplus::Color color;
			for (size_t j = 0; j < imgWidth; j += step)
			{
				image.GetPixel(i, j, &color);
				widths[j / step] = (float)10.f / 255.0f;
			}
		}
	}
	return 0;
}