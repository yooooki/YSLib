﻿/*
	Copyright (C) by Franksoft 2009 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file Font.h
\ingroup Adaptor
\brief 平台无关的字体库。
\version r7674;
\author FrankHB<frankhb1989@gmail.com>
\since build 296 。
\par 创建时间:
	2009-11-12 22:02:40 +0800;
\par 修改时间:
	2012-03-30 16:36 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::Adaptor::Font;
*/


#ifndef YSL_INC_ADAPTOR_FONT_H_
#define YSL_INC_ADAPTOR_FONT_H_

#include "../Core/yfunc.hpp"
#include "../Core/yobject.h"
#include <string>
#include "../Core/yexcept.h"
#include <ystdex/utility.hpp>

//包含 FreeType2 。

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_CACHE_H

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Drawing)

//前向声明。
class Font;
class FontFamily;
class Typeface;
class FontCache;


/*!
\brief 字体大小。
\since build 277 。
*/
typedef u8 FontSize;
/*!
\brief 字体文件路径。
\since build 277 。
*/
typedef std::string FontPath;
/*!
\brief 字型家族名称。
\since build 277 。
*/
typedef std::string FamilyName;
/*!
\brief 字型样式名称。
\since build 277 。
*/
typedef std::string StyleName;


/*!
\brief 字体样式。
\since build 197 。
*/
enum class FontStyle : u8
{
	Regular = 0, //!< 常规字体。
	Bold = 1, //!< 粗体。
	Italic = 2, //!< 斜体。
	Underline = 4, //!< 下划线。
	Strikeout = 8 //!< 删除线。
};

DefBitmaskOperations(FontStyle, u8)


/*!
\brief 取样式名称。
\post 返回值非空。
\note 无异常抛出。
\since build 277 。
*/
yconstfn const char*
FetchName(FontStyle style) ynothrow
{
	return style == FontStyle::Bold ? "Bold" : (style == FontStyle::Italic
		? "Italic" : (style == FontStyle::Underline ? "Underline" : (style
		== FontStyle::Strikeout ? "Strikeout" : "Regular")));
}


/*!
\brief 字型家族 (Typeface Family) 标识。
\warning 非虚析构。
\since build 145 。
*/
class FontFamily : private noncopyable
{
public:
	typedef map<const StyleName, Typeface*> FaceMap; //!< 字型组索引类型。

	FontCache& Cache;

private:
	FamilyName family_name;

protected:
	FaceMap mFaces; //!< 字型组索引类型。

public:
	/*!
	\brief 使用字体缓存引用和名称构造字型家族。
	*/
	FontFamily(FontCache&, const FamilyName&);

	/*!
	\brief 向字型组和字型组索引添加字型对象。
	\since build 277 。
	*/
	void
	operator+=(Typeface&);
	/*!
	\brief 从字型组和字型组索引中移除指定字型对象。
	\since build 277 。
	*/
	bool
	operator-=(Typeface&);

	DefGetter(const ynothrow, const FamilyName&, FamilyName, family_name)
	/*!
	\brief 取指定样式的字型指针。
	\note 若非 Regular 样式失败则尝试取 Regular 样式的字型指针。
	*/
	Typeface*
	GetTypefacePtr(FontStyle) const;
	/*!
	\brief 取指定样式名称的字型指针。
	*/
	Typeface*
	GetTypefacePtr(const StyleName&) const;
};


/*!
\brief 字型标识。
\warning 非虚析构。
\since build 145 。
*/
class Typeface : private noncopyable
{
	friend ::FT_Error
	simpleFaceRequester(::FTC_FaceID, ::FT_Library, ::FT_Pointer, ::FT_Face*);

//	static const ::FT_Matrix MNormal, MOblique;
public:
	const FontPath Path;

private:
	FontFamily* pFontFamily;
//	::FT_Face face;
//	bool bBold, bOblique, bUnderline;
	StyleName style_name;

	::FT_Long face_index;
	::FT_Int cmap_index;
/*	FT_Long nGlyphs;
	FT_UShort uUnitPerEM;
	FT_Int nCharmaps;
	FT_Long lUnderlinePos;
	FT_Matrix matrix;
	vector<u8> fixSizes;*/

public:
	/*!
	\brief 使用字体缓存引用在指定字体文件路径读取指定索引的字型并构造对象。
	\post <tt>bool(pFontFamily)</tt> 。
	*/
	Typeface(FontCache&, const FontPath&, u32 = 0
		/*, const bool bb = false,
		const bool bi = false, const bool bu = false*/);

	/*!
	\brief 比较：相等关系。
	*/
	bool
	operator==(const Typeface&) const;
	/*!
	\brief 比较：严格递增偏序关系。
	*/
	bool
	operator<(const Typeface&) const;

	DefGetterMem(const ynothrow, FamilyName, FamilyName, GetFontFamily())
	/*!
	\brief 取字型家族。
	\since build 278 。
	*/
	DefGetter(const ynothrow, const FontFamily&, FontFamily, *pFontFamily)
	DefGetter(const ynothrow, const StyleName&, StyleName, style_name)
	/*!
	\brief 取字符映射索引号。
	\since build 278 。
	*/
	DefGetter(const ynothrow, ::FT_Int, CMapIndex, cmap_index)
};


/*!
\brief 取默认字型引用。
\throw LoggedEvent 记录异常事件。
\note 仅抛出以上异常。
\since build 194 。
*/
const Typeface&
FetchDefaultTypeface() ythrow(LoggedEvent);


/*!
\brief 字符位图。
\warning 非虚析构。
\warning 若为空时调用成员函数时行为未定义。
\since build 147 。
*/
class CharBitmap
{
public:
	typedef ::FTC_SBit NativeType;
	typedef ::FT_Byte* BufferType;
	typedef ::FT_Byte ScaleType;
	typedef ::FT_Char SignedScaleType;

private:
	NativeType bitmap;

public:
	/*!
	\brief 使用本机类型对象构造字符位图对象。
	*/
	yconstfn
	CharBitmap(const NativeType&);

	yconstfn DefCvt(const ynothrow, NativeType, bitmap)

	yconstfn DefGetter(const ynothrow, BufferType, Buffer, bitmap->buffer)
	yconstfn DefGetter(const ynothrow, ScaleType, Width, bitmap->width)
	yconstfn DefGetter(const ynothrow, ScaleType, Height, bitmap->height)
	yconstfn DefGetter(const ynothrow, SignedScaleType, Left, bitmap->left)
	yconstfn DefGetter(const ynothrow, SignedScaleType, Top, bitmap->top)
	yconstfn DefGetter(const ynothrow, SignedScaleType, XAdvance,
		bitmap->xadvance)
	yconstfn DefGetter(const ynothrow, SignedScaleType, YAdvance,
		bitmap->yadvance)
};

yconstfn
CharBitmap::CharBitmap(const NativeType& b)
	: bitmap(b)
{}


/*!
\brief 字体缓存。
\warning 非虚析构。
\since build 209 。
*/
class FontCache : private noncopyable,
	private OwnershipTag<Typeface>, private OwnershipTag<FontFamily>
{
	friend class Typeface;
	/*!
	\brief 友元类：访问 scaler 等对象。
	\since build 280 。
	*/
	friend class Font;

public:
	/*!
	\brief 字体文件路径组类型。

	字体文件路径映射至此文件中的字型数。
	\since build 277 。
	*/
	typedef map<FontPath, ::FT_Long> PathMap;
	typedef set<Typeface*, ystdex::deref_comp<const Typeface>>
		FaceSet; //!< 字型组类型。
	typedef map<FamilyName, FontFamily*> FamilyMap; //!< 字型家族组索引类型。

	/*!
	\brief 字形缓冲区大小。
	\note 单位为字节。
	\since build 277 。
	*/
	static yconstexpr size_t DefaultGlyphCacheSize = 128U << 10;

private:
	::FT_Library library; //!< 库实例。
	::FTC_Manager manager; //!< 内存管理器实例。
	::FTC_CMapCache cmapCache;
	::FTC_SBitCache sbitCache;

protected:
	/*
	\brief 字体文件路径映射。
	\since build 277 。
	*/
	PathMap mPaths;
	FaceSet sFaces; //!< 字型组。
	FamilyMap mFamilies; //!< 字型家族组索引。

	Typeface* pDefaultFace; //!< 默认字型指针。

public:
	/*!
	\brief 构造：读取指定路径的字体文件并分配指定大小的缓存空间。
	*/
	explicit
	FontCache(const_path_t, size_t = DefaultGlyphCacheSize);
	/*!
	\brief 析构：释放空间。
	*/
	~FontCache();

public:
	DefGetter(const ynothrow, const PathMap&, Paths, mPaths) \
		//!< 取字体文件路径映射。
	/*!
	\brief 取字型组。
	\since build 281 。
	*/
	DefGetter(const ynothrow, const FaceSet&, Faces, sFaces)
	DefGetter(const ynothrow, const FamilyMap&, FamilyIndices, mFamilies) \
		//!< 取字型家族组索引。
//	Font*
//	GetFontPtr() const;
	/*!
	\brief 取指定名称的字型家族指针。
	*/
	const FontFamily*
	GetFontFamilyPtr(const FamilyName&) const;
	/*!
	\brief 取默认字型指针。
	\throw LoggedEvent 记录异常事件。
	\note 仅抛出以上异常。
	*/
	const Typeface*
	GetDefaultTypefacePtr() const ythrow(LoggedEvent);
//	Typeface*
//	GetTypefacePtr(u16) const; //!< 取字型组储存的指定索引的字型指针。
	/*!
	\brief 取指定名称的字型指针。
	*/
	const Typeface*
	GetTypefacePtr(const FamilyName&, const StyleName&) const;

private:
	/*!
	\brief 取当前本机类型字型。
	\since build 280 。
	*/
	::FT_Face
	GetNativeFace(Typeface*) const;

	/*!
	\brief 向字型家族组添加字型对象。
	\since build 277 。
	*/
	void
	operator+=(FontFamily&);
	/*!
	\brief 向字型组添加字型对象。
	\since build 277 。
	*/
	void
	operator+=(Typeface&);

	/*!
	\brief 从字型家族组中移除指定字型对象。
	\since build 277 。
	*/
	bool
	operator-=(FontFamily&);
	/*!
	\brief 从字型组中移除指定字型对象。
	\since build 277 。
	*/
	bool
	operator-=(Typeface&);

public:
	/*!
	\brief 清除缓存。
	*/
	void
	ClearCache();

private:
	/*!
	\brief 清除容器。
	*/
	void
	ClearContainers();

public:
	/*!
	\brief 从字体文件组中载入字型信息。
	*/
	void
	LoadTypefaces();

private:
	/*!
	\brief 从路径指定的字体文件中的载入指定数量的字型信息。
	\note 若指定字体文件不在字体文件组中则先按路径添加此文件。
	\since build 277 。
	*/
	void
	LoadTypefaces(const FontPath&, size_t);

public:
	/*!
	\brief 按路径添加字体文件并载入字型信息。
	*/
	bool
	LoadFontFile(const FontPath&);

	/*!
	\brief 初始化默认字型。
	*/
	void
	InitializeDefaultTypeface();

	/*
	!\brief 清除字形缓存。
	*/
	PDefH(void, ResetGlyphCache)
		ImplRet(FTC_Manager_Reset(manager))
};


/*!
\brief 字体：字模，包含字型、样式和大小。
\warning 非虚析构。
\since build 145 。
*/
class Font
{
public:
	static yconstexpr FontSize DefaultSize = 12,
		MinimalSize = 4, MaximalSize = 96;

private:
	mutable ::FTC_ScalerRec scaler;
	/*!
	\brief 字体样式。
	\since build 297 。
	*/
	FontStyle style;

public:
	/*!
	\brief 构造指定字型家族、大小和样式的字体对象。
	*/
	explicit
	Font(const FontFamily& = FetchDefaultTypeface().GetFontFamily(),
		FontSize = DefaultSize, FontStyle = FontStyle::Regular);

	DefPred(const ynothrow, Bold, bool(style & FontStyle::Bold))
	DefPred(const ynothrow, Italic, bool(style & FontStyle::Italic))
	DefPred(const ynothrow, Underline, bool(style & FontStyle::Underline))
	DefPred(const ynothrow, Strikeout, bool(style & FontStyle::Strikeout))

	/*!
	\brief 取跨距。
	\since build 280 。
	*/
	s8
	GetAdvance(ucs4_t, FTC_SBit = nullptr) const;
	/*!
	\brief 取升部。
	\since build 280 。
	*/
	s8
	GetAscender() const;
	/*!
	\brief 取降部。
	\since build 280 。
	*/
	s8
	GetDescender() const;
	DefGetterMem(const ynothrow, const FamilyName&, FamilyName,
		GetFontFamily())
	DefGetter(const ynothrow, FontCache&, Cache, GetFontFamily().Cache)
	DefGetterMem(const ynothrow, const FontFamily&, FontFamily, GetTypeface())
	DefGetter(const ynothrow, FontSize, Size, FontSize(scaler.height))
	DefGetter(const ynothrow, FontStyle, style, style)
	/*!
	\brief 取当前字型和大小渲染的指定字符的字形。
	\param c 指定需要被渲染的字符。
	\param flags FreeType 渲染标识。
	\warning flags 可能被移除，应仅用于内部实现。
	\since build 280 。
	*/
	CharBitmap
	GetGlyph(ucs4_t c, ::FT_UInt flags = FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL)
		const;
	/*!
	\brief 取字体对应的字符高度。
	\since build 280 。
	*/
	FontSize
	GetHeight() const ynothrow;
	DefGetter(const ynothrow, StyleName, StyleName, FetchName(style))

private:
	/*!
	\brief 取内部信息。
	\since build 280 。
	*/
	::FT_SizeRec&
	GetInternalInfo() const;

public:
	/*!
	\brief 取字型引用。
	\since build 280 。
	*/
	DefGetter(const ynothrow, Typeface&, Typeface,
		*static_cast<Typeface*>(scaler.face_id))

	/*!
	\brief 设置字体大小。
	*/
	void
	SetSize(FontSize = DefaultSize);
	/*!
	\brief 设置样式。
	\since build 280 。
	*/
	bool
	SetStyle(FontStyle);
};

YSL_END_NAMESPACE(Drawing)

YSL_END

#endif

