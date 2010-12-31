﻿/*
	Copyright (C) by Franksoft 2009 - 2010.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ygui.cpp
\ingroup Shell
\brief 平台无关的图形用户界面实现。
\version 0.3262;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2009-11-16 20:06:58 + 08:00;
\par 修改时间:
	2010-12-30 16:11 + 08:00;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::Shell::YGUI;
*/


#include "ygui.h"
//#include <stack>

YSL_BEGIN

using namespace Drawing;
using namespace Components;
using namespace Components::Controls;
using namespace Components::Widgets;

YSL_BEGIN_NAMESPACE(Components)

YSL_BEGIN_NAMESPACE(Controls)

IWidget*
GetCursorWidgetPtr(GHHandle<YGUIShell> hShl, YDesktop& d, const Point& pt)
{
	HWND hWnd(hShl->GetTopWindowHandle(d, pt));

	return hWnd ? hWnd->GetTopWidgetPtr(pt) : NULL;
}

IVisualControl*
GetFocusedObjectPtr(YDesktop& d)
{
	IVisualControl* p(d.GetFocusingPtr()), *q(NULL);
	IUIBox* pCon;

	while(p && (pCon = dynamic_cast<IUIBox*>(p))
		&& (q = pCon->GetFocusingPtr()))
		p = q;
	return p;
}

void
RequestFocusCascade(IVisualControl& c)
{
	IVisualControl* p(&c);
	EventArgs e;

	do
	{
		p->RequestFocus(e);
	}while((p = dynamic_cast<IVisualControl*>(p->GetContainerPtr())));
}

void
ReleaseFocusCascade(IVisualControl& c)
{
	IVisualControl* p(&c);
	EventArgs e;

	do
	{
		p->ReleaseFocus(e);
	}while((p = dynamic_cast<IVisualControl*>(p->GetContainerPtr())));
}


//记录输入保持状态。

YSL_BEGIN_NAMESPACE(InputStatus)

HeldStateType KeyHeldState(Free);
HeldStateType TouchHeldState(Free);
Vec DraggingOffset(Vec::FullScreen);
Timers::YTimer HeldTimer(1000, false);
Point VisualControlLocation(Point::FullScreen);
Point LastVisualControlLocation(Point::FullScreen);


bool
RepeatHeld(HeldStateType& s,
	Timers::TimeSpan InitialDelay, Timers::TimeSpan RepeatedDelay)
{
	//三状态自动机。
	switch(s)
	{
	case Free:
		/*
		必须立即转移状态，否则 HeldTimer.Activate() 会使 HeldTimer.Refresh()
		始终为 false ，导致状态无法转移。
		*/
		s = Pressed;
		HeldTimer.SetInterval(InitialDelay); //初始键按下延迟。
		Activate(HeldTimer);
		break;

	case Pressed:		
	case Held:
		if(HeldTimer.Refresh())
		{
			if(s == Pressed)
			{
				s = Held;
				HeldTimer.SetInterval(RepeatedDelay); //重复键按下延迟。
			}
			return true;
		}
		break;
	}
	return false;
}

void
ResetHeldState(HeldStateType& s)
{
	Deactivate(HeldTimer);
	s = Free;
}

YSL_END_NAMESPACE(InputStatus)

namespace
{
	using namespace InputStatus;

	//独立焦点指针：即时输入（按下）状态所在控件指针。
	IVisualControl* p_TouchDown(NULL);
	IVisualControl* p_KeyDown(NULL);

	bool bEntered(false); //记录指针是否在控件内部。

	namespace ExOp
	{
		typedef enum
		{
			NoOp = 0,
			TouchUp = 1,
			TouchDown = 2,
			TouchHeld = 3
		} ExOpType;
	};
	ExOp::ExOpType ExtraOperation(ExOp::NoOp);

	void
	TryEnter(IVisualControl& c, TouchEventArgs& e)
	{
		if(!bEntered && p_TouchDown == &c)
		{
			FetchEvent<EControl::Enter>(c)(c, e);
			bEntered = true;
		}
	}
	void
	TryLeave(IVisualControl& c, TouchEventArgs& e)
	{
		if(bEntered && p_TouchDown == &c)
		{
			FetchEvent<EControl::Leave>(c)(c, e);
			bEntered = false;
		}
	}

	void ResetTouchHeldState()
	{
		ResetHeldState(TouchHeldState);
		DraggingOffset = Vec::FullScreen;
	}

	IVisualControl*
	GetTouchedVisualControlPtr(IUIBox& con, Point& pt)
	{
		using namespace ExOp;

		IUIBox* pCon(&con);
		IVisualControl* p;

		while((p = pCon->GetTopVisualControlPtr(pt)))
		{
			pt -= p->GetLocation();
			if(!(pCon = dynamic_cast<IUIBox*>(p)))
				break;
			if(ExtraOperation == TouchDown)
			{
				pCon->RequestToTop();
				pCon->ClearFocusingPtr();
			}
		}
		if(!p)
			p = dynamic_cast<IVisualControl*>(pCon);
		if(ExtraOperation == TouchHeld)
		{
			if(p_TouchDown != p)
			{
				if(p_TouchDown)
				{
					if(p)
						pt += LocateForWidget(*p, *p_TouchDown);
					if(bEntered)
					{
						TouchEventArgs e(pt);

						TryLeave(*p_TouchDown, e);
					}
				}
				return p_TouchDown;
			}
			else if(!bEntered)
			{
				TouchEventArgs e(pt);

				TryEnter(*p, e);
			}
		}
		return p;
	}

	IVisualControl*
	GetFocusedEnabledVisualControlPtr(IVisualControl* p)
	{
		return p && p->IsEnabled() ? p : NULL;
	}

	inline IVisualControl*
	GetFocusedEnabledVisualControlPtr(YDesktop& d)
	{
		return GetFocusedEnabledVisualControlPtr(GetFocusedObjectPtr(d));
	}

	bool
	ResponseKeyUpBase(IVisualControl& c, KeyEventArgs& e)
	{
		ResetHeldState(KeyHeldState);
		if(p_KeyDown == &c && KeyHeldState == Free)
			FetchEvent<EControl::KeyPress>(c)(c, e);
		FetchEvent<EControl::KeyUp>(c)(c, e);
		FetchEvent<EControl::Leave>(c)(c, e);
		p_KeyDown = NULL;
		return true;
	}
	bool
	ResponseKeyDownBase(IVisualControl& c, KeyEventArgs& e)
	{
		p_KeyDown = &c;
		FetchEvent<EControl::Enter>(c)(c, e);
		FetchEvent<EControl::KeyDown>(c)(c, e);
		return true;
	}
	bool
	ResponseKeyHeldBase(IVisualControl& c, KeyEventArgs& e)
	{
		if(p_KeyDown != &c)
		{
			ResetHeldState(KeyHeldState);
			return false;
		}
		FetchEvent<EControl::KeyHeld>(c)(c, e);
		return true;
	}

	bool
	ResponseTouchUpBase(IVisualControl& c, TouchEventArgs& e)
	{
		ResetTouchHeldState();
		if(p_TouchDown == &c && TouchHeldState == Free)
			FetchEvent<EControl::Click>(c)(c, e);
		FetchEvent<EControl::TouchUp>(c)(c, e);
		TryLeave(c, e);
		p_TouchDown = NULL;
		return true;
	}
	bool
	ResponseTouchDownBase(IVisualControl& c, TouchEventArgs& e)
	{
		p_TouchDown = &c;
		TryEnter(c, e);
		FetchEvent<EControl::TouchDown>(c)(c, e);
		return true;
	}
	bool
	ResponseTouchHeldBase(IVisualControl& c, TouchEventArgs& e)
	{
	/*	if(p_TouchDown != &c)
		{
			ResetTouchHeldState();
			return false;
		}*/
		if(p_TouchDown == &c)
		{
			FetchEvent<EControl::TouchHeld>(c)(c, e);
			return true;
		}
		return false;
	}

	bool
	ResponseKeyBase(YDesktop& d, HKeyCallback f)
	{
		IVisualControl* const p(GetFocusedEnabledVisualControlPtr(d));

		return f(p ? *p : d);
	}

	bool
	ResponseTouchBase(IUIBox& con, HTouchCallback f)
	{
		VisualControlLocation = f;
		IVisualControl* const pVC(GetTouchedVisualControlPtr(con, f));

		return pVC ? f(*pVC, f) : false;
	}
}

bool
ResponseKeyUp(YDesktop& d, KeyEventArgs& e)
{
	return ResponseKeyBase(d, HKeyCallback(e, ResponseKeyUpBase));
}
bool
ResponseKeyDown(YDesktop& d, KeyEventArgs& e)
{
	return ResponseKeyBase(d, HKeyCallback(e, ResponseKeyDownBase));
}
bool
ResponseKeyHeld(YDesktop& d, KeyEventArgs& e)
{
	return ResponseKeyBase(d, HKeyCallback(e, ResponseKeyHeldBase));
}

bool
ResponseTouchUp(IUIBox& con, TouchEventArgs& e)
{
	ExtraOperation = ExOp::TouchUp;
	return ResponseTouchBase(con, HTouchCallback(e, ResponseTouchUpBase));
}
bool
ResponseTouchDown(IUIBox& con, TouchEventArgs& e)
{
	ExtraOperation = ExOp::TouchDown;
	return ResponseTouchBase(con, HTouchCallback(e, ResponseTouchDownBase));
}
bool
ResponseTouchHeld(IUIBox& con, TouchEventArgs& e)
{
	ExtraOperation = ExOp::TouchHeld;
	return ResponseTouchBase(con, HTouchCallback(e, ResponseTouchHeldBase));
}

YSL_END_NAMESPACE(Controls)

YSL_END_NAMESPACE(Components)

YSL_BEGIN_NAMESPACE(Drawing)

namespace
{
	void
	DrawWidgetBounds(IWindow& w, const Point& p, const Size& s, Color c)
	{
		DrawRect(w.GetContext(), p, Size(s - Vec(1, 1)), c);
	}
}

void
DrawWindowBounds(IWindow* pWnd, Color c)
{
	YAssert(pWnd, "Window pointer is null.");

	DrawWidgetBounds(*pWnd, Point::Zero, pWnd->GetSize(), c);
}

void
DrawWidgetBounds(IWidget& w, Color c)
{
	IWindow* pWnd(FetchDirectWindowPtr(w));

	if(pWnd)
		DrawWidgetBounds(*pWnd, LocateOffset(&w, Point::Zero, pWnd),
			w.GetSize(), c);
}

void
DrawWidgetOutline(IWidget& w, Color c)
{
	IWindow* pWnd(FetchWindowPtr(w));

	if(pWnd)
		DrawWidgetBounds(*pWnd, LocateOffset(&w, Point::Zero, pWnd),
			w.GetSize(), c);
}

YSL_END_NAMESPACE(Drawing)

YSL_END

