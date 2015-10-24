#define _SCL_SECURE_NO_WARNINGS
#include <iostream>
#include <chrono>
#include "..\img\pcx.hpp"
#include "..\img\ImgUtils.hpp"

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::steady_clock::time_point ClockPoint;

inline void PrintElapsed(std::string text, ClockPoint start, ClockPoint end)
{
	std::cout << text << "\t:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count() << " ms" << std::endl;
}

int main()
{
	auto start = Clock::now();
	std::cout << "Linear pcx-image processing program" << std::endl;
	auto timer = Clock::now();
	auto img = new PCX("NoiseInp.pcx");
	PrintElapsed("Open file", timer, Clock::now());
	auto imgSize = img->Height() * img->Width();
	auto newPixels = new unsigned char[imgSize];
	timer = Clock::now();
	ImgUtils::MedianFilter(img->Raw(), newPixels, img->Height(), img->Width());
	PrintElapsed("Median filter", timer, Clock::now());
	timer = Clock::now();
	ImgUtils::Rotate180(newPixels, imgSize);
	PrintElapsed("Rotate image", timer, Clock::now());
	timer = Clock::now();
	img->WriteRaw(newPixels, newPixels + imgSize);
	img->Save("NoiseOut.pcx");
	PrintElapsed("Save file", timer, Clock::now());
	PrintElapsed("Total time", start, Clock::now());
	delete[] newPixels;
	return 0;
}