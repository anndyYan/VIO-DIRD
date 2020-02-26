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
#include "cImage.h"
#include "FreeImage.h"

#include <cassert>
#include <stdexcept>

namespace DIRD {
	bool cImage::isInit_ = false;

	cImage::cImage() 
		: image_(NULL)
		, width_ (0), height_(0), pitch_(0), imageType_(FIT_UNKNOWN),  bpp_(0)
	{
		if (isInit_ == false) {
			FreeImage_Initialise();
			isInit_ = true;
		}
	}

	cImage::cImage(const std::string& filename) 
		: image_(NULL)
		, width_ (0), height_(0), pitch_(0), imageType_(FIT_UNKNOWN),  bpp_(0)
	{
		if (isInit_ == false) {
			FreeImage_Initialise();
			isInit_ = true;
		}

		if (load(filename) == false) {
			throw std::runtime_error((std::string)("Cannot load bitmap: ") + filename);
		}
	}

	cImage::cImage(const int u, const int v, const int bpp)
		: image_(NULL)
		, width_ (0), height_(0), pitch_(0), imageType_(FIT_UNKNOWN),  bpp_(0)
	{
		if (isInit_ == false) {
			FreeImage_Initialise();
			isInit_ = true;
		}

		if (create(u, v, bpp) == false) {
			throw std::runtime_error("Cannot create bitmap");
		}
	}

	cImage::~cImage() {
		destroy();
	}

	void cImage::destroy() {
		if (image_ != NULL) {
			FreeImage_Unload(image_);
			image_ = NULL;
		}
	}

	bool cImage::load(const std::string& filename) {
		destroy();
		
		image_ = FreeImage_Load(FIF_PNG, filename.c_str());
		if (image_ == NULL) {
			return false;
		}
		
		width_ = FreeImage_GetWidth(image_);
		height_ = FreeImage_GetHeight(image_);
		pitch_ = FreeImage_GetPitch(image_);
		imageType_ = FreeImage_GetImageType(image_);
		bpp_ = FreeImage_GetBPP(image_) / 8;
		
		if (imageType_ != FIT_BITMAP || bpp_ != 1) {
			FreeImage_Unload(image_);
			image_ = NULL;
			return false;
		}
		
		return true;
	}

	bool cImage::write(const std::string& filename) const {
		assert(image_ != NULL);

		return FreeImage_Save(FIF_PNG, image_, filename.c_str()) == TRUE;
	}


	bool cImage::create(const int u, const int v, const int bpp) {
		destroy();

		image_ = FreeImage_AllocateT(FIT_BITMAP, u, v, bpp);

		width_ = FreeImage_GetWidth(image_);
		height_ = FreeImage_GetHeight(image_);
		pitch_ = FreeImage_GetPitch(image_);
		imageType_ = FreeImage_GetImageType(image_);
		bpp_ = FreeImage_GetBPP(image_) / 8;

		return (image_ != NULL);
	}
	
	int cImage::getWidth() const {
		assert(image_ != NULL);

		return width_;
	}
	
	int cImage::getHeight() const {
		assert(image_ != NULL);

		return height_;
	}
}

