#include <algorithm>
#include <cstdlib>
#include <ctime>

class ImgUtils
{
private:

	static inline void LoadLine(unsigned char* source, unsigned char* window)
	{
		window[0] = source[-1];
		window[1] = source[0];
		window[2] = source[1];
	}

	static inline void LoadWindow(unsigned char* source, int y, int x, int width, unsigned char* window)
	{
		LoadLine(source + (y - 1) * width + x, window + 0);
		LoadLine(source + (y + 0) * width + x, window + 3);
		LoadLine(source + (y + 1) * width + x, window + 6);
	}

	static inline void SortWindow(unsigned char* window)
	{
		std::sort(window, window + 9);
	}

public:

	static void AddNoise(unsigned char* original, int heigth, int width, float p)
	{
		srand(time(NULL));
		int amount = heigth * width * p, x, y;
		for (int i = 0; i < amount; ++i)
		{
			y = rand() % heigth;
			x = rand() % width;
			original[y * width + x] = (i % 2) ? 255 : 0 ;
		}
	}

	static void MedianFilter(unsigned char* original, unsigned char* processed, int heigth, int width)
	{
		auto window = new unsigned char[9];
		int H = heigth - 1, W = width - 1;
		for (int i = 1; i < H; ++i)
			for (int j = 1; j < W; ++j)
			{
				LoadWindow(original, i, j, width, window);
				SortWindow(window);
				processed[i * width + j] = window[4];
			}
		delete[] window;
	}

	static void Rotate180(unsigned char* original, int size)
	{
		std::reverse(original, original + size);
	}
};