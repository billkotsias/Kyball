// Math related stuff
#pragma once
#ifndef FANLIB_Math_H
#define FANLIB_Math_H

#include <limits>

#define Infinite std::numeric_limits<double>::infinity()
#define NaN std::numeric_limits<double>::quiet_NaN()
#define Infinitef std::numeric_limits<float>::infinity()
#define NaNf std::numeric_limits<float>::quiet_NaN()
#define Pi 3.14159265358979323846

namespace FANLib {

	class Math {

	public:

		/// confine a value within its bounds
		/// => value = the one to confine
		///	   min = the minimum bound
		///	   max = the maximum bound
		template <class Class> inline static Class bound(Class value, const Class min, const Class max) {
			if (value > max) {
				value = max;
			} else if (value < min) {
				value = min;
			}
			return value;
		}

		/// symmetrically confine a value within its bounds
		/// => value = the one to confine
		///	   bound = the maximum bound, the symmetrical minimum one is assumed to be '- bound'
		template <class Class> inline static Class bound(Class value, const Class bound) {
			if (value > bound) {
				value = bound;
			} else if (value < - bound) {
				value = - bound;
			}
			return value;
		}

		/// round a number to its nearest integer
		/// => value = the one to round
		template <class Class> inline static Class round(Class value) {
			if (value > 0) {
				value += 0.5;
			} else if (value < 0) {
				value -= 0.5;
			}
			return (int)value;
		}

		/**
		* Find the greatest common divisor of 2 numbers
		* See http://en.wikipedia.org/wiki/Greatest_common_divisor
		*
		* @param[in] a First number
		* @param[in] b Second number
		* @return greatest common divisor
		*/
		inline static unsigned gcd ( unsigned a, unsigned b )
		{
			unsigned c;
			while ( a != 0 )
			{
				c = a;
				a = b%a;
				b = c;
			}
			return b;
		}
		/**
		* Find the least common multiple of 2 numbers
		* See http://en.wikipedia.org/wiki/Least_common_multiple
		*
		* @param[in] a First number
		* @param[in] b Second number
		* @return least common multiple
		*/
		inline static unsigned lcm(const unsigned &a, const unsigned &b)
		{
			return a * b / gcd(a, b);
		}
		/**
		* Find the greatest common divisor of an array of numbers
		* See http://en.wikipedia.org/wiki/Greatest_common_divisor
		*
		* @param[in] n Pointer to an array of number
		* @param[in] size Size of the array
		* @return greatest common divisor
		*/
		inline static unsigned gcd(const unsigned* const n, const unsigned size)
		{
			unsigned last_gcd, i;
			if(size < 2) return 0;
			last_gcd = gcd(n[0], n[1]);
			for(i=2; i < size; i++)
			{
				last_gcd = gcd(last_gcd, n[i]);
			}
			return last_gcd;
		}
		/**
		* Find the least common multiple of an array of numbers
		* See http://en.wikipedia.org/wiki/Least_common_multiple
		*
		* @param[in] n Pointer to an array of number
		* @param[in] size Size of the array
		* @return least common multiple
		*/
		inline static unsigned lcm(const unsigned* const n, const unsigned size)
		{
			unsigned last_lcm, i;
			if(size < 2) return 0;
			last_lcm = lcm(n[0], n[1]);
			for(i=2; i < size; i++)
			{
				last_lcm = lcm(last_lcm, n[i]);
			}
			return last_lcm;
		}

		template<class Cont> inline static unsigned lcm(Cont cont)
		{
			unsigned last_lcm, i;

			if(cont.size() < 2) return 0;
			last_lcm = lcm(cont.at(0), cont.at(1));
			for( i = cont.size()-1; i >= 2 ; --i)
			{
				last_lcm = lcm(last_lcm, cont.at(i));
			}
			return last_lcm;
		}
	};
}

#endif // FANLIB_Math_H