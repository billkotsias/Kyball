// A pixel-surface blitter

#pragma once
#ifndef FANLIB_Blitter_H
#define FANLIB_Blitter_H

namespace FANLib {

	class Surface;
	class Area;

	class Blitter {

	public:

		static const int decimal = 16; /// number of decimal digits; 15 => 17.15 fixed-point int

		enum FX {
			COPY,
			MAX_ALPHA,
			ADD_ALPHA,
		};

		/// generic template-resolving function
		static void draw(const Surface&, const Area&, const Surface&, const Area&, Blitter::FX = Blitter::COPY);

		/// all-in-1 template
		template <class DestFormat, class SourceFormat, void (*plot)(DestFormat&, SourceFormat&)>
		static void _draw(const Surface&, const Area&, const Surface&, const Area&);

		/// "plot" functions
		/// - <copy> : no fx
		template <class DestFormat, class SourceFormat, DestFormat (*convert)(SourceFormat&)>
		inline static void copy(DestFormat& dest, SourceFormat& source) {
			dest = convert(source);
		}

		/// - <maxAlpha> : if source alpha > dest alpha, dest alpha = source alpha!
		inline static void maxAlpha(unsigned __int32& dest, unsigned __int8& source) {
			unsigned __int32 sAlpha = convertARGB_A8(source);
			if (dest < sAlpha) dest = (dest & 0xFFFFFF) | sAlpha;
		}
		inline static void maxAlpha(unsigned __int32& dest, unsigned __int32& source) {
			if (dest < source) dest = (dest & 0xFFFFFF) | (source & 0xFF000000);
		}

		/// - <addAlpha> : add source alpha to dest alpha (clamped at 255)
		inline static void addAlpha(unsigned __int32& dest, unsigned __int8& source) {
			unsigned __int32 alpha = source + (dest >> 24);
			if (alpha > 255) alpha = 255;
			dest = (dest & 0xFFFFFF) | (alpha << 24);
		}
		inline static void addAlpha(unsigned __int32& dest, unsigned __int32& source) {
			unsigned __int8 source8 = source >> 24;
			addAlpha(dest, source8);
		}

		/// conversion between pixel formats
		/// - no conversion required
		template <class Class>
		inline static Class convertNone(Class& source) {
			return source;
		}
		/// - ARGB_8888 <- GREYSCALE_8
		inline static unsigned __int32 convertARGB_GREY8(unsigned __int8& source) {
			return source + (source << 8) + (source << 16) + (255 << 24);
		}
		/// - ARGB_8888 <- A_8
		inline static unsigned __int32 convertARGB_A8(unsigned __int8& source) {
			return source << 24;
		}

	};
}

#endif