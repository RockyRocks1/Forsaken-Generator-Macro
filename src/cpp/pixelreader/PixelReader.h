#include <windows.h>
#include <vector>
#include "colorutils.h"

struct CoordPair  {
	int x;
	int y;
};

class PixelReader {
private:
	HDC m_hdcScreen;
	HDC m_hdcMemory;
	HBITMAP m_hBitmap;
	BYTE* m_pPixels;
	int m_iX;
	int m_iY;
	int m_iWidth;
	int m_iHeight;
public:
	PixelReader();
	~PixelReader();


	bool CaptureScreenRegion(int x, int y, int width, int height);
	bool FreeCaptureRegion();

	
public:
	int PixelGetColor(int x, int y, bool useScreenCoords = false);
	bool PixelSearch(CoordPair& retValue, int x, int y, int width, int height, int color, int variation = 0);
	std::vector<CoordPair> FindPixelOccurences(int x, int y, int width, int height, int color, int variation = 0);
	bool BitmapSearch(CoordPair& retValue, HBITMAP bitmap);

};

