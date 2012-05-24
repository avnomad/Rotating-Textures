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

#ifndef BITMAP_H
#define BITMAP_H

#include <memory>
#include <utility>

struct Bitmap
{
	// fields
	std::unique_ptr<unsigned char[]> data;
	unsigned int width;
	unsigned int height;

	// constructors
	Bitmap():data(nullptr),width(0),height(0){}
	Bitmap(const wchar_t *path);

	// operators
	Bitmap &operator=(Bitmap &&original)
	{
		width = original.width;
		height = original.height;
		data = std::move(original.data);
		return *this;
	} // end function operator=

}; // end struct Bitmap

#endif // BITMAP_H