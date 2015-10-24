#define _SCL_SECURE_NO_WARNINGS
#include <iostream>
#include <chrono>
#include <fstream>
#include <string>
#include "..\img\ImgUtils.hpp"

using namespace std;

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

int main()
{
	auto start = Clock::now();
	std::cout << "Linear bmp-image processing program" << std::endl;
	auto timer = Clock::now();
	bitmap img("NoiseInp.bmp");
	int length = (img.width() + 2) * (img.height() + 2);
	unsigned char *pixels = new unsigned char[length];
	img.export_gray_scale_with_frame(pixels);
	PrintElapsed("Open file", timer, Clock::now());
	unsigned char *newPixels = new unsigned char[length];
	timer = Clock::now();
	ImgUtils::MedianFilter(pixels, newPixels, img.height() + 2, img.width() + 2);
	PrintElapsed("Median filter", timer, Clock::now());
	timer = Clock::now();
	ImgUtils::Rotate180(newPixels, length);
	PrintElapsed("Rotate image", timer, Clock::now());
	timer = Clock::now();
	img.import_gray_scale_with_frame(newPixels);
	img.save_image("NoiseOut.bmp");
	PrintElapsed("Save file", timer, Clock::now());
	PrintElapsed("Total time", start, Clock::now());
	delete[] pixels;
	delete[] newPixels;
	return 0;
}