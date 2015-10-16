#pragma once
#ifndef PCX_H
#define PCX_H

#include <string>
#include <fstream>

typedef unsigned char   BYTE;       /*  8-bit unsigned data type  */
typedef unsigned short  WORD;       /* 16-bit unsigned data type  */

class PCX
{
private:
	const int COUNTMASK = 0x3F;
	const int COUNTFLAG = 0xC0;

	typedef struct PCXHEADER
	{
		BYTE   Id;                  /*  00h     Manufacturer ID        */
		BYTE   Version;             /*  01h     Version                */
		BYTE   Format;              /*  02h     Encoding Scheme        */
		BYTE   BitsPixelPlane;      /*  03h     Bits/Pixel/Plane       */
		WORD   Xmin;                /*  04h     X Start (upper left)   */
		WORD   Ymin;                /*  06h     Y Start (top)          */
		WORD   Xmax;                /*  08h     X End (lower right)    */
		WORD   Ymax;                /*  0Ah     Y End (bottom)         */
		WORD   Hdpi;                /*  0Ch     Horizontal Resolution  */
		WORD   Vdpi;                /*  0Eh     Vertical Resolution    */
		BYTE   EgaPalette[48];      /*  10h     16-Color EGA Palette   */
		BYTE   Reserved;            /*  40h     Reserved               */
		BYTE   NumberOfPlanes;      /*  41h     Number of Color Planes */
		WORD   BytesLinePlane;      /*  42h     Bytes/Line/Plane       */
		WORD   PaletteInfo;         /*  44h     Palette Interpretation */
		WORD   HScreenSize;         /*  46h     Horizontal Screen Size */
		WORD   VScreenSize;         /*  48h     Vertical Screen Size   */
		BYTE   Filler[54];          /*  4Ah     Reserved               */
	};

	PCXHEADER *header = NULL;
	BYTE *pixels = NULL;

	int width = 0;
	int height = 0;

	int DecodeLine(BYTE* source, BYTE* pixelsPart, int lineLength);
	int EncodeLine(BYTE* source, BYTE* buf, int bufSize);
	void Decode(BYTE* source);
	void Encode(std::ofstream *img);

public:
	PCX(std::string filename);
	~PCX();

	int Width();
	int Height();
	BYTE* Raw();
	void NewRaw(BYTE* newPixels);
	void Load(std::string filename);
	void Save(std::string filename);
};

#endif