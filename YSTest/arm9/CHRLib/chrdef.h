﻿// CHRLib -> CHRDefinition by Franksoft 2009 - 2010
// CodePage = UTF-8;
// CTime = 2009-11-17 17:52:35;
// UTime = 2010-6-14 18:22;
// Version = 0.1463;


#ifndef INCLUDED_CHRDEF_H_
#define INCLUDED_CHRDEF_H_

// CHRDefinition ：类型定义。

#define CHRLIB_BEGIN	namespace CHRLib {
#define CHRLIB_END		}
#define _CHRLIB_			::CHRLib::
#define _CHRLIB			::CHRLib

#define CHRLIB_BEGIN_NAMESPACE(s)	namespace s {
#define CHRLIB_END_NAMESPACE(s)	}

CHRLIB_BEGIN

typedef unsigned char ubyte_t;
typedef unsigned long usize_t;

// UCS-2 字符类型定义。
typedef unsigned short uchar_t;
typedef signed int uchardiff_t;

/*
typedef struct
{
	u8   length;
	u16* word;
} string;
*/

/*typedef struct
{
	s32 x;
	s32 y;
} POINT;

typedef struct
{
	POINT base;
	POINT end;
} BLOCK;*/

/*
bool  IsInArea(const BLOCK&, const POINT&);
BLOCK CheckBlock(const BLOCK&, const BLOCK&);
*/

//typedef enum {NONE = 0, HOLLOW = 1, SHADOW = 2, BACK_GROUND = 3} FNTS; //FONT Style;
typedef enum {Deg0 = 0, Deg90 = 1, Deg180 = 2, Deg270 = 3} CROT; //Character Rotation;

/*typedef struct
{
	PIXEL Color;
	PIXEL BgColor;
	u8    Align;
	CROT  Rot;
	FNTS  Fx;
	bool  Cut;  //Cut
	u8    WSpc; //Width Space
	u8    HSpc; //Height Space
	FONT* Font;
} CHRSTAT;*/

CHRLIB_END

#endif

