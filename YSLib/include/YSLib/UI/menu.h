﻿/*
	Copyright (C) by Franksoft 2011.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file menu.h
\ingroup UI
\brief 样式相关的菜单。
\version 0.1653;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2011-06-02 12:17:38 +0800;
\par 修改时间:
	2011-06-10 17:31 +0800;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::UI::Menu;
*/


#ifndef YSL_INC_SHELL_MENU_H_
#define YSL_INC_SHELL_MENU_H_

#include "textlist.h"
#include "yuicont.h" //for Widgets::ZOrderType;

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Components)

YSL_BEGIN_NAMESPACE(Controls)

class MenuHost;

const Widgets::ZOrderType DefaultMenuZOrder(224); //!< 默认菜单 Z 顺序值。


//! \brief 文本菜单。
class Menu : public TextList
{
	friend class MenuHost;

public:
	typedef size_t ID; //!< 菜单标识类型。
	typedef map<IndexType, Menu*> SubMap; //!< 子菜单映射表类型。
	typedef SubMap::value_type ValueType; //!< 子菜单映射表项目类型。

	ID id; //!< 菜单标识。

protected:
	MenuHost* pHost; //!< 宿主指针。
	Menu* pParent; //!< 父菜单指针。
	SubMap mSubMenus; //!< 子菜单映射表：存储非空子菜单指针。

public:
	/*!
	\brief 构造：使用指定边界、文本列表和菜单标识。
	*/
	explicit
	Menu(const Rect& = Rect::Empty,
		const shared_ptr<ListType>& = shared_ptr<ListType>(), ID = 0);
	DefGetter(ID, ID, id)

	/*!
	\brief 访问索引指定的子菜单。
	\exception std::out_of_range 异常中立：指定子菜单越界或不存在：由 at 抛出。
	*/
	PDefHOperator1(Menu&, [], size_t idx)
		ImplRet(*mSubMenus.at(idx))

	/*!
	\brief 向子菜单组添加关联索引和指针指定的菜单。
	\note 若索引越界或菜单指针为空则忽略。
	*/
	void
	operator+=(const ValueType&);

	/*!
	\brief 向菜单组移除指定子菜单索引项。
	\note 若索引越界或不存在则忽略。
	*/
	bool
	operator-=(IndexType);

	DefGetter(Menu*, ParentPtr, pParent)

	/*!
	\brief 按指定 Z 顺序显示菜单。
	\note 菜单宿主指针为空时忽略。
	*/
	bool
	Show(Widgets::ZOrderType = DefaultMenuZOrder);

	/*!
	\brief 隐藏菜单。
	\note 菜单宿主指针为空时忽略。
	*/
	bool
	Hide();

protected:
	/*!
	\brief 绘制菜单项。
	*/
	virtual void
	PaintItem(const Graphics&, const Rect&, ListType::size_type);
};


//! \brief 菜单宿主。
class MenuHost : noncopyable
{
public:
	typedef Menu* ItemType; //!< 菜单组项目类型：记录菜单控件指针。
	typedef map<Menu::ID, ItemType> MenuMap; //!< 菜单组类型。
	typedef MenuMap::value_type ValueType;

	Forms::AFrame& Frame; //!< 框架窗口。
	Menu* SubMenuPointer; //!< 锁定子菜单指针：指定当前处理的子菜单。

protected:
	MenuMap mMenus; //!< 菜单组：存储非空菜单指针。

public:
	MenuHost(Forms::AFrame&);
	virtual
	~MenuHost();

	/*!
	\brief 向菜单组添加标识和指针指定的菜单。
	\note 覆盖菜单对象的菜单标识成员；若菜单项已存在则覆盖旧菜单项。
	*/
	void
	operator+=(const ValueType&);
	/*!
	\brief 向菜单组添加菜单。
	\note 标识由菜单对象的菜单标识成员指定；若菜单项已存在则覆盖旧菜单项。
	*/
	void
	operator+=(Menu&);

	/*!
	\brief 从菜单组移除菜单标识指定的菜单。
	\note 同时置菜单宿主指针为空。
	*/
	bool
	operator-=(Menu::ID);

	/*!
	\brief 访问菜单标识指定的菜单。
	\exception std::out_of_range 异常中立：由 at 抛出。
	*/
	PDefHOperator1(Menu&, [], Menu::ID id)
		ImplRet(*mMenus.at(id))

	/*!
	\brief 判断框架窗口中是否正在显示菜单标识指定的菜单。
	*/
	bool
	IsShowing(Menu::ID);

	/*!
	\brief 判断菜单组中是否存在菜单标识指定的菜单。
	*/
	PDefH1(bool, Contains, Menu::ID id)
		ImplRet(mMenus.find(id) != mMenus.end())
	/*!
	\brief 判断菜单组中是否存在指定的菜单。
	*/
	bool
	Contains(Menu&);

	/*
	\brief 清除菜单组。
	\note 同时置菜单的菜单宿主指针为空。
	*/
	void
	Clear();

	/*
	\brief 按指定 Z 顺序显示菜单组中菜单标识指定的菜单。
	*/
	void
	Show(Menu::ID, Widgets::ZOrderType = DefaultMenuZOrder);
	/*!
	\brief 按指定 Z 顺序显示指定菜单 mnu。
	\pre 断言： Contains(mnu) 。
	*/
	void
	Show(Menu& mnu, Widgets::ZOrderType = DefaultMenuZOrder);

	/*
	\brief 按指定 Z 顺序显示菜单组中的所有菜单。
	*/
	void
	ShowAll(Widgets::ZOrderType = DefaultMenuZOrder);

private:
	/*
	\brief 按指定 Z 顺序显示指定菜单 mnu。
	*/
	void
	ShowRaw(Menu& mnu, Widgets::ZOrderType = DefaultMenuZOrder);

public:
	/*
	\brief 隐藏菜单组中菜单标识指定的菜单。
	*/
	void
	Hide(Menu::ID);
	/*!
	\brief 隐藏指定菜单 mnu。
	\pre 断言： Contains(mnu) 。
	*/
	void
	Hide(Menu& mnu);

	/*
	\brief 隐藏菜单组中的所有菜单。
	*/
	void
	HideAll();

private:
	/*!
	\brief 隐藏指定菜单 mnu。
	*/
	void
	HideRaw(Menu& mnu);
};

inline void
MenuHost::Show(Menu& mnu, Widgets::ZOrderType z)
{
	YAssert(Contains(mnu), "Menu is not contained @ MenuHost::Show;");

	ShowRaw(mnu, z);
}

inline void
MenuHost::Hide(Menu& mnu)
{
	YAssert(Contains(mnu), "Menu is not contained @ MenuHost::HideRaw;");

	HideRaw(mnu);
}

YSL_END_NAMESPACE(Controls)

YSL_END_NAMESPACE(Components)

YSL_END

#endif
