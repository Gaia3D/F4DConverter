#pragma once

namespace gaia3d
{
	typedef unsigned long ColorU4;

	enum ColorMode {NoColor, SingleColor, ColorsOnVertices};
}

///< 가장 최하위 바이트를 가지고 오는 매크로(R채널)
#define LOWBYTE(w)			((unsigned char)(((unsigned long)(w)) & 0xff))
///< RGB값을 입력해 mesh의 색을 결정하는 매크로
#define MakeColorU4(r,g,b)	((gaia3d::ColorU4)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned long)(unsigned char)(b))<<16)))
///< R채널 값을 가져오는 매크로
#define GetRedValue(rgb)	(LOWBYTE(rgb))
///< G채널 값을 가져오는 매크로
#define GetGreenValue(rgb)	(LOWBYTE(((unsigned short)(rgb)) >> 8))
///< B채널 값을 가져오는 매크로
#define GetBlueValue(rgb)	(LOWBYTE((rgb)>>16))
///< 기본 값을 설정하는 매크로. 회색.
#define DefaultColor		MakeColorU4(204, 204, 204)