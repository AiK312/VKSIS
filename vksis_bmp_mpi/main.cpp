#define _SCL_SECURE_NO_WARNINGS
#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <string>
#include "..\img\ImgUtils.hpp"

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::steady_clock::time_point ClockPoint;

class bitmap
{
public:
	bitmap(const std::string& filename) :
		file_name_(filename),
		data_(0),
		length_(0),
		width_(0),
		height_(0),
		row_increment_(0),
		bytes_per_pixel_(0)
	{
		load_bitmap();
	}

	~bitmap()
	{
		delete[] data_;
	}

	void load_bitmap()
	{
		std::ifstream stream(file_name_.c_str(), std::ios::binary);
		if (!stream)
		{
			std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - file " << file_name_ << " not found!" << std::endl;
			return;
		}
		bitmap_file_header bfh;
		bitmap_information_header bih;
		read_from_stream(stream, &bfh, sizeof(bfh));
		read_from_stream(stream, &bih, sizeof(bih));
		if (bfh.type != 19778)
		{
			stream.close();
			std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid type value " << bfh.type << " expected 19778." << std::endl;
			return;
		}
		if (bih.bit_count != 24)
		{
			stream.close();
			std::cerr << "bitmap_image::load_bitmap() ERROR: bitmap_image - Invalid bit depth " << bih.bit_count << " expected 24." << std::endl;
			return;
		}
		height_ = bih.height;
		width_ = bih.width;
		bytes_per_pixel_ = bih.bit_count >> 3;
		unsigned int padding = (4 - ((3 * width_) % 4)) % 4;
		char padding_data[4] = { 0,0,0,0 };
		create_bitmap();
		for (unsigned int i = 0; i < height_; ++i)
		{
			unsigned char* data_ptr = row(height_ - i - 1);
			stream.read(reinterpret_cast<char*>(data_ptr), sizeof(char) * bytes_per_pixel_ * width_);
			stream.read(padding_data, padding);
		}
	}

	void create_bitmap()
	{
		length_ = width_ * height_ * bytes_per_pixel_;
		row_increment_ = width_ * bytes_per_pixel_;
		if (0 != data_)
			delete[] data_;
		data_ = new unsigned char[length_];
	}

	void save_image(const std::string& file_name)
	{
		std::ofstream stream(file_name.c_str(), std::ios::binary);
		if (!stream)
		{
			std::cout << "bitmap_image::save_image(): Error - Could not open file " << file_name << " for writing!" << std::endl;
			return;
		}
		bitmap_information_header bih =
		{
			40,	width_,	height_, 1, static_cast<unsigned short>(bytes_per_pixel_ << 3),	0, (((bih.width * bytes_per_pixel_) + 3) & 0x0000FFFC) * bih.height, 0,	0, 0, 0
		};
		bitmap_file_header bfh =
		{
			19778, 55 + bih.size_image,	0, 0, sizeof(bitmap_file_header) + sizeof(bitmap_information_header)
		};
		write_to_stream(stream, &bfh, sizeof(bfh));
		write_to_stream(stream, &bih, sizeof(bih));
		unsigned int padding = (4 - ((3 * width_) % 4)) % 4;
		char padding_data[4] = { 0x0,0x0,0x0,0x0 };
		for (unsigned int i = 0; i < height_; ++i)
		{
			unsigned char* data_ptr = data_ + (row_increment_ * (height_ - i - 1));
			stream.write(reinterpret_cast<char*>(data_ptr), sizeof(unsigned char) * bytes_per_pixel_ * width_);
			stream.write(padding_data, padding);
		}

		stream.close();
	}

	inline void export_gray_scale_with_frame(unsigned char* gray)
	{
		unsigned char* itr = data_;
		for (int i = 0; i < height_; ++i)
			for (int j = 0; j < width_; ++j)
			{
				unsigned char gray_value = static_cast<unsigned char>((0.299 * (*(itr + 2))) +
					(0.587 * (*(itr + 1))) +
					(0.114 * (*(itr + 0))));
				gray[(i + 1) * (width_ + 2) + (j + 1)] = gray_value;
				itr += bytes_per_pixel_;
				if (j == 0)
					gray[(i + 1) * (width_ + 2) + j] = gray_value;
				if (j == width_ - 1)
					gray[(i + 1) * (width_ + 2) + (j + 2)] = gray_value;
			}
		std::copy(gray + (width_ + 2), gray + (width_ + 2) * 2, gray);
		std::copy(gray + (width_ + 2) * (height_), gray + (width_ + 2) * (height_ + 1), gray + (width_ + 2) * (height_ + 1));
	}

	inline void import_gray_scale_with_frame(unsigned char* gray)
	{
		unsigned char* itr = data_;
		for (int i = 0; i < height_; ++i)
			for (int j = 0; j < width_; ++j)
			{
				unsigned char gray_value = gray[(i + 1) * (width_ + 2) + (j + 1)];
				*(itr++) = gray_value;
				*(itr++) = gray_value;
				*(itr++) = gray_value;
			}
	}

	inline unsigned int width() const
	{
		return width_;
	}

	inline unsigned int height() const
	{
		return height_;
	}

private:
#pragma pack(push, 1)
	struct bitmap_file_header
	{
		unsigned short type;
		unsigned int   size;
		unsigned short reserved1;
		unsigned short reserved2;
		unsigned int   off_bits;
	};

	struct bitmap_information_header
	{
		unsigned int   size;
		unsigned int   width;
		unsigned int   height;
		unsigned short planes;
		unsigned short bit_count;
		unsigned int   compression;
		unsigned int   size_image;
		unsigned int   x_pels_per_meter;
		unsigned int   y_pels_per_meter;
		unsigned int   clr_used;
		unsigned int   clr_important;
	};
#pragma pack(pop)

	template<typename T>
	inline void read_from_stream(std::ifstream& stream, T* t, int size)
	{
		stream.read(reinterpret_cast<char*>(t), size);
	}

	template<typename T>
	inline void write_to_stream(std::ofstream& stream, const T* t, int size)
	{
		stream.write(reinterpret_cast<const char*>(t), size);
	}

	inline unsigned char* row(unsigned int row_index) const
	{
		return data_ + (row_index * row_increment_);
	}

	std::string    file_name_;
	unsigned char* data_;
	unsigned int   length_;
	unsigned int   width_;
	unsigned int   height_;
	unsigned int   row_increment_;
	unsigned int   bytes_per_pixel_;
};

inline void PrintElapsed(std::string text, ClockPoint start, ClockPoint end)
{
	std::cout << text << "\t:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count() << " ms" << std::endl;
}

inline long long Duration()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now().time_since_epoch()).count();
}

template<typename T> 
inline void printElement(T t, const int& width)
{
	std::cout << std::left << std::setw(width) << std::setfill(' ') << t;
}

int main(int argc, char *argv[])
{
	auto start = Clock::now();
	ClockPoint timer;
	bitmap* img = NULL;
	unsigned char *pixels;
	long long times[5], *allTimes = NULL, timeSendSize, timeSendBuf, timeRecvBuf, timeEnd;
	int imgSize[2] = { 0 }, length, rank, nproc;
	int &heigth = imgSize[0], &width = imgSize[1];
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	if (rank == 0)
	{
		std::cout << "Parallel bmp-image processing program" << std::endl;
		timer = Clock::now();
		img = new bitmap("NoiseInp.bmp");
		PrintElapsed("Open file", timer, Clock::now());
		width = img->width() + 2;
		heigth = img->height() + 2;
		length = width * heigth;
		pixels = new unsigned char[length];
		img->export_gray_scale_with_frame(pixels);
		timeSendSize = Duration();
	}
	MPI_Bcast(&imgSize, 2, MPI_INT, 0, MPI_COMM_WORLD);
	times[0] = Duration();
	int linePerProc = std::ceil((heigth - 2) / nproc);
	int *sConuts = new int[nproc], *sDist = new int[nproc];
	int tmp = heigth - 2;
	for (int i = 0; i < nproc; ++i)
	{
		sConuts[i] = (std::min(linePerProc, tmp) + 2) * width;
		sDist[i] = (i == 0) ? 0 : (sDist[i - 1] + sConuts[i - 1]) - 2 * width;
		tmp -= linePerProc;
	}
	unsigned char *recvBuf = new unsigned char[sConuts[rank]];
	int recvSize = sConuts[rank];
	char *sendBuf = (char*)((img != NULL) ? pixels : NULL);
	if (rank == 0)
		timeSendBuf = Duration();
	MPI_Scatterv(sendBuf, sConuts, sDist, MPI_CHAR, recvBuf, recvSize, MPI_CHAR, 0, MPI_COMM_WORLD);
	times[1] = Duration();
	unsigned char *newImg = new unsigned char[recvSize];
	timer = Clock::now();
	ImgUtils::MedianFilter(recvBuf, newImg, recvSize / width, width);
	times[2] = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timer).count();
	timer = Clock::now();
	ImgUtils::Rotate180(newImg, recvSize);
	times[3] = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timer).count();
	for (int i = 0; i < nproc; ++i)
	{
		sConuts[i] -= 2 * width;
		sDist[i] += width;
	}
	recvSize = sConuts[rank];
	std::reverse(sConuts, sConuts + nproc);
	std::reverse(sDist, sDist + nproc);
	times[4] = Duration();
	MPI_Gatherv(newImg + width, recvSize, MPI_CHAR, sendBuf, sConuts, sDist, MPI_CHAR, 0, MPI_COMM_WORLD);
	if (rank == 0)
	{
		timeRecvBuf = Duration();
		timer = Clock::now();
		img->import_gray_scale_with_frame((unsigned char*)sendBuf);
		img->save_image("NoiseOut.bmp");
		PrintElapsed("Save file", timer, Clock::now());
		PrintElapsed("Total time", start, Clock::now());
		delete img;
	}
	allTimes = new long long[nproc * 5];
	MPI_Gather(times, 5, MPI_LONG_LONG_INT, allTimes, 5, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
	if (rank == 0)
	{
		long long timeStart = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();
		std::cout << "send size after start : " << timeSendSize - timeStart<< std::endl;
		printElement("proc", 6);
		printElement("recv start", 18);
		printElement("recv send ", 18);
		std::cout << std::endl;
		for (int i = 0; i < nproc; ++i)
		{
			printElement(i, 6);
			printElement(allTimes[5 * i] - timeStart, 18);
			printElement(allTimes[5 * i] - timeSendSize, 18);
			std::cout << std::endl;
		}
		std::cout << "send buf after start  : " << timeSendBuf - timeStart<< std::endl;
		printElement("proc", 6);
		printElement("recv start", 18);
		printElement("recv send ", 18);
		std::cout << std::endl;
		for (int i = 0; i < nproc; ++i)
		{
			printElement(i, 6);
			printElement(allTimes[5 * i + 1] - timeStart, 18);
			printElement(allTimes[5 * i + 1] - timeSendBuf, 18);
			std::cout << std::endl;
		}
		std::cout << "img processing" << std::endl;
		printElement("proc", 6);
		printElement("median fil", 18);
		printElement("rotate    ", 18);
		std::cout << std::endl;
		for (int i = 0; i < nproc; ++i)
		{
			printElement(i, 6);
			printElement(allTimes[5 * i + 2], 18);
			printElement(allTimes[5 * i + 3], 18);
			std::cout << std::endl;
		}
		std::cout << "recv buf after start  : " << timeRecvBuf - timeStart << std::endl;
		printElement("proc", 6);
		printElement("recv start", 18);
		printElement("recv send ", 18);
		std::cout << std::endl;
		for (int i = 0; i < nproc; ++i)
		{
			printElement(i, 6);
			printElement(allTimes[5 * i + 4] - timeStart, 18);
			printElement(timeRecvBuf - allTimes[5 * i + 4], 18);
			std::cout << std::endl;
		}
	}
	MPI_Finalize();
	delete[] allTimes;
	delete[] recvBuf;
	delete[] newImg;
	delete[] pixels;
	return 0;
}