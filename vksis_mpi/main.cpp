#define _SCL_SECURE_NO_WARNINGS
#include <mpi.h>
#include <iomanip>
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
	long long times[5], *allTimes = NULL, timeSendSize, timeSendBuf, timeRecvBuf, timeEnd;
	PCX* img = NULL;
	int imgSize[2] = { 0 };
	int &heigth = imgSize[0], &width = imgSize[1], rank, nproc;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	if (rank == 0)
	{
		std::cout << "Parallel pcx-image processing program" << std::endl;
		timer = Clock::now();
		img = new PCX("NoiseInp.pcx");
		PrintElapsed("Open file", timer, Clock::now());
		width = img->Width();
		heigth = img->Height();
		timeSendSize = Duration();
		//std::cout << "Send size\t:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now().time_since_epoch()).count() << " ns" << std::endl;
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
	char *sendBuf = (char*)((img != NULL) ? img->Raw() : NULL);
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
		img->Save("NoiseOut.pcx");
		PrintElapsed("Save file", timer, Clock::now());
		PrintElapsed("Total time", start, Clock::now());
		delete img;
	}
	allTimes = new long long[nproc * 5];
	MPI_Gather(times, 5, MPI_LONG_LONG_INT, allTimes, 5, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
	if (rank == 0)
	{
		long long timeStart = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();
		std::cout << "send size after start : " << timeSendSize - timeStart << std::endl;
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
		std::cout << "send buf after start  : " << timeSendBuf - timeStart << std::endl;
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
	return 0;
}