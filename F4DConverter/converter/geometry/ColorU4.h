#pragma once

namespace gaia3d
{
	typedef unsigned long ColorU4;

	enum ColorMode {NoColor, SingleColor, ColorsOnVertices};
}

#define LOWBYTE(w)			((unsigned char)(((unsigned long)(w)) & 0xff))
#define MakeColorU4(r,g,b)	((gaia3d::ColorU4)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned long)(unsigned char)(b))<<16)))
#define GetRedValue(rgb)	(LOWBYTE(rgb))
#define GetGreenValue(rgb)	(LOWBYTE(((unsigned short)(rgb)) >> 8))
#define GetBlueValue(rgb)	(LOWBYTE((rgb)>>16))
#define DefaultColor		MakeColorU4(204, 204, 204)