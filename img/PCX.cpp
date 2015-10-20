#define _SCL_SECURE_NO_WARNINGS
#include "PCX.h"

PCX::PCX(std::string fileName)
{
	Load(fileName);
}

PCX::~PCX()
{
	if (header != NULL)
		delete[] header;
	if (pixels != NULL)
		delete[] pixels;
}

void PCX::Load(std::string fileName)
{
	std::ifstream img(fileName, std::ios::binary);
	if (!img)
		throw std::ifstream::failure("Can't open image : " + fileName);

	img.seekg(0, std::ios::end);
	int fileSize = img.tellg();
	img.seekg(0, std::ios::beg);

	header = (PCXHEADER*)(new char[sizeof(PCXHEADER)]);
	if (header == NULL)
		throw std::exception("Can't allocate memory to PCXHEADER");

	img.read((char*)header, sizeof PCXHEADER);
	if (header->Id != 10 || header->Format != 1)
		throw std::exception("Bad pcx image format");
	if (header->NumberOfPlanes != 1 || header->BitsPixelPlane != 8)
		throw std::exception("Pcx is not 256 grayscale image");

	width = header->Xmax - header->Xmin + 1;
	height = header->Ymax - header->Ymin + 1;
	frameH = height + 2;
	frameW = width + 2;
	header->PaletteInfo = 2;

	// make frame
	pixels = new BYTE[frameH * frameW];

	auto sourceSize = fileSize - sizeof(PCXHEADER);
	auto source = new BYTE[sourceSize];
	if (source == NULL)
		throw std::exception("Can't allocate memory to source pcx data");
	img.read((char*)source, sourceSize);
	Decode(source);
	delete[] source;
}

void PCX::Save(std::string fileName)
{
	std::ofstream img(fileName, std::ios::binary);
	if (!img)
		throw std::ofstream::failure("Can't open image : " + fileName);
	img.write((char*)header, sizeof PCXHEADER);
	Encode(&img);
}

int PCX::DecodeLine(BYTE* sourceBytes, BYTE* decodedBytes, int lineLength)
{
	int sourcePos = 0, decodedPos = 1;
	BYTE curByte, curByteCount;
	while (decodedPos < lineLength)
	{
		curByteCount = 1;
		curByte = sourceBytes[sourcePos++];
		if ((curByte & COUNTFLAG) == COUNTFLAG)
		{
			curByteCount = curByte & COUNTMASK;
			curByte = sourceBytes[sourcePos++];
		}
		for (; curByteCount && decodedPos < lineLength; --curByteCount, ++decodedPos)
			decodedBytes[decodedPos] = curByte;
	}

	// make frame
	decodedBytes[0] = decodedBytes[1];
	decodedBytes[lineLength] = decodedBytes[lineLength - 1];

	return sourcePos;
}

void PCX::Decode(BYTE* source)
{
	for (int i = 0, size = 0; i < frameH; ++i)
	{
		if (i == 1 || i == (frameH - 1))
			std::copy(pixels + frameW * (i - 1), pixels + frameW * i, pixels + frameW * i);
		else
			size += DecodeLine(source + size, pixels + frameW * i, width + 1);
	}
}

int PCX::EncodeLine(BYTE* sourceBytes, BYTE* encodedBytes, int lineLength)
{
	int sourcePos = 1, encodedPos = 0;
	BYTE curByte, curByteCount;
	while (sourcePos < lineLength)
	{
		for (curByteCount = 1, curByte = sourceBytes[sourcePos]; curByte == sourceBytes[sourcePos + curByteCount] && sourcePos + curByteCount < lineLength && curByteCount < COUNTMASK; ++curByteCount);

		if (curByte < COUNTFLAG && curByteCount == 1)
			encodedBytes[encodedPos++] = curByte;
		else
		{
			encodedBytes[encodedPos++] = curByteCount | COUNTFLAG;
			encodedBytes[encodedPos++] = curByte;
		}
		sourcePos += curByteCount;
	}
	return encodedPos;
}

void PCX::Encode(std::ofstream *img)
{
	auto buf = new BYTE[width * 2];

	for (int i = 0, size = 0; i < frameH; ++i)
	{
		if (i == 0 || i == (frameH - 1))
			continue;
		size = EncodeLine(pixels + frameW * i, buf, width + 1);
		img->write((char*)buf, size);
	}
	delete[] buf;
}

int PCX::Width()
{
	return frameW;
}

int PCX::Height()
{
	return frameH;
}

BYTE* PCX::Raw()
{
	return pixels;
}