// A rectangle vector area in a surface

#pragma once
#ifndef FANLIB_Area_H
#define FANLIB_Area_H

namespace FANLib {

	class Area {

	protected:

		double X1;
		double Y1;
		double X2;
		double Y2;
		double WIDTH;
		double HEIGHT;

	public:

		inline Area(double _x1, double _y1, double _x2, double _y2) : X1(_x1), Y1(_y1), X2(_x2), Y2(_y2) {
			WIDTH = X2 - X1;
			HEIGHT = Y2 - Y1;
		};

		inline double x1() const { return X1; };
		inline double x2() const { return X2; };
		inline double y1() const { return Y1; };
		inline double y2() const { return Y2; };
		inline double width() const { return WIDTH; };
		inline double height() const { return HEIGHT; };
	};
}

#endif