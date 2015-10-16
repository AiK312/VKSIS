#include <iostream>
#include "img\PCX.h"
int main()
{
	auto img = new PCX("tst4.pcx");
	img->Save("tst.pcx");
	img->~PCX();
	return 0;
}