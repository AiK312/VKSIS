#define _SCL_SECURE_NO_WARNINGS
#include <mpi.h>
#include <iostream>
#include <chrono>
#include "..\img\pcx.hpp"
#include "..\img\ImgUtils.hpp"

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::steady_clock::time_point ClockPoint;

inline void PrintElapsed(std::string text, ClockPoint start, ClockPoint end)
{
	std::cout << text << "\t:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count() << " ns" << std::endl;
}

int main(int argc, char *argv[])
{
	auto start = Clock::now();
	auto timer = Clock::now();
	PCX* img = NULL;
	int imgSize[2] = { 0 };
	int &heigth = imgSize[0], &width = imgSize[1];
	int rank, nproc;
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
		std::cout << "Send size\t:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count() << " ns" << std::endl;
	}
	MPI_Bcast(&imgSize, 2, MPI_INT, 0, MPI_COMM_WORLD);
	std::cout << rank << " Recived szie\t:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count() << " ns" << std::endl;
	int linePerProc = std::ceil((heigth - 2) / nproc);
	int *sConuts = new int[nproc], *sDist = new int[nproc];
	int tmp = heigth - 2;
	for (int i = 0; i < nproc; ++i)
	{
		sConuts[i] = (std::min(linePerProc, tmp) + 2) * width;
		sDist[i] = (i == 0) ? 0 : (sDist[i - 1] + sConuts[i - 1]) - 2 * width;
		tmp -= linePerProc;
	}
	int recvSize = sConuts[rank];
	unsigned char *recvBuf = new unsigned char[recvSize];
	char *sendBuf = (char*)((img != NULL) ? img->Raw() : NULL);
	if (rank == 0)
		std::cout << "Send buf\t:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count() << " ns" << std::endl;
	MPI_Scatterv(sendBuf, sConuts, sDist, MPI_CHAR, recvBuf, recvSize, MPI_CHAR, 0, MPI_COMM_WORLD);
	std::cout << rank << " Recived buf\t:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count() << " ns" << std::endl;
	unsigned char *newImg = new unsigned char[recvSize];
	timer = Clock::now();
	ImgUtils::MedianFilter(recvBuf, newImg, recvSize / width, width);
	PrintElapsed("Median filter", timer, Clock::now());
	timer = Clock::now();
	ImgUtils::Rotate180(newImg, recvSize);
	PrintElapsed("Rotate image", timer, Clock::now());
	for (int i = 0; i < nproc; ++i)
	{
		sConuts[i] -= 2 * width;
		sDist[i] += width;
	}
	recvSize = sConuts[rank];
	std::reverse(sConuts, sConuts + nproc);
	std::reverse(sDist, sDist + nproc);
	std::cout << rank << " Send proc\t:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count() << " ns" << std::endl;
	MPI_Gatherv(newImg + width, recvSize, MPI_CHAR, sendBuf, sConuts, sDist, MPI_CHAR, 0, MPI_COMM_WORLD);
	if (rank == 0)
	{
		std::cout << "Recived proc\t:\t" << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count() << " ns" << std::endl;
		timer = Clock::now();
		img->Save("NoiseOut.pcx");
		PrintElapsed("Save file", timer, Clock::now());
	}
	MPI_Finalize();
	if (rank == 0)
	{
		PrintElapsed("Total time", start, Clock::now());
		delete img;
	}
	delete[] recvBuf;
	delete[] newImg;
	return 0;
}