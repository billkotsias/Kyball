#include "gfx\Blitter.h"

#include "gfx\Surface.h"
#include "gfx\Area.h"
#include <math.h>

namespace FANLib {

	template <class DestFormat, class SourceFormat, void (*plot)(DestFormat&, SourceFormat&)>
	void Blitter::_draw(const Surface& dest, const Area& dArea, const Surface& source, const Area& sArea) {

		/// cases of total clipping
		if (dArea.x1() >= dest.getSizeX() || dArea.y1() >= dest.getSizeY() ||
			dArea.x2() < 0 || dArea.y2() < 0) return;

		/// "source" steppers - always the same, regardless "destination's" clipping
		double _xStep = sArea.width() / dArea.width();
		double yStep = sArea.height() / dArea.height();
		int xStep = (int)(_xStep * (1<<decimal) );

		/// left-clipping
		int dXStart;	/// leftmost column in destination surface
		int sXStart;	/// leftmost column in source surface (fixed-point number)
		int dAreax1 = (int)dArea.x1(); /// only int is needed, so cache it

		if (dAreax1 < 0) {
			dXStart = 0;
			sXStart = -dAreax1 * xStep;
		} else {
			dXStart = dAreax1;
			sXStart = 0;
		}

		/// right-clipping
		int dXEnd;	/// rightmost column in destination surface
		int dAreax2 = (int)dArea.x2();

		if (dAreax2 >= dest.getSizeX()) {
			dXEnd = dest.getSizeX();
		} else {
			dXEnd = dAreax2;
		}

		/// top-clipping
		int dYStart;	/// topmost row in destination surface
		int sYStart;	/// topmost row in source surface (fixed-point number)
		int dAreay1 = (int)dArea.y1();

		if (dAreay1 < 0) {
			dYStart = 0;
			sYStart = (int)(-dAreay1 * yStep);
		} else {
			dYStart = dAreay1;
			sYStart = 0;
		}

		/// bottom-clipping
		int dYEnd;	/// bottom row in destination surface
		int dAreay2 = (int)dArea.y2();

		if (dAreay2 >= dest.getSizeY()) {
			dYEnd = dest.getSizeY();
		} else {
			dYEnd = dAreay2;
		}

		/// pointer to top-left pixel in destination
		DestFormat* dData;
		dData = (DestFormat*)dest.get1stPixel();
		dData += dYStart * dest.getRowSize();

		/// pointer to top-left pixel in source - it is assumed that the "source area" is within "source's surface"
		SourceFormat* _sData;
		_sData = (SourceFormat*)source.get1stPixel();
		_sData += (int)(sArea.x1() + _xStep/2.);
		_sData += ((int)(sArea.y1() + yStep/2.)) * source.getRowSize();

		/// drawing loop
		for (int k = 0; k < dYEnd - dYStart; ++k) {

			SourceFormat* sData = _sData + ((int)(yStep * k + sYStart)) * source.getRowSize();
			int sPtr = sXStart;

			/// draw row
			for (int i = dXStart; i < dXEnd; ++i) {
				plot(dData[i], sData[sPtr >> decimal]);
				sPtr += xStep;
			}

			dData += dest.getRowSize();
		}
	}

	void Blitter::draw(const Surface& dest, const Area& dArea, const Surface& source, const Area& sArea, Blitter::FX fx) {

		/// resolve input => output format case
		switch (dest.getFormat()) {

			case Surface::ARGB_8888: /// <destination>

				switch (source.getFormat()) {
					
					case Surface::ARGB_8888:	/// <source>

						switch (fx) {
							case Blitter::COPY:
								_draw<unsigned __int32, unsigned __int32,
									&copy<unsigned __int32, unsigned __int32, &convertNone<unsigned __int32> > >(dest, dArea, source, sArea);
								return;
							case Blitter::MAX_ALPHA:
								_draw<unsigned __int32, unsigned __int32, &maxAlpha >(dest, dArea, source, sArea);
								return;
							case Blitter::ADD_ALPHA:
								_draw<unsigned __int32, unsigned __int32, &addAlpha >(dest, dArea, source, sArea);
								return;
							default:
								return;
						}

					case Surface::GREYSCALE_8:	/// <source>

						switch (fx) {
							case Blitter::COPY:
								_draw<unsigned __int32, unsigned __int8, &copy<unsigned __int32, unsigned __int8, &convertARGB_GREY8 > >(dest, dArea, source, sArea);
								return;
							case Blitter::MAX_ALPHA:
								_draw<unsigned __int32, unsigned __int8, &maxAlpha >(dest, dArea, source, sArea); /// GREYSCALE_8 is read as <A_8>
								return;
							case Blitter::ADD_ALPHA:
								_draw<unsigned __int32, unsigned __int8, &addAlpha >(dest, dArea, source, sArea); /// GREYSCALE_8 is read as <A_8>
								return;
							default:
								return;
						}
						

					case Surface::A_8:			/// <source>

						switch (fx) {
							case Blitter::COPY:
								_draw<unsigned __int32, unsigned __int8, &copy<unsigned __int32, unsigned __int8, &convertARGB_A8 > >(dest, dArea, source, sArea);
								return;
							case Blitter::MAX_ALPHA:
								_draw<unsigned __int32, unsigned __int8, &maxAlpha >(dest, dArea, source, sArea);
								return;
							case Blitter::ADD_ALPHA:
								_draw<unsigned __int32, unsigned __int8, &addAlpha >(dest, dArea, source, sArea);
								return;
							default:
								return;
						}

					default:
						return;

				}

			default:
				return;
		}
	}

}