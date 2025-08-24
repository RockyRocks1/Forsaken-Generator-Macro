#include "ColorUtils.h"
int BGRtoRGB(int bgrValue) {
	char b = (bgrValue >> 16) & 0xFF;
	char g = (bgrValue >> 8) & 0xFF;
	char r = bgrValue & 0xFF;

	return (r << 16) | (g << 8) | b;

}
int RGBtoBGR(int rgbValue) {
	char r = (rgbValue >> 16) & 0xFF;
	char g = (rgbValue >> 8) & 0xFF;
	char b = rgbValue & 0xFF;

	return (b << 16) | (g << 8) | r;
}
