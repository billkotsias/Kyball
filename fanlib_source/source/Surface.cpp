#include "gfx\Surface.h"

namespace FANLib {

	Surface::~Surface() {
		if (dataOwner) {
			switch (bpp) {
				case 32:
					delete [] (unsigned __int32*)(data);
					break;
				case 8:
					delete [] (unsigned __int8*)(data);
					break;
			}
			data = 0;
		}
	}

	void Surface::calcBpp() {
		switch (format) {
			case ARGB_8888:
				bpp = 32;
				break;
			case A_8:
			case GREYSCALE_8:
			default:
				bpp = 8;
				break;
		}
	}

	void Surface::copyTo(void* buffer) {
		switch (bpp) {
			case 32:
				_copy<unsigned __int32>((unsigned __int32*)buffer, (unsigned __int32*)data);
				return;
			case 8:
				_copy<unsigned __int8>((unsigned __int8*)buffer, (unsigned __int8*)data);
				return;
		}
	}

	void Surface::copyFrom(void* buffer) {
		switch (bpp) {
			case 32:
				_copy<unsigned __int32>((unsigned __int32*)data, (unsigned __int32*)buffer);
				return;
			case 8:
				_copy<unsigned __int8>((unsigned __int8*)data, (unsigned __int8*)buffer);
				return;
		}
	}

	void Surface::fill(unsigned __int32 colour) {
		switch (bpp) {
			case 32:
				_fill<unsigned __int32>(colour);
				return;
			case 8:
				_fill<unsigned __int8>(colour);
				return;
		}
	}

}