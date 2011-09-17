﻿/*
	Copyright (C) by Franksoft 2009 - 2011.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ywidget.h
\ingroup UI
\brief 样式无关的图形用户界面部件。
\version r6076;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2009-11-16 20:06:58 +0800;
\par 修改时间:
	2011-09-18 02:46 +0800;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::UI::Widget;
*/


#ifndef YSL_INC_UI_YWIDGET_H_
#define YSL_INC_UI_YWIDGET_H_

#include "ycomp.h"
#include "yrender.h"
#include "yfocus.hpp"
#include "ywgtevt.h"
#include "../Service/ydraw.h"
#include "../Service/ygdi.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Components)

//! \brief 部件接口。
DeclInterface(IWidget)
	DeclIEntry(bool IsVisible() const) //!< 判断是否可见。

	DeclIEntry(const Point& GetLocation() const)
	DeclIEntry(const Size& GetSize() const)
	/*!
	\brief 取容器指针的引用。
	\warning 注意修改容器指针时，应保持和容器包含部件的状态同步。
	*/
	DeclIEntry(IWidget*& GetContainerPtrRef() const)
	/*!
	\brief 取渲染器。
	*/
	DeclIEntry(Renderer& GetRenderer() const)
	/*!
	\brief 取焦点响应器。
	*/
	DeclIEntry(FocusResponder& GetFocusResponder() const)
	/*!
	\brief 取控制器。
	*/
	DeclIEntry(AController& GetController() const)

	/*!
	\brief 取包含指定点且被指定谓词过滤的顶端部件指针。
	\return 若为保存了子部件中的可见部件的容器则返回指针，否则返回 \c nullptr 。
	\note 使用部件坐标。
	*/
	DeclIEntry(IWidget* GetTopWidgetPtr(const Point&, bool(&)(const IWidget&)))

	DeclIEntry(void SetVisible(bool)) //!< 设置可见。
	DeclIEntry(void SetLocation(const Point&)) \
		//!< 设置左上角所在位置（相对于父容器的偏移坐标）。
	DeclIEntry(void SetSize(const Size&)) \
		//!< 设置大小。

	/*!
	\brief 刷新：在指定图形接口上下文以指定偏移起始按指定边界绘制界面。
	\return 实际被绘制的界面区域。
	\note 边界仅为暗示，允许实现忽略，但可以保证边界内的区域保持最新显示状态。
	\note 若部件的内部状态能够保证显示状态最新，则返回的区域可能比参数 r 更小。
	*/
	DeclIEntry(Rect Refresh(const PaintEventArgs& e))
EndDecl


/*!
\brief 判断点是否在部件的可视区域内。
*/
bool
Contains(const IWidget&, SPos, SPos);
/*!
\brief 判断点是否在部件的可视区域内。
*/
inline bool
Contains(const IWidget& wgt, const Point& pt)
{
	return Contains(wgt, pt.X, pt.Y);
}

/*!
\brief 判断点是否在可见部件的可视区域内。
*/
bool
ContainsVisible(const IWidget& wgt, SPos x, SPos y);
/*!
\brief 判断点是否在可见部件的可视区域内。
*/
inline bool
ContainsVisible(const IWidget& wgt, const Point& pt)
{
	return ContainsVisible(wgt, pt.X, pt.Y);
}

/*!
\ingroup HelperFunction
\brief 取部件的容器指针。
\note 使用此函数确保返回值传递的值语义。
*/
inline IWidget*
FetchContainerPtr(const IWidget& wgt)
{
	return wgt.GetContainerPtrRef();
}

/*
\ingroup HelperFunction
\brief 取指定部件的图形接口上下文。
*/
inline const Graphics&
FetchContext(const IWidget& wgt)
{
	return wgt.GetRenderer().GetContext();
}

/*!
\ingroup HelperFunction
\brief 取焦点对象指针。
\return 若为保存了子部件中的焦点对象的容器则返回指针，否则返回 \c nullptr 。
*/
inline IWidget*
FetchFocusingPtr(IWidget& wgt)
{
	return wgt.GetFocusResponder().GetFocusingPtr();
}

/*!
\brief 取部件边界。
*/
inline Rect
GetBoundsOf(const IWidget& wgt)
{
	return Rect(wgt.GetLocation(), wgt.GetSize());
}

/*!
\brief 设置部件边界。
*/
void
SetBoundsOf(IWidget&, const Rect&);

/*!
\brief 设置部件的无效区域。
*/
void
SetInvalidationOf(IWidget&);

/*!
\brief 在容器设置部件的无效区域。
\note 若容器不存在则忽略。
*/
void
SetInvalidationToParent(IWidget&);

/*!
\brief 清除焦点指针，同时以此部件作为事件源，调用被清除焦点部件的 LostFocus 事件。
\note 若此部件非容器则无效。
*/
void
ClearFocusingPtrOf(IWidget&);

/*!
\brief 无效化：使部件区域在窗口缓冲区中无效。
*/
void
Invalidate(IWidget&);

/*!
\brief 级联无效化：使相对于部件的指定区域在直接和简洁的窗口缓冲区中无效。
*/
void
InvalidateCascade(IWidget&, const Rect&);

/*
\brief 渲染：若缓冲存储不可用则刷新 wgt 。
\note 无条件更新实际渲染的区域至 e.ClipArea 。
*/
void
Render(IWidget& wgt, PaintEventArgs&& e);

/*
\brief 渲染子部件。
*/
void
RenderChild(IWidget&, PaintEventArgs&&);
/*
\brief 渲染子部件。
*/
inline void
RenderChild(IWidget& wgt, const PaintEventArgs& e)
{
	return RenderChild(wgt, PaintEventArgs(e));
}

/*!
\brief 请求提升至容器顶端。

\todo 完全实现提升 IWidget 至容器顶端（目前仅实现父容器为 AFrame 的情形）。
*/
void
RequestToTop(IWidget&);

/*!
\brief 更新部件至指定图形设备上下文的指定点。
*/
void
Update(const IWidget&, const PaintEventArgs&);

/*!
\brief 验证：若部件的缓冲区存在，绘制缓冲区使之有效。
\return 实际绘制的区域，意义同 IWidget::Refresh 。
*/
Rect
Validate(IWidget&);


/*!
\brief 显示部件。
\note 设置可见性和容器（若存在）背景重绘状态并设置自身刷新状态。
*/
void
Show(IWidget&);

/*!
\brief 隐藏部件。
\note 设置不可见性和容器（若存在）背景重绘状态并取消自身刷新状态。
*/
void
Hide(IWidget&);


//! \brief 方向模块。
class MOriented
{
protected:
	Drawing::Orientation Orientation;

	explicit
	MOriented(Drawing::Orientation);

	DefGetter(Drawing::Orientation, Orientation, Orientation)
};

inline
MOriented::MOriented(Drawing::Orientation o)
	: Orientation(o)
{}


/*!
\brief 部件控制器。
*/
class WidgetController : public AController
{
public:
	GEventWrapper<EventT(HPaintEvent)> Paint;

	explicit
	WidgetController(bool);

	ImplI1(AController) EventMapping::ItemType&
	GetItemRef(const VisualEvent&);

	ImplI1(AController) DefClone(WidgetController, Clone)
};


/*!
\brief 可视样式。
*/
class Visual
{
private:
	bool visible; //!< 可见性。
	bool transparent; //!< 透明性。
	Point location; //!< 左上角所在位置（相对于容器的偏移坐标）。
	Size size; //!< 部件大小。

public:
	Color BackColor; //!< 默认背景色。
	Color ForeColor; //!< 默认前景色。

	/*!
	\brief 构造：使用指定边界、前景色和背景色。
	*/
	explicit
	Visual(const Rect& = Rect::Empty,
		Color = Drawing::ColorSpace::White, Color = Drawing::ColorSpace::Black);
	DefDeCopyCtor(Visual)
	DefDeMoveCtor(Visual)
	virtual DefEmptyDtor(Visual)

	DefPredicate(Visible, visible)
	DefPredicate(Transparent, transparent) //!< 判断是否透明。

	DefGetter(SPos, X, GetLocation().X)
	DefGetter(SPos, Y, GetLocation().Y)
	DefGetter(SDst, Width, GetSize().Width)
	DefGetter(SDst, Height, GetSize().Height)
	DefGetter(const Point&, Location, location)
	DefGetter(const Size&, Size, size)

	DefSetter(bool, Visible, visible)
	DefSetter(bool, Transparent, transparent) //!< 设置透明性。
	/*!
	\brief 设置位置：横坐标。
	\note 非虚 \c public 实现。
	*/
	PDefH1(void, SetX, SPos x)
		ImplBodyBase1(Visual, SetLocation, Point(x, GetY()))
	/*!
	\brief 设置位置：纵坐标。
	\note 非虚 \c public 实现。
	*/
	PDefH1(void, SetY, SPos y)
		ImplBodyBase1(Visual, SetLocation, Point(GetX(), y))
	/*!
	\brief 设置大小：宽。
	\note 非虚 \c public 实现。
	*/
	PDefH1(void, SetWidth, SDst w)
		ImplBodyBase1(Visual, SetSize, Size(w, GetHeight()))
	/*!
	\brief 设置大小：高。
	\note 非虚 \c public 实现。
	*/
	PDefH1(void, SetHeight, SDst h)
		ImplBodyBase1(Visual, SetSize, Size(GetWidth(), h))
	/*!
	\brief 设置位置。
	\note 虚 \c public 实现。
	*/
	virtual DefSetter(const Point&, Location, location)
	/*!
	\brief 设置位置。
	\note 非虚 \c public 实现。
	*/
	PDefH2(void, SetLocation, SPos x, SPos y)
		ImplBodyBase1(Visual, SetLocation, Point(x, y))
	/*!
	\brief 设置大小。
	\note 虚 \c public 实现。
	*/
	virtual void
	SetSize(const Size& s);
	/*!
	\brief 设置大小。
	\note 非虚 \c public 实现。
	*/
	PDefH2(void, SetSize, SDst w, SDst h)
		ImplBodyBase1(Visual, SetSize, Size(w, h))
};


//! \brief 部件。
class Widget : public Visual,
	implements IWidget
{
private:
	mutable IWidget* pContainer; //!< 从属的部件容器的指针。
	unique_ptr<Renderer> pRenderer; //!< 渲染器指针。
	unique_ptr<FocusResponder> pFocusResponser; //!< 焦点响应器指针。

public:
	unique_ptr<AController> pController; //!< 控制器指针。

	explicit
	Widget(const Rect& = Rect::Empty,
		Color = Drawing::ColorSpace::White, Color = Drawing::ColorSpace::Black);
	PDefTH3(_tRenderer, _tFocusResponser, _tController)
	inline Widget(const Rect& r = Rect::Empty,
		_tRenderer&& pRenderer_ = unique_raw(new Renderer()),
		_tFocusResponser&& pFocusResponser_ = unique_raw(new FocusResponder()),
		_tController pController_ = nullptr)
		: Visual(r), pContainer(), pRenderer(yforward(pRenderer_)),
		pFocusResponser(yforward(pFocusResponser_)),
		pController(yforward(pController_))
	{}
	/*!
	\brief 复制构造：除容器指针为空外深复制。
	*/
	Widget(const Widget&);
	DefDeMoveCtor(Widget)
	/*!
	\brief 析构：虚实现。

	自动释放焦点后释放部件资源。
	\note 由于不完整类型 WidgetController 的依赖性无法使用 inline 实现。
	*/
	virtual
	~Widget();

	ImplI1(IWidget) DefPredicateBase(Visible, Visual)

	ImplI1(IWidget) DefGetterBase(const Point&, Location, Visual)
	ImplI1(IWidget) DefGetterBase(const Size&, Size, Visual)
	ImplI1(IWidget) DefGetter(IWidget*&, ContainerPtrRef, pContainer)
	ImplI1(IWidget) DefGetter(Renderer&, Renderer, *pRenderer)
	ImplI1(IWidget) DefGetter(FocusResponder&, FocusResponder, *pFocusResponser)
	ImplI1(IWidget) AController&
	GetController() const;
	ImplI1(IWidget) PDefH2(IWidget*, GetTopWidgetPtr, const Point&,
		bool(&)(const IWidget&))
		ImplRet(nullptr)

	ImplI1(IWidget) DefSetterBase(bool, Visible, Visual)
	ImplI1(IWidget) DefSetterBase(const Point&, Location, Visual)
	ImplI1(IWidget) DefSetterBase(const Size&, Size, Visual)
	/*!
	\brief 设置渲染器为指定指针指向的对象，同时更新渲染器状态。
	\note 若指针为空，则使用新建的 WidgetRenderer 对象。
	\note 取得指定对象的所有权。
	*/
	void
	SetRenderer(unique_ptr<Renderer>&&);
	/*!
	\brief 设置焦点响应器为指定指针指向的对象，同时清除焦点指针。
	\note 若指针为空，则使用新建的 FocusResponder 对象。
	\note 取得指定对象的所有权。
	*/
	void
	SetFocusResponser(unique_ptr<FocusResponder>&&);

	/*!
	\brief 刷新：在指定图形接口上下文以指定偏移起始按指定边界绘制界面。
	*/
	ImplI1(IWidget) Rect
	Refresh(const PaintEventArgs&);
};

inline AController&
Widget::GetController() const
{
	if(!pController)
		throw BadEvent();
	return *pController;
}

YSL_END_NAMESPACE(Components)

YSL_END

#endif

