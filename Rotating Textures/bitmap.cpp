//	Copyright (C) 2011-2012 Vaptistis Anogeianakis <el05208@mail.ntua.gr>
/*
 *	This file is part of Rotating Textures.
 *
 *	Rotating Textures is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Rotating Textures is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Rotating Textures.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>
using std::ifstream;
using std::ios;

#include <stdexcept>
using std::runtime_error;

#include <windows.h>

#include "bitmap.h"

Bitmap::Bitmap(const wchar_t *path)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	ifstream in;
	unsigned int size;

	in.open(path,ios::binary|ios::in);
	if(!in)
		throw runtime_error("Could not open file!");
	in.read((char*)&fileHeader,sizeof(fileHeader));
	if(!in)
		throw runtime_error("Could not read the entire file header!");
	in.read((char*)&infoHeader,sizeof(infoHeader));
	if(!in)
		throw runtime_error("Could not read the entire info header!");
	if(fileHeader.bfType != *(WORD*)"BM")
		throw runtime_error("Magic Number not \"MP\"!");
	if(infoHeader.biSize != sizeof(infoHeader))
		throw runtime_error("Unsupported DIB version!");
	if(fileHeader.bfOffBits != sizeof(fileHeader)+sizeof(infoHeader))
		throw runtime_error("Masks or Color table present!");

	// more error checking goes here...

	static_assert(sizeof(unsigned char) == 1,"sizeof(unsigned char) must be 1");
	size = fileHeader.bfSize - fileHeader.bfOffBits;

	data.reset(new unsigned char[size]);
	in.read((char*)data.get(),size);

	width = infoHeader.biWidth;
	height = infoHeader.biHeight;

	in.close();
	in.clear();
}; // end function loadBitmapFromFile
