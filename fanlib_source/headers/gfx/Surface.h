// A pixel surface for blitting

#pragma once
#ifndef FANLIB_Surface_H
#define FANLIB_Surface_H

namespace FANLib {

	class Surface {

	public:

		enum PixelFormat {
			ARGB_8888,
			GREYSCALE_8,
			A_8,
		};

	protected:

		void* data;		/// pointer to 1st pixel
		int sizeX;		/// size in "picture elements"
		int sizeY;
		int rowSpace;	/// "dead" pixels between rows
		int rowSize;	/// cached calculated value of space between a pixel and the one below it
		int size;		/// total number of elements, including "dead" ones
		bool dataOwner;	/// has the pixel buffer been created by this surface?
		PixelFormat format;
		int bpp;		/// cached bits/pixel

		void calcBpp();

		inline void calcMisc() {
			rowSize = sizeX + rowSpace;
			size = sizeY * rowSize;
		};

		template <class Class>
		void _copy(Class* dest, Class* source) {
			for (int i = 0; i < size; ++i) {
				dest[i] = source[i];
			}
		};

		template <class Class>
		void _fill(unsigned __int32 colour) {
			for (int i = 0; i < size; ++i) {
				((Class*)(data))[i] = colour;
			}
		};

	public:

		// constructor : pass data externally
		inline Surface(PixelFormat _format, void* _data, int _sizeX, int _sizeY, int _rowSpace) : format(_format), data(_data),
			sizeX(_sizeX), sizeY(_sizeY), rowSpace(_rowSpace), dataOwner(false) {

			calcBpp();
			calcMisc();
		};

		// constructor : create data internally
		inline Surface(PixelFormat _format, int _sizeX, int _sizeY) : format(_format), sizeX(_sizeX), sizeY(_sizeY), dataOwner(true) {

			calcBpp();

			switch (bpp) {
				case 32:
					rowSpace = 0; /// no space wasted
					calcMisc();
					data = new unsigned __int32[sizeY * rowSize];
					break;
				case 8:
					rowSpace = (4 - (sizeX % 4)) & 3;
					calcMisc();
					data = new unsigned __int8[sizeY * rowSize];
					break;
			}
		};

		// destructor
		~Surface();

		inline Surface getSubface(int x1, int y1, int x2, int y2) const {
			/// check for possible errors yourself! (for performance reasons)
			void* _data = (char*)data + x1 + ((y1 * rowSize * bpp) >> 3);
			int _sizeX = x2 - x1;
			int _sizeY = y2 - y1;
			int _rowSpace = rowSpace + sizeX - _sizeX;
			return Surface(format, _data, _sizeX, _sizeY, _rowSpace);
		};

		/// copy surface to another buffer; <NOTE> : buffer must be equal or greater in size <!>
		void copyTo(void*);

		/// copy another buffer in this surface; <NOTE> : buffer must be equal or greater in size <!>
		void copyFrom(void*);

		/// fill texture with a colour
		void fill(unsigned __int32);

		inline void* get1stPixel()	const { return data; };
		inline int getSizeX()		const { return sizeX; };
		inline int getSizeY()		const { return sizeY; };
		inline int getSize()		const { return size; };
		inline int getRowSpace()	const { return rowSpace; };
		inline int getRowSize()		const { return rowSize; };
		inline int getFormat()		const { return format; };
		inline int getBpp()			const { return bpp; };
		inline bool isDataOwner()	const { return dataOwner; };
	};
}

#endif