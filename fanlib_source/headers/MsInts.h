#pragma once
#ifndef FANLIB_MSINTS_H
#define FANLIB_MSINTS_H

// MSVC fundamental types used by the blitter. They must stay macros so
// "unsigned __int32" is still a valid type.
#if !defined(_MSC_VER)
#define __int8 char
#define __int16 short
#define __int32 int
#define __int64 long long
#endif

#endif
