﻿/*
	Copyright (C) by Franksoft 2011 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ShlReader.h
\ingroup YReader
\brief Shell 阅读器框架。
\version r1660
\author FrankHB<frankhb1989@gmail.com>
\since build 263
\par 创建时间:
	2011-11-24 17:08:33 +0800
\par 修改时间:
	2012-12-04 22:31 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YReader::ShlReader
*/


#ifndef INC_YREADER_SHLREADER_H_
#define INC_YREADER_SHLREADER_H_ 1

#include "Shells.h"
#include "DSReader.h"
#include "HexBrowser.h"
#include "ReaderSetting.h"
#include "ReadingList.h"

YSL_BEGIN_NAMESPACE(YReader)

class ShlReader;


class ReaderBox : public Control
{
public:
	//! \since build 357
	typedef ystdex::subscriptive_iterator<ReaderBox, IWidget> Iterator;

	/*!
	\brief 弹出菜单按钮。
	\since build 274
	*/
	Button btnMenu;
	/*!
	\brief 设置按钮。
	\since build 330
	*/
	Button btnSetting;
	/*!
	\brief 显示信息框按钮。
	\since build 274
	*/
	Button btnInfo;
	/*!
	\brief 返回按钮。
	\since build 274
	*/
	Button btnReturn;
	/*!
	\brief 后退按钮。
	\since build 286
	*/
	Button btnPrev;
	/*!
	\brief 前进按钮。
	\since build 286
	*/
	Button btnNext;
	ProgressBar pbReader;
	Label lblProgress;

	ReaderBox(const Rect&);

	//! \since build 357
	DefWidgetMemberIteration(btnMenu, btnSetting, btnInfo, btnReturn, btnPrev,
		btnNext, pbReader, lblProgress)

	/*!
	\brief 更新进度数据。
	\since build 271
	*/
	void
	UpdateData(DualScreenReader&);
};


class TextInfoBox : public DialogBox
{
public:
	Label lblEncoding;
	Label lblSize;
	/*!
	\brief 指示迭代器位置。
	\since build 273
	*/
	//@{
	Label lblTop;
	Label lblBottom;
	//@}

	TextInfoBox();

	/*!
	\brief 刷新：按指定参数绘制界面并更新状态。
	\since build 294
	*/
	virtual void
	Refresh(PaintEventArgs&&);

	void
	UpdateData(DualScreenReader&);
};


class FileInfoPanel : public Panel
{
public:
	Label lblPath, lblSize, lblAccessTime, lblModifiedTime, lblOperations;
	
	FileInfoPanel();
};


class ShlReader : public ShlDS
{
protected:
	/*!
	\brief 当前路径。
	\since build 296
	*/
	IO::Path CurrentPath;
	/*!
	\brief 背景任务：用于滚屏。
	\since build 297
	*/
	std::function<void()> fBackgroundTask;
	/*!
	\brief 退出标识。
	\since build 302

	用于保证对一个实例只进行一次有效切换的标识。
	*/
	bool bExit;

public:
	/*!
	\brief 构造：使用指定路径。
	\since build 296
	*/
	ShlReader(const IO::Path&);

	/*!
	\brief 退出阅读器：停止后台任务并发送消息准备切换至 ShlExplorer 。
	\since build 295
	*/
	void
	Exit();

	/*!
	\brief 载入阅读器配置。
	\note 若失败则使用默认初始化。
	\since build 344
	*/
	static ReaderSetting
	LoadGlobalConfiguration();

	/*!
	\brief 处理输入消息：发送绘制消息，当处于滚屏状态时自动执行滚屏。
	\since build 289
	*/
	void
	OnInput() override;

	/*!
	\brief 保存阅读器配置。
	\since build 344
	*/
	static void
	SaveGlobalConfiguration(const ReaderSetting&);
};


/*!
\brief 文本阅读器 Shell 。
\since build 296
*/
class ShlTextReader : public ShlReader
{
public:
	/*!
	\brief 近期浏览记录。
	\since build 296
	*/
	ReadingList& LastRead;
	/*!
	\brief 当前阅读器设置。
	\since build 334
	*/
	ReaderSetting CurrentSetting;

protected:
	/*!
	\brief 滚屏计时器。
	\since build 289
	*/
	Timers::Timer tmrScroll;
	/*!
	\brief 按键计时器。
	\note 独立计时，排除路由事件干扰。
	\since build 300
	*/
	InputTimer tmrInput;
	//! \since build 323
	//@{
	DualScreenReader reader;
	ReaderBox boxReader;
	TextInfoBox boxTextInfo;
	//! \brief 设置面板。
	SettingPanel pnlSetting;
	unique_ptr<TextFile> pTextFile;
	MenuHost mhMain;
	//@}

public:
	/*!
	\brief 构造：使用指定路径。
	\since build 296
	*/
	ShlTextReader(const IO::Path&);
	/*!
	\brief 析构：释放资源。
	\since build 286
	*/
	~ShlTextReader() override;

private:
	/*!
	\brief 执行阅读器命令。
	\since build 274
	*/
	void
	Execute(IndexEventArgs::ValueType);

public:
	/*!
	\brief 读取文件。
	\note 不刷新按钮状态。
	\since build 286
	*/
	void
	LoadFile(const IO::Path&);

	/*!
	\brief 当自动滚屏有效状态为 true 时超时自动滚屏。
	\since build 289
	*/
	void
	Scroll();

private:
	void
	ShowMenu(Menu::ID, const Point&);

	/*!
	\brief 停止自动滚屏。
	\since build 297
	*/
	void
	StopAutoScroll();

	/*!
	\brief 切换编码。
	\note 若文本文件无效、参数为 Encoding() 或与当前编码相同则忽略。
	\since build 292
	*/
	void
	Switch(Text::Encoding);

	/*!
	\brief 更新近期浏览记录并更新按钮状态。
	\param 是否后退。
	\since build 286
	*/
	void
	UpdateReadingList(bool);

public:
	/*!
	\brief 更新按钮状态。
	\note 检查近期浏览记录状态确定可用性。
	\since build 286
	*/
	void
	UpdateButtons();

private:
	void
	OnClick(TouchEventArgs&&);

	void
	OnKeyDown(KeyEventArgs&&);
};


/*!
\brief 十六进制浏览器 Shell 。
\since build 296
*/
class ShlHexBrowser : public ShlReader
{
public:
	HexViewArea HexArea;
	FileInfoPanel pnlFileInfo;

	/*!
	\brief 构造：使用指定路径。
	\since build 296
	*/
	ShlHexBrowser(const IO::Path&);
};

YSL_END_NAMESPACE(YReader)

#endif

