/*
Copyright 2012. All rights reserved.
Institute of Measurement and Control Systems
Karlsruhe Institute of Technology, Germany

This file is part of libDird.
Authors: Henning Lategahn
         Johannes Beck
         Bernd Kitt
Website: http://www.mrt.kit.edu/libDird.php

libDird is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or any later version.

libDird is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
libDird; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA 
*/

#pragma once
#include <string>
#include <string.h>
#include <cassert>
#include <FreeImage.h>

struct FIBITMAP;

namespace DIRD {

class cImage
{
public:
	cImage();
	cImage(const std::string& filename);
	cImage(const int u, const int v, const int bpp);

	~cImage();

	void getPixel();
	
	bool create(const int u, const int v, const int bpp);
	bool load(const std::string& filename);

	bool write(const std::string& filename) const;

	void destroy();
	
	int getWidth() const;
	int getHeight() const;
	
	template <typename T>
	void getPixel(const int u, const int v, T& pixelData) const {
		assert(image_ != NULL);
		assert(sizeof(pixelData) == bpp_);
		
		BYTE* bits = FreeImage_GetScanLine(image_, v);
		memcpy(&pixelData, bits+bpp_*u, bpp_);
	}

	template <typename T>
	void setPixel(const int u, const int v, const T& pixelData) {
		assert(image_ != NULL);
		assert(sizeof(pixelData) == bpp_);
		
		BYTE* bits = FreeImage_GetScanLine(image_, v);
		memcpy(bits+bpp_*u, &pixelData, bpp_);
	}
private:
	static bool isInit_;
	FIBITMAP* image_;
	unsigned int width_;
	unsigned int height_;
	unsigned int pitch_;
	int imageType_;
	unsigned int bpp_;
};

}
