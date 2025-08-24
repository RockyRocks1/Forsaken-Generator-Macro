#include <stdexcept>
#include "PixelReader.h"

PixelReader::PixelReader() {
    this->m_hdcScreen = GetDC(NULL);
    if (this->m_hdcScreen == NULL)
        std::runtime_error("Failed to get screen device context.");
    this->m_hdcMemory = CreateCompatibleDC(this->m_hdcScreen);
    if (this->m_hdcMemory == NULL) {
        ReleaseDC(NULL, this->m_hdcScreen);
        std::runtime_error("Failed to get memory context.");
    }

    this->m_hBitmap = NULL;
    this->m_pPixels = nullptr;
}
PixelReader::~PixelReader() {
    this->FreeCaptureRegion();

    if (this->m_hdcScreen == NULL)
        throw std::runtime_error("Failed to get screen device context.");
}
bool PixelReader::CaptureScreenRegion(int x, int y, int width, int height) {
    this->m_iX = x;
    this->m_iY = y;
    this->m_iWidth = width;
    this->m_iHeight = height;
    this->FreeCaptureRegion();

    this->m_hBitmap = CreateCompatibleBitmap(this->m_hdcScreen, width, height);
    if (!this->m_hBitmap)
        return false;

    SelectObject(this->m_hdcMemory, this->m_hBitmap);

    bool success = BitBlt(this->m_hdcMemory, 0, 0, width, height, this->m_hdcScreen, x, y, SRCCOPY);
    if (!success)
        return false;

    BITMAPINFO bitmapInfo = {};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = -height; // top-down
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

 


    this->m_pPixels = new BYTE[width * height * 4];
    if (!this->m_pPixels)
        return false;

    GetDIBits(this->m_hdcMemory, this->m_hBitmap, 0, height, this->m_pPixels, &bitmapInfo, DIB_RGB_COLORS);
    return true;
}
bool PixelReader::FreeCaptureRegion() {
    if (this->m_pPixels) {
        delete[] this->m_pPixels;
        this->m_pPixels = NULL;
    }

    if (this->m_hBitmap) {
        DeleteObject(this->m_hBitmap);
        this->m_hBitmap = NULL;
    }

    return true;
}
int PixelReader::PixelGetColor(int x, int y, bool useScreenCoords) {
    if (!this->m_pPixels)
        return -1;

    if (useScreenCoords) {
        x -= this->m_iX;
        y -= this->m_iY;
    }

    if ( x >= this->m_iWidth || y >= this->m_iHeight) {
        return -1;
    }

    int index = (y * this->m_iWidth + x) * 4;
    BYTE r = this->m_pPixels[index + 2];
    BYTE g = this->m_pPixels[index + 1];
    BYTE b = this->m_pPixels[index + 0];

    return (r << 16) | (g << 8) | b;
}
bool PixelReader::PixelSearch(CoordPair &retValue, int x, int y, int width, int height, int color, int variation) {
    if (!this->m_pPixels)
        return false;
    BYTE colorR = color >> 16;
    BYTE colorG = (color >> 8) & 0xFF;
    BYTE colorB = color & 0xFF;

    int x2 = x + width;
    int y2 = y + height;

    for (int xCoord = x; xCoord < x2; xCoord++) {
        for (int yCoord = y; yCoord < y2; yCoord++) {
            int index = (yCoord * this->m_iWidth + xCoord) * 4;
            BYTE r = this->m_pPixels[index + 2];
            BYTE g = this->m_pPixels[index + 1];
            BYTE b = this->m_pPixels[index + 0];

            if (abs(colorR - r) <= variation && 
                abs(colorG - g) <= variation && 
                abs(colorB - b) <= variation) {
                retValue = { xCoord, yCoord };
                return true;
            }
                
        }
    }
    return false;
}
std::vector<CoordPair> PixelReader::FindPixelOccurences(int x, int y, int width, int height, int color, int variation) {
    std::vector<CoordPair> pixelOccurences;
    if (!this->m_pPixels)
        return {};

    BYTE colorR = color >> 16;
    BYTE colorG = (color >> 8) & 0xFF;
    BYTE colorB = color & 0xFF;

    int x2 = x + width;
    int y2 = y + height;

    for (int xCoord = x; xCoord < x2; xCoord++) {
        for (int yCoord = y; yCoord < y2; yCoord++) {
            int index = (yCoord * this->m_iWidth + xCoord) * 4;
            BYTE r = this->m_pPixels[index + 2];
            BYTE g = this->m_pPixels[index + 1];
            BYTE b = this->m_pPixels[index + 0];

            if (abs(colorR - r) <= variation &&
                abs(colorG - g) <= variation &&
                abs(colorB - b) <= variation) {
                pixelOccurences.push_back({ xCoord, yCoord });
            }
        }
    }
    return pixelOccurences;
}
bool PixelReader::BitmapSearch(CoordPair& retValue, HBITMAP bitmap) {
    return true;
}