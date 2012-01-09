﻿/*
	Copyright (C) by Franksoft 2009 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file TextRenderer.cpp
\ingroup Service
\brief 文本渲染。
\version r6889;
\author FrankHB<frankhb1989@gmail.com>
\since build 275 。
\par 创建时间:
	2009-11-13 00:06:05 +0800;
\par 修改时间:
	2012-01-05 15:54 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::Service::TextRenderer;
*/


#include "YSLib/Service/TextRenderer.h"
#include "YSLib/Service/yblit.h"

using namespace ystdex;

YSL_BEGIN

using namespace Drawing;
using namespace Text;

YSL_BEGIN_NAMESPACE(Drawing)

void
TextRenderer::ClearLine(u16 l, SDst n)
{
	const auto& g(GetContext());
	const auto h(g.GetHeight());

	if(g.IsValid() && l < h)
	{
		if(n == 0 || l + n > h)
			n = h - l;
		ClearPixel(g[l], g.GetWidth() * n);
	}
}


TextRegion::TextRegion()
	: GTextRendererBase<TextRegion>(), TextState(), BitmapBufferEx()
{
	InitializeFont();
}
TextRegion::TextRegion(Drawing::Font& fnt)
	: GTextRendererBase<TextRegion>(), TextState(fnt), BitmapBufferEx()
{
	InitializeFont();
}
TextRegion::TextRegion(FontCache& fc)
	: GTextRendererBase<TextRegion>(), TextState(fc), BitmapBufferEx()
{
	InitializeFont();
}

void
TextRegion::InitializeFont()
{
	Font.SetSize(Font::DefaultSize);
	Font.UpdateSize();
	ResetPen();
}

void
TextRegion::ClearLine(u16 l, SDst n)
{
	const auto& g(GetContext());

	if(l > g.GetHeight())
		return;
	if(!n)
		--n;
	if(g.IsValid() && pBufferAlpha)
	{
		const u32 t((l + n > g.GetHeight() ? g.GetHeight() - l : n)
			* g.GetWidth());

		yunseq(ClearPixel(g[l], t),
			ClearPixel(&pBufferAlpha[l * g.GetWidth()], t));
	}
}

void
TextRegion::ClearTextLine(u16 l)
{
	auto& ts(GetTextState());
	SDst h(GetTextLineHeightExOf(ts));

	ClearLine(ts.Margin.Top + h * l, h);
}

void
TextRegion::Scroll(ptrdiff_t n)
{
	if(GetHeight() > Margin.Bottom)
		Scroll(n, GetHeight() - Margin.Bottom);
}
void
TextRegion::Scroll(ptrdiff_t n, SDst h)
{
	if(pBuffer && pBufferAlpha)
	{
		const s32 t(((h + Margin.Bottom > GetHeight()
			? GetHeight() - Margin.Bottom : h)
			- Margin.Top - abs(n)) * GetWidth());

		if(n && t > 0)
		{
			u32 d(Margin.Top), s(d);

			if(n > 0)
				d += n;
			else
				s -= n;
			yunseq(s *= GetWidth(), d *= GetWidth());
			yunseq(ystdex::pod_move_n(&pBuffer[s], t, &pBuffer[d]),
				ystdex::pod_move_n(&pBufferAlpha[s], t, &pBufferAlpha[d]));
		}
	}
}


void
DrawClippedText(const Graphics& g, const Rect& mask, TextState& ts,
	const String& str)
{
	TextRenderer tr(ts, g, mask);

	PrintLine(tr, str);
}
void
DrawClippedText(const Graphics& g, const Rect& mask, const Rect& r,
	const String& str, const Padding& m, Color color)
{
	TextState ts;

	(ts.Font = Font()).Update(); //设置默认字体。
	ts.ResetForBounds(r, g.GetSize(), m);
	ts.Color = color;
	DrawClippedText(g, mask, ts, str);
}

void
DrawText(const Graphics& g, TextState& ts, const String& str)
{
	DrawClippedText(g, Rect(Point::Zero, g.GetSize()), ts, str);
}
void
DrawText(const Graphics& g, const Rect& r, const String& str, const Padding& m,
	Color color)
{
	DrawClippedText(g, Rect(Point::Zero, g.GetSize()), r, str, m, color);
}
void
DrawText(TextRegion& tr, const Graphics& g, const Point& pt, const Size& s,
	const String& str)
{
	PutLine(tr, str);
	BlitTo(g.GetBufferPtr(), tr, g.GetSize(), Point::Zero, pt, s);
}

YSL_END_NAMESPACE(Drawing)

YSL_END

