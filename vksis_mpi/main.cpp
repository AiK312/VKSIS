#include <mpi.h>
#include <iostream>
#include "..\img\pcx.h"
#include "..\img\ImgUtils.hpp"

int main(int argc, char *argv[])
{
	PCX* img = NULL;
	int imgSize[2] = {0};
	int &heigth = imgSize[0], &width = imgSize[1];
	int rank, nproc;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	printf("process %d of %d\n", rank, nproc);

	if (rank == 0)
	{
		img = new PCX("tstN.pcx");
		width = img->Width();
		heigth = img->Height();
	}

	MPI_Bcast(&imgSize, 2, MPI_INT, 0, MPI_COMM_WORLD);
	std::cout << rank << " " << heigth << " " << width << std::endl;
	
	int linePerProc = std::ceil((heigth - 2) / nproc);

	int *sConuts = new int[nproc], *sDist = new int[nproc];

	int tmp = heigth - 2;
	for (int i = 0; i < nproc; ++i)
	{
		sConuts[i] = (std::min(linePerProc, tmp) + 2) * width;
		sDist[i] = (i == 0) ? 0 : (sDist[i-1] + sConuts[i-1]) - 2 * width;
		tmp -= linePerProc;
	}

	unsigned char *recvBuf = new unsigned char[sConuts[rank]];
	int recvSize = sConuts[rank];
	char *sendBuf = (char*)((img != NULL) ? img->Raw() : NULL);

	MPI_Scatterv(sendBuf, sConuts, sDist, MPI_CHAR, recvBuf, recvSize, MPI_CHAR, 0, MPI_COMM_WORLD);

	std::reverse(recvBuf, recvBuf + recvSize);
	unsigned char *newImg = new unsigned char[recvSize];
	ImgUtils::MedianFilter(recvBuf, newImg, recvSize / width, width);
	for (int i = 0; i < nproc; ++i)
	{
		sConuts[i] -= 2 * width;
		sDist[i] += width;
	}
	recvSize = sConuts[rank];
	std::reverse(sConuts, sConuts + nproc);
	std::reverse(sDist, sDist + nproc);

	MPI_Gatherv(newImg + width, recvSize, MPI_CHAR, sendBuf, sConuts, sDist, MPI_CHAR, 0, MPI_COMM_WORLD);
	delete[] recvBuf;
	delete[] newImg;
	if (rank == 0)
	{
		img->Save("tstMPI.pcx");
		delete img;
	}
	MPI_Finalize();
	return 0;
}