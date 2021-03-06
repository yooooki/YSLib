﻿/*
	© 2009-2016 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file Initialization.h
\ingroup Helper
\brief 框架初始化。
\version r836
\author FrankHB <frankhb1989@gmail.com>
\since 早于 build 132
\par 创建时间:
	2009-10-21 23:15:08 +0800
\par 修改时间:
	2016-09-04 22:52 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	Helper::Initialization
*/


#ifndef INC_Helper_Initialization_h_
#define INC_Helper_Initialization_h_ 1

#include "YModules.h"
#include YFM_YSLib_Core_ValueNode // for ValueNode;
#include YFM_YSLib_Core_YApplication // for Application;
#include YFM_YSLib_Adaptor_Font // for Drawing::FontCache;
#include YFM_YSLib_Service_ContentType // for MIMEBiMapping;

namespace YSLib
{

#if YCL_DS
/*!
\brief 设置是否显示初始化的诊断消息。
\since build 690
*/
extern bool ShowInitializedLog;
#endif

/*!
\brief 初始化关键模块。
\throw FatalError 初始化失败。
\note 第二参数表示调用签名；后两个参数用于抛出异常。
\since build 725
*/
YF_API void
InitializeKeyModule(std::function<void()>, const char*, const char*,
	string_view);

/*!
\brief 处理最外层边界的异常，若捕获致命错误则在之后终止程序。
\note 可作为 FilterException 的参数，用于统一处理抛出到主函数的异常。
\sa FilterException
\since build 691
*/
YF_API void
TraceForOutermost(const std::exception&, RecordLevel) ynothrow;


/*!
\brief 载入 NPLA1 配置文件。
\param show_info 是否在标准输出中显示信息。
\pre 间接断言：指针参数非空。
\return 读取的配置。
\note 预设行为、配置文件和配置项参考 Documentation::YSLib 。
\since build 450
*/
YF_API YB_NONNULL(1, 2) ValueNode
LoadNPLA1File(const char* disp, const char* path, ValueNode(*creator)(),
	bool show_info = {});

/*!
\brief 初始化应用程序组件。
\throw GeneralEvent 嵌套异常：加载失败。
\since build 693
*/
YF_API void
LoadComponents(Application&, const ValueNode&);

/*!
\brief 载入默认配置。
\return 读取的配置。
\sa LoadNPLA1File
\since build 344
*/
YF_API ValueNode
LoadConfiguration(bool = {});

/*!
\brief 保存默认配置。
\throw GeneralEvent 配置文件路径指定的文件不存在或保存失败。\
\since build 344
*/
YF_API void
SaveConfiguration(const ValueNode&);


/*!
\brief 初始化系统字体缓存。
\pre 默认字体缓存已初始化。
\throw FatalError 字体缓存初始化失败。
\since build 398

加载默认字体文件路径和默认字体目录中的字型至默认字体缓存。
*/
YF_API void
InitializeSystemFontCache(Drawing::FontCache&, const string&, const string&);


/*!
\brief 取值类型根节点。
\pre 断言：已初始化。
\since build 688
*/
YF_API ValueNode&
FetchRoot() ynothrow;

/*!
\brief 取默认字体缓存。
\exception FatalError 字体缓存初始化失败。
\since build 425
*/
YF_API Drawing::FontCache&
FetchDefaultFontCache();

/*!
\brief 取 MIME 类型名和文件扩展名双向映射对象。
\since build 450
*/
YF_API MIMEBiMapping&
FetchMIMEBiMapping();

} // namespace YSLib;

#endif

