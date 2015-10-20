#define _SCL_SECURE_NO_WARNINGS
#include <iostream>
#include "..\img\bitmap_image.hpp"

int main()
{
	bitmap_image img("tst.bmp");
	int length = (img.width()+2) * (img.height()+2);
	unsigned char *pixels = new unsigned char[length];
	img.export_gray_scale_with_frame(pixels);
	img.import_gray_scale_with_frame(pixels);
	img.save_image("tst2.bmp");
	delete[] pixels;
	//auto newPixels = new unsigned char[img->Height() * img->Width()];
	//ImgUtils::AddNoise(img->Raw(), img->Height(), img->Width(), 0.15);
	//ImgUtils::MedianFilter(img->Raw(), newPixels, img->Height(), img->Width());
	//std::reverse(newPixels, newPixels + img->Height() * img->Width());
	//std::copy(newPixels, newPixels + img->Height() * img->Width(), img->Raw());
	//ImgUtils::Rotate180(newPixels, img->Height() * img->Width());
	//std::copy(img)
	//img->NewRaw(newPixels);
	//img->Save("tst.pcx");
	return 0;
}