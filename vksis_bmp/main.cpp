#define _SCL_SECURE_NO_WARNINGS
#include <iostream>
#include "..\img\bitmap_image.hpp"
#include "..\img\ImgUtils.hpp"

int main()
{
	bitmap_image img("tst.bmp");
	int length = (img.width()+2) * (img.height()+2);
	unsigned char *pixels = new unsigned char[length];
	unsigned char *newPixels = new unsigned char[length];
	img.export_gray_scale_with_frame(pixels);
	ImgUtils::MedianFilter(pixels, newPixels, img.height() + 2, img.width() + 2);
	img.import_gray_scale_with_frame(newPixels);
	img.save_image("tst2.bmp");
	delete[] pixels;
	delete[] newPixels;
	return 0;
}