#pragma once
#include "pch.h"
#include <vector>
#include <iostream>
//char* to wchar_t*, free with delete[] or use auto pointers to avoid memory leaks
wchar_t* towide(const char* imgPath);

//Last Error utilities
//Shows a message box
extern void LastErrorBox();
//Writes Error in an output stream, such as cout or any other one
std::wostream& ShowLastError(std::wostream& os);
std::ostream&  ShowLastError(std::ostream& os);

//can load only 4 byte color bmp
//uses GDI
int HeigthMapFromBmp(const char*    imgPath, std::vector<std::vector<float>>& heights, int step = 1);
int HeigthMapFromBmp(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step = 1);

// can load any type of bmp, jpg, png, gif...
// initializes GDI+ and shuts it down when finished
int HeigthMapFromImg(const char*    imgPath, std::vector<std::vector<float>>& heights, int step = 1);
int HeigthMapFromImg(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step = 1);
int HeigthMapFromImgFlat(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step = 1);

// Same as HeigthMapFromImg but supposes GDI+ is already initialized
// Call it if GDI+ is already used and initialized somewhere else
// internally the HeigthMapFromImg functions calls HeightMapFromImgApi
int HeightMapFromImgApi(const char*    imgPath, std::vector<std::vector<float>>& heights, int step = 1);
int HeightMapFromImgApi(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step = 1);
//int HeightMapFromImgApiFlat(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step = 1);
int HeightMapFromImgApiFlat(const wchar_t* imgPath, std::vector<std::vector<float>>& heights, int step = 1);
