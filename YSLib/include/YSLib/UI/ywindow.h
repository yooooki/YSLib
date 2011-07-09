﻿/*
	Copyright (C) by Franksoft 2009 - 2011.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ywindow.h
\ingroup UI
\brief 样式无关的图形用户界面窗口。
\version 0.4385;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2009-12-28 16:46:40 +0800;
\par 修改时间:
	2011-07-09 17:54 +0800;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::UI::YWindow;
*/


#ifndef YSL_INC_UI_YWINDOW_H_
#define YSL_INC_UI_YWINDOW_H_

#include "ypanel.h"
#include "../Service/yblit.h"

YSL_BEGIN

#undef YWindowAssert

YSL_BEGIN_NAMESPACE(Components)

#ifdef YCL_USE_YASSERT

#	define YWindowAssert(ptr, comp, func) \
	Components::yassert((ptr) && (ptr)->GetContext().IsValid(), \
	"The graphics context is invalid.", __LINE__, __FILE__, #comp, #func)

#else

#	define YWindowAssert(ptr, comp, func) \
	assert((ptr) && (ptr)->GetContext().IsValid())

#endif

YSL_BEGIN_NAMESPACE(Forms)

//! \brief 窗口接口。
DeclBasedInterface(IWindow, IPanel)
	DeclIEntry(bool RequiresUpdate() const)

	DeclIEntry(const Graphics& GetContext() const) //!< 取图形接口上下文。
	DeclIEntry(IWindow* GetWindowPtr() const)
	DeclIEntry(const Rect& GetInvalidatedArea() const)

	DeclIEntry(void CommitInvalidatedArea(const Rect&))

	DeclIEntry(void Update()) //!< 按需更新（以父窗口、屏幕优先顺序）。
EndDecl


/*!
\brief 是否需要刷新。
\note 若无效区域长宽都不为零，则需要刷新。
*/
bool
RequiresRefresh(const IWindow&);

/*!
\brief 设置窗口无效区域。
*/
void
SetInvalidationOf(IWindow&);


/*!
\brief 显示窗口。
\note 设置可见性和容器（若存在）背景重绘状态并设置自身刷新状态。
*/
void
Show(IWindow&);

/*!
\brief 隐藏窗口。
\note 设置不可见性和容器（若存在）背景重绘状态并取消自身刷新状态。
*/
void
Hide(IWindow&);


//! \brief 窗口模块。
class MWindow : protected Widgets::MWindowObject
{
protected:
	//基类中的 pWindow 为父窗口对象句柄，若为空则说明无父窗口。
	mutable shared_ptr<Drawing::Image> hBgImage; //!< 背景图像句柄。
	Rect rInvalidated; \
		//!< 无效区域：包含所有新绘制请求的区域（不一定是最小的）。
	bool bUpdate; //!< 更新属性：表示绘制结束，缓冲区准备完毕。

public:
	/*!
	\brief 构造：使用指定背景图像、窗口指针和 Shell 。
	*/
	explicit
	MWindow(const shared_ptr<Drawing::Image>& = share_raw(new Drawing::Image()),
		IWindow* = nullptr);

	PDefH0(bool, RequiresUpdate) const
		ImplRet(bUpdate);

	DefGetter(shared_ptr<Drawing::Image>&, BackgroundImagePtr, hBgImage)
};


//! \brief 抽象窗口。
class AWindow : public Controls::Control, protected MWindow,
	implements IWindow
{
public:
	/*!
	\brief 构造：使用指定边界、背景图像、窗口指针和 Shell 句柄。
	*/
	explicit
	AWindow(const Rect& = Rect::Empty,
		const shared_ptr<Drawing::Image>& = share_raw(new Drawing::Image()),
		IWindow* = nullptr);

	ImplI1(IWindow) PDefH0(bool, RequiresUpdate) const
		ImplBodyBase0(MWindow, RequiresUpdate)

	ImplI1(IWindow) DefGetterBase(IWindow*, WindowPtr, MWindow)
	ImplI1(IWindow) DefGetter(const Rect&, InvalidatedArea, rInvalidated)
	DefGetterBase(shared_ptr<Drawing::Image>&, BackgroundImagePtr, MWindow)
	/*!
	\brief 取位图背景指针。
	*/
	BitmapPtr
	GetBackgroundPtr() const;

	/*!
	\brief 设置大小。
	\note 虚 \c public 实现。
	*/
	virtual void
	SetSize(const Size&);
	DeclIEntry(void SetBufferSize(const Size&)) //!< 设置显示缓冲区大小。

	PDefH1(void, BeFilledWith, PixelType c) const
		ImplRet(Drawing::Fill(GetContext(), c)) \
		//!< 以纯色填充显示缓冲区。

	void
	CommitInvalidatedArea(const Rect&);

	PDefH0(void, ClearBackground) const //!< 清除背景。
		ImplRet(ClearImage(GetContext()))

protected:
	/*!
	\brief 绘制背景图像。
	*/
	bool
	DrawBackgroundImage();

	DeclIEntry(bool DrawContents())

public:
	/*!
	\brief 绘制界面（不检查刷新状态）。
	*/
	virtual void
	DrawRaw();

	/*!
	\brief 使相对于部件的指定区域在窗口缓冲区中无效。
	*/
	ImplI1(IWindow) void
	Invalidate(const Rect&);

	/*!
	\brief 刷新：绘制界面。
	*/
	ImplI1(IWindow) void
	Refresh();

	/*!
	\brief 按需更新。
	\note 以父窗口、屏幕优先顺序。
	*/
	ImplI1(IWindow) void
	Update();

	/*!
	\brief 更新至指定图形设备上下文的指定点。
	\note 以相对于容器的坐标作为相对于图形设备上下文的偏移。
	*/
	void
	UpdateTo(const Graphics&, const Point& = Point::Zero) const;

	/*!
	\brief 更新至桌面。
	*/
	virtual void
	UpdateToDesktop();

	/*!
	\brief 更新至上层窗口缓冲区。
	*/
	virtual void
	UpdateToWindow() const;
};


//! \brief 抽象框架窗口。
class AFrame : public AWindow, protected Widgets::MUIContainer
{
public:
	explicit
	AFrame(const Rect& = Rect::Empty,
		const shared_ptr<Drawing::Image>& = share_raw(new Drawing::Image()),
		IWindow* = nullptr);

	ImplI1(IWindow) void
	operator+=(IWidget&);
	ImplI1(IWindow) void
	operator+=(IControl&);
	ImplI1(IWindow) PDefHOperator1(void, +=, GMFocusResponser<IControl>& rsp)
		ImplBodyBase1(MUIContainer, operator+=, rsp)
	virtual void
	operator+=(IWindow&);
	template<class _type>
	inline void
	operator+=(_type& p)
	{
		return operator+=(static_cast<typename std::conditional<
			std::is_convertible<_type&, IWindow&>::value,
			IWindow&, typename std::conditional<std::is_convertible<_type&,
			IControl&>::value, IControl&, IWidget&>::type>::type>(p));
	}

	ImplI1(IWindow) bool
	operator-=(IWidget&);
	ImplI1(IWindow) bool
	operator-=(IControl&);
	ImplI1(IWindow) PDefHOperator1(bool, -=, GMFocusResponser<IControl>& rsp)
		ImplBodyBase1(MUIContainer, operator-=, rsp)
	virtual bool
	operator-=(IWindow&);
	template<class _type>
	inline bool
	operator-=(_type& p)
	{
		return operator-=(static_cast<typename std::conditional<
			std::is_convertible<_type&, IWindow&>::value,
			IWindow&, typename std::conditional<std::is_convertible<_type&,
			IControl&>::value, IControl&, IWidget&>::type>::type>(p));
	}

	using MUIContainer::Contains;

	ImplI1(IWindow) PDefH0(IControl*, GetFocusingPtr)
		ImplBodyBase0(GMFocusResponser<IControl>, GetFocusingPtr)
	ImplI1(IWindow) PDefH1(IWidget*, GetTopWidgetPtr, const Point& pt)
		ImplBodyBase1(MUIContainer, GetTopWidgetPtr, pt)
	ImplI1(IWindow) PDefH1(IControl*, GetTopControlPtr, const Point& pt)
		ImplBodyBase1(MUIContainer, GetTopControlPtr, pt)

	void
	Add(IControl&, Widgets::ZOrderType = Widgets::DefaultZOrder);

	ImplI1(IWindow) void
	ClearFocusingPtr();

	ImplI1(IWindow) PDefH1(bool, ResponseFocusRequest, AFocusRequester& req)
		ImplBodyBase1(MUIContainer, ResponseFocusRequest, req)

	ImplI1(IWindow) PDefH1(bool, ResponseFocusRelease, AFocusRequester& req)
		ImplBodyBase1(MUIContainer, ResponseFocusRelease, req)
};


//! \brief 标准框架窗口。
class Frame : public AFrame
{
protected:
	Drawing::BitmapBuffer Buffer; //!< 显示缓冲区。

public:
	/*!
	\brief 构造：使用指定边界、背景图像和窗口指针。
	*/
	explicit
	Frame(const Rect& = Rect::Empty,
		const shared_ptr<Drawing::Image>& = share_raw(new Drawing::Image()),
		IWindow* = nullptr);
	virtual
	~Frame();

	ImplI1(AWindow) DefGetter(const Graphics&, Context, Buffer)

	ImplI1(AWindow) PDefH1(void, SetBufferSize, const Size& s)
		ImplRet(Buffer.SetSize(s.Width, s.Height)) //!< 设置显示缓冲区大小。

protected:
	/*!
	\brief 绘制内容。
	\note 绘制部件。
	*/
	ImplI1(AWindow) bool
	DrawContents();
};

YSL_END_NAMESPACE(Forms)

YSL_END_NAMESPACE(Components)

YSL_END

#endif

