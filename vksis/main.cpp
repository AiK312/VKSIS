#include <iostream>
#include "img\PCX.h"
#include "img\ImgUtils.hpp"
int main()
{
	auto img = new PCX("tstN.pcx");
	auto newPixels = new unsigned char[img->Height() * img->Width()];
	//ImgUtils::AddNoise(img->Raw(), img->Height(), img->Width(), 0.15);
	ImgUtils::MedianFilter(img->Raw(), newPixels, img->Height(), img->Width());
	ImgUtils::Rotate180(newPixels, img->Height() * img->Width());
	img->NewRaw(newPixels);
	
	img->Save("tst.pcx");
	return 0;
}