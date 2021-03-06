﻿/*
	© 2014-2017 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file NPLA1.h
\ingroup NPL
\brief NPLA1 公共接口。
\version r3600
\author FrankHB <frankhb1989@gmail.com>
\since build 472
\par 创建时间:
	2014-02-02 17:58:24 +0800
\par 修改时间:
	2017-10-22 15:48 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	NPL::NPLA1
*/


#ifndef NPL_INC_NPLA1_h_
#define NPL_INC_NPLA1_h_ 1

#include "YModules.h"
#include YFM_NPL_NPLA // for NPLATag, ValueNode, TermNode, LoggedEvent,
//	NPL::AccessTerm, ystdex::equality_comparable, ystdex::exclude_self_t,
//	YSLib::AreEqualHeld, YSLib::GHEvent, ystdex::make_function_type_t,
//	ystdex::make_parameter_tuple_t, ystdex::make_expanded,
//	ystdex::invoke_nonvoid, ystdex::make_transform, std::accumulate,
//	ystdex::bind1, std::placeholders::_2, ystdex::examiners::equal_examiner;

namespace NPL
{

//! \since build 665
namespace A1
{

/*!
\brief NPLA1 元标签。
\note NPLA1 是 NPLA 的具体实现。
\since build 597
*/
struct YF_API NPLA1Tag : NPLATag
{};


/*!
\ingroup ThunkType
\brief 值记号：节点中的值的占位符。
*/
enum class ValueToken
{
	Null,
	/*!
	\brief 未定义值。
	\since build 732
	*/
	Undefined,
	/*!
	\brief 未指定值。
	\since build 732
	*/
	Unspecified,
	GroupingAnchor,
	OrderedAnchor
};

/*!
\brief 取值记号的字符串表示。
\return 表示对应记号值的字符串，保证不相等的输入对应不相等的结果。
\throw std::invalid_argument 输入的值不是合法的值记号。
\relates ValueToken
\since build 768
*/
YF_API string
to_string(ValueToken);


//! \since build 674
//@{
//! \brief 插入 NPLA1 子节点。
//@{
/*!
\note 插入后按名称排序顺序。

第一参数指定的变换结果插入第二参数指定的容器。
若映射操作返回节点名称为空则根据当前容器内子节点数量加前缀 $ 命名以避免重复。
*/
YF_API void
InsertChild(TermNode&&, TermNode::Container&);

/*!
\note 保持顺序。

直接插入 NPLA1 子节点到序列容器末尾。
*/
YF_API void
InsertSequenceChild(TermNode&&, NodeSequence&);
//@}

/*!
\brief 变换 NPLA1 节点 S 表达式抽象语法树为 NPLA1 语义结构。
\exception std::bad_function_call 第三至五参数为空。
\return 变换后的新节点（及子节点）。

第一参数指定源节点，其余参数指定部分变换规则。
当被调用的第二至第四参数不修改传入的节点参数时变换不修改源节点。
过程如下：
若源节点为叶节点，直接返回使用第三参数创建映射的节点。
若源节点只有一个子节点，直接返回这个子节点的变换结果。
否则，使用第四参数从第一个子节点取作为变换结果名称的字符串。
	若名称非空则忽略第一个子节点，只变换剩余子节点。
		当剩余一个子节点（即源节点有两个子节点）时，直接递归变换这个节点并返回。
		若变换后的结果名称非空，则作为结果的值；否则，结果作为容器内单一的值。
	否则，新建节点容器，遍历并变换剩余的节点插入其中，返回以之构造的节点。
		第二参数指定此时的映射操作，若为空则默认使用递归 TransformNode 调用。
		调用第五参数插入映射的结果到容器。
*/
YF_API ValueNode
TransformNode(const TermNode&, NodeMapper = {}, NodeMapper = MapNPLALeafNode,
	NodeToString = ParseNPLANodeString, NodeInserter = InsertChild);

/*!
\brief 变换 NPLA1 节点 S 表达式抽象语法树为 NPLA1 序列语义结构。
\exception std::bad_function_call 第三至五参数为空。
\return 变换后的新节点（及子节点）。
\sa TransformNode

和 TransformNode 变换规则相同，
但插入的子节点以 NodeSequence 的形式作为变换节点的值而不是子节点，可保持顺序。
*/
YF_API ValueNode
TransformNodeSequence(const TermNode&, NodeMapper = {},
	NodeMapper = MapNPLALeafNode, NodeToString = ParseNPLANodeString,
	NodeSequenceInserter = InsertSequenceChild);
//@}


/*!
\brief 加载 NPLA1 翻译单元。
\throw LoggedEvent 警告：被加载配置中的实体转换失败。
*/
//@{
template<typename _type, typename... _tParams>
ValueNode
LoadNode(_type&& tree, _tParams&&... args)
{
	TryRet(A1::TransformNode(std::forward<TermNode&&>(tree),
		yforward(args)...))
	CatchThrow(ystdex::bad_any_cast& e, LoggedEvent(YSLib::sfmt(
		"Bad NPLA1 tree found: cast failed from [%s] to [%s] .", e.from(),
		e.to()), YSLib::Warning))
}

template<typename _type, typename... _tParams>
ValueNode
LoadNodeSequence(_type&& tree, _tParams&&... args)
{
	TryRet(A1::TransformNodeSequence(std::forward<TermNode&&>(tree),
		yforward(args)...))
	CatchThrow(ystdex::bad_any_cast& e, LoggedEvent(YSLib::sfmt(
		"Bad NPLA1 tree found: cast failed from [%s] to [%s] .", e.from(),
		e.to()), YSLib::Warning))
}
//@}


//! \pre 间接断言：参数指定的上下文中的尾调用动作为空。
//@{
/*!
\brief NPLA1 表达式节点规约：调用至少一次求值例程规约子表达式。
\return 规约状态。
\sa ContextNode::RewriteGuarded
\sa ReduceOnce
\since build 730

以第一参数为项，以 ReduceOnce 为规约函数调用 ContextNode::RewriteGuarded 。
*/
YF_API ReductionStatus
Reduce(TermNode&, ContextNode&);

/*!
\brief 再次规约。
\sa ContextNode::SetupTail
\sa ReduceOnce
\return ReductionStatus::Retrying
\since build 807

确保再次 Reduce 调用并返回要求重规约的结果。
*/
YF_API ReductionStatus
ReduceAgain(TermNode&, ContextNode&);
//@}

/*!
\note 可能使参数中容器的迭代器失效。
\sa Reduce
*/
//@{
/*!
\note 按语言规范，子项规约顺序未指定。
\note 忽略子项重规约要求。
*/
//@{
/*!
\brief 对范围内的第二项开始逐项规约。
\throw InvalidSyntax 容器为空。
\sa ReduceChildren
\since build 773
*/
//@{
YF_API void
ReduceArguments(TNIter, TNIter, ContextNode&);
//! \since build 685
inline PDefH(void, ReduceArguments, TermNode::Container& con, ContextNode& ctx)
	ImplRet(ReduceArguments(con.begin(), con.end(), ctx))
inline PDefH(void, ReduceArguments, TermNode& term, ContextNode& ctx)
	ImplRet(ReduceArguments(term.GetContainerRef(), ctx))
//@}

//! \note 失败视为重规约。
//@{
/*!
\brief 规约并检查成功：调用 Reduce 并检查结果。
\sa CheckedReduceWith
\sa Reduce
\since build 735
*/
YF_API void
ReduceChecked(TermNode&, ContextNode&);

/*!
\brief 规约闭包。
\return 根据规约后剩余项确定的规约结果。
\sa CheckNorm
\sa ReduceChecked
\since build 802

构造规约项，规约后替换到第一参数指定项。
规约项的内容由第四参数的闭包指定。第三参数指定是否通过转移构造而不保留原项。
规约后转移闭包规约的结果：子项以及引用的值的目标被转移到第一参数指定的项。
结果中子项和值之间被转移的相对顺序未指定。
规约闭包可作为 β 规约或动态环境中求值的尾调用。
*/
YF_API ReductionStatus
ReduceCheckedClosure(TermNode&, ContextNode&, bool, TermNode&);
//@}

/*!
\brief 规约子项。
\since build 697
*/
//@{
YF_API void
ReduceChildren(TNIter, TNIter, ContextNode&);
inline PDefH(void, ReduceChildren, TermNode::Container& con, ContextNode& ctx)
	ImplExpr(ReduceChildren(con.begin(), con.end(), ctx))
inline PDefH(void, ReduceChildren, TermNode& term, ContextNode& ctx)
	ImplExpr(ReduceChildren(term.GetContainerRef(), ctx))
//@}
//@}

/*!
\brief 有序规约子项。
\return 当存在子项时为最后一个子项的规约状态，否则为 ReductionStatus::Clean 。
\since build 773
*/
//@{
YF_API ReductionStatus
ReduceChildrenOrdered(TNIter, TNIter, ContextNode&);
inline PDefH(ReductionStatus, ReduceChildrenOrdered, TermNode::Container& con,
	ContextNode& ctx)
	ImplRet(ReduceChildrenOrdered(con.begin(), con.end(), ctx))
inline PDefH(ReductionStatus, ReduceChildrenOrdered, TermNode& term,
	ContextNode& ctx)
	ImplRet(ReduceChildrenOrdered(term.GetContainerRef(), ctx))
//@}

/*!
\brief 规约第一个子项。
\return 规约状态。
\sa ReduceNested
\see https://en.wikipedia.org/wiki/Fexpr 。
\since build 730

快速严格性分析：
无条件求值枝节点第一项以避免非确定性推断子表达式求值的附加复杂度。
调用 ReduceNested 规约子项。
*/
YF_API ReductionStatus
ReduceFirst(TermNode&, ContextNode&);

/*!
\brief 规约可能嵌套规约的项。
\sa Reduce
\since build 807

保存上下文的尾调用动作并调用 Reduce 。
若调用返回且不需要重规约则重置尾调用动作，并放弃规约时可能设置的动作尾调用动作；
否则，放弃保存的尾调用动作，保留规约时可能设置的尾调用动作。
*/
YF_API ReductionStatus
ReduceNested(TermNode&, ContextNode&);

/*!
\brief NPLA1 表达式节点规约：调用求值例程规约子表达式。
\pre 间接断言：参数指定的上下文中的尾调用动作为空。
\return 规约状态。
\note 异常安全为调用遍的最低异常安全保证。
\note 可能使参数中容器的迭代器失效。
\note 默认不需要重规约。这可被求值遍改变。
\note 可被求值遍调用以实现递归求值。
\sa ContextNode::EvaluateLeaf
\sa ContextNode::EvaluateList
\sa ValueToken
\sa ReduceAgain
\since build 806
\todo 实现 ValueToken 保留处理。

对应不同的节点次级结构分类，一次迭代按以下顺序选择以下分支之一，按需规约子项：
对枝节点调用 ContextNode::EvaluateList 求值；
对空节点或值为 ValueToken 的叶节点不进行操作；
对其它叶节点调用 ContextNode::EvaluateList 求值。
迭代结束调用 CheckReducible ，根据结果判断是否进行重规约。
此处约定的迭代中对节点的具体结构分类默认也适用于其它 NPLA1 实现 API ；
例外情况应单独指定明确的顺序。
例外情况包括输入节点不是表达式语义结构（而是 AST ）的 API ，如 TransformNode 。
当前实现返回的规约状态总是 ReductionStatus::Clean ，否则会循环迭代。
若需要保证无异常时仅在规约成功后终止，使用 ReduceChecked 代替。
*/
YF_API ReductionStatus
ReduceOnce(TermNode&, ContextNode&);

/*!
\brief 规约有序序列：顺序规约子项，结果为最后一个子项的规约结果。
\return 当存在子项时为最后一个子项的规约状态，否则为 ReductionStatus::Clean 。
\sa ReduceChildrenOrdered
\since build 764

可直接实现顺序求值。
对空参数返回未指定值。
参考调用文法：
$sequence <object>...
*/
YF_API ReductionStatus
ReduceOrdered(TermNode&, ContextNode&);

/*!
\brief 移除容器第一个子项到指定迭代器的项后规约。
\note 按语言规范，子项规约顺序未指定。
\return ReductionStatus::Retrying
\sa ReduceAgain
\since build 733
*/
YF_API ReductionStatus
ReduceTail(TermNode&, ContextNode&, TNIter);
//@}


/*!
\brief 设置跟踪深度节点：调用规约时显示深度和上下文等信息。
\note 主要用于调试。
\sa ContextNode::Guard
\since build 685
*/
YF_API void
SetupTraceDepth(ContextNode& ctx, const string& name = yimpl("$__depth"));


//! \note ValueObject 参数分别指定替换添加的前缀和被替换的分隔符的值。
//@{
/*!
\brief 变换分隔符中缀表达式为前缀表达式。
\sa AsIndexNode
\since build 753

移除子项中值和指定分隔符指定的项，并以 AsIndexNode 添加指定前缀值作为子项。
最后一个参数指定返回值的名称。
*/
//@{
//! \note 非递归变换。
YF_API TermNode
TransformForSeparator(const TermNode&, const ValueObject&, const ValueObject&,
	const TokenValue& = {});

//! \note 递归变换。
YF_API TermNode
TransformForSeparatorRecursive(const TermNode&, const ValueObject&,
	const ValueObject&, const TokenValue& = {});
//@}

/*!
\brief 查找项中的指定分隔符，若找到则替换项为去除分隔符并添加替换前缀的形式。
\return 是否找到并替换了项。
\sa EvaluationPasses
\sa TransformForSeparator
\since build 730
*/
YF_API ReductionStatus
ReplaceSeparatedChildren(TermNode&, const ValueObject&, const ValueObject&);
//@}


//! \since build 751
//@{
/*!
\brief 包装上下文处理器。
\note 忽略被包装的上下文处理器可能存在的返回值，自适应默认返回规约结果。
\warning 非虚析构。
*/
template<typename _func>
struct WrappedContextHandler
	: private ystdex::equality_comparable<WrappedContextHandler<_func>>
{
	_func Handler;

	//! \since build 757
	//@{
	template<typename _tParam, yimpl(typename
		= ystdex::exclude_self_t<WrappedContextHandler, _tParam>)>
	WrappedContextHandler(_tParam&& arg)
		: Handler(yforward(arg))
	{}
	template<typename _tParam1, typename _tParam2, typename... _tParams>
	WrappedContextHandler(_tParam1&& arg1, _tParam2&& arg2, _tParams&&... args)
		: Handler(yforward(arg1), yforward(arg2), yforward(args)...)
	{}

	DefDeCopyMoveCtorAssignment(WrappedContextHandler)
	//@}

	template<typename... _tParams>
	ReductionStatus
	operator()(_tParams&&... args) const
	{
		Handler(yforward(args)...);
		return ReductionStatus::Clean;
	}

	/*!
	\brief 比较上下文处理器相等。
	\note 使用 YSLib::AreEqualHeld 。
	\since build 756
	*/
	friend PDefHOp(bool, ==, const WrappedContextHandler& x,
		const WrappedContextHandler& y)
		ImplRet(YSLib::AreEqualHeld(x.Handler, y.Handler))
};

template<class _tDst, typename _func>
inline _tDst
WrapContextHandler(_func&& h, ystdex::false_)
{
	return WrappedContextHandler<YSLib::GHEvent<ystdex::make_function_type_t<
		void, ystdex::make_parameter_tuple_t<typename _tDst::BaseType>>>>(
		yforward(h));
}
template<class, typename _func>
inline _func
WrapContextHandler(_func&& h, ystdex::true_)
{
	return yforward(h);
}
template<class _tDst, typename _func>
inline _tDst
WrapContextHandler(_func&& h)
{
	using BaseType = typename _tDst::BaseType;

	// XXX: It is a hack to adjust the convertible result for the expanded
	//	caller here. It should have been implemented in %GHEvent, however types
	//	those cannot convert to expanded caller cannot be SFINAE'd out,
	//	otherwise it would cause G++ 5.4 crash with internal compiler error:
	//	"error reporting routines re-entered".
	return A1::WrapContextHandler<_tDst>(yforward(h), ystdex::or_<
		std::is_constructible<BaseType, _func&&>,
		std::is_constructible<BaseType, ystdex::expanded_caller<
		typename _tDst::FuncType, ystdex::decay_t<_func>>>>());
}
//@}


//! \warning 非虚析构。
//@{
/*!
\brief 形式上下文处理器。
\since build 674

处理列表项的操作符。
*/
class YF_API FormContextHandler
	: private ystdex::equality_comparable<FormContextHandler>
{
public:
	ContextHandler Handler;
	/*!
	\brief 项检查例程：验证被包装的处理器的调用符合前置条件。
	\since build 733
	*/
	std::function<bool(const TermNode&)> Check{IsBranch};

	//! \since build 697
	template<typename _func,
		yimpl(typename = ystdex::exclude_self_t<FormContextHandler, _func>)>
	FormContextHandler(_func&& f)
		: Handler(A1::WrapContextHandler<ContextHandler>(yforward(f)))
	{}
	//! \since build 733
	template<typename _func, typename _fCheck>
	FormContextHandler(_func&& f, _fCheck c)
		: Handler(A1::WrapContextHandler<ContextHandler>(yforward(f))), Check(c)
	{}
	//! \since build 757
	DefDeCopyMoveCtorAssignment(FormContextHandler)

	/*!
	\brief 比较上下文处理器相等。
	\note 忽略检查例程的等价性。
	\since build 748
	*/
	friend PDefHOp(bool, ==, const FormContextHandler& x,
		const FormContextHandler& y)
		ImplRet(x.Handler == y.Handler)

	/*!
	\brief 处理一般形式。
	\return Handler 调用的返回值，或 ReductionStatus::Clean 。
	\exception NPLException 异常中立。
	\throw LoggedEvent 警告：类型不匹配，
		由 Handler 抛出的 ystdex::bad_any_cast 转换。
	\throw LoggedEvent 错误：由 Handler 抛出的 ystdex::bad_any_cast 外的
		std::exception 转换。
	\throw std::invalid_argument 项检查未通过。
	\since build 751

	项检查不存在或在检查通过后，变换无参数规约，然后对节点调用 Hanlder ，否则抛出异常。
	无参数时第一参数具有两个子项且第二项为空节点。无参数变换删除空节点。
	*/
	ReductionStatus
	operator()(TermNode&, ContextNode&) const;
};


/*!
\brief 严格上下文处理器。
\since build 754

对参数先进行求值的处理列表项的操作符。
*/
class YF_API StrictContextHandler
	: private ystdex::equality_comparable<StrictContextHandler>
{
public:
	//! \since build 696
	FormContextHandler Handler;

	//! \since build 697
	template<typename _func,
		yimpl(typename = ystdex::exclude_self_t<StrictContextHandler, _func>)>
	StrictContextHandler(_func&& f)
		: Handler(yforward(f))
	{}
	//! \since build 733
	template<typename _func, typename _fCheck>
	StrictContextHandler(_func&& f, _fCheck c)
		: Handler(yforward(f), c)
	{}
	//! \since build 757
	DefDeCopyMoveCtorAssignment(StrictContextHandler)

	//! \brief 比较上下文处理器相等。
	friend PDefHOp(bool, ==, const StrictContextHandler& x,
		const StrictContextHandler& y)
		ImplRet(x.Handler == y.Handler)

	/*!
	\brief 处理函数。
	\return Handler 调用的返回值。
	\throw ListReductionFailure 列表子项不大于一项。
	\sa ReduceArguments
	\since build 751

	对每一个子项求值；然后检查项数，对可调用的项调用 Hanlder ，否则抛出异常。
	*/
	ReductionStatus
	operator()(TermNode&, ContextNode&) const;
};
//@}


//! \since build 733
//@{
template<typename... _tParams>
inline void
RegisterForm(ContextNode& ctx, const string& name, _tParams&&... args)
{
	NPL::RegisterContextHandler(ctx, name,
		FormContextHandler(yforward(args)...));
}

//! \brief 转换上下文处理器。
template<typename... _tParams>
inline ContextHandler
ToContextHandler(_tParams&&... args)
{
	return StrictContextHandler(yforward(args)...);
}

/*!
\brief 注册严格上下文处理器。
\note 使用 ADL ToContextHandler 。
\since build 754
*/
template<typename... _tParams>
inline void
RegisterStrict(ContextNode& ctx, const string& name, _tParams&&... args)
{
	NPL::RegisterContextHandler(ctx, name, ToContextHandler(yforward(args)...));
}
//@}

/*!
\brief 注册分隔符转换变换和处理例程。
\sa NPL::RegisterContextHandler
\sa ReduceChildren
\sa ReduceOrdered
\sa ReplaceSeparatedChildren
\since build 753

变换带有中缀形式的分隔符记号的表达式为指定名称的前缀表达式并去除分隔符，
然后注册前缀语法形式。
最后一个参数指定是否有序，选择语法形式为 ReduceOrdered 或 ReduceChildren 之一。
前缀名称不需要是记号支持的标识符。
*/
YF_API void
RegisterSequenceContextTransformer(EvaluationPasses&, ContextNode&,
	const TokenValue&, const ValueObject&, bool = {});


/*!
\brief 断言枝节点。
\pre 断言：参数指定的项是枝节点。
\since build 761
*/
inline PDefH(void, AssertBranch, const TermNode& term,
	const char* msg = "Invalid term found.") ynothrowv
	ImplExpr(yunused(msg), YAssert(IsBranch(term), msg))

/*!
\brief 取项的参数个数：子项数减 1 。
\pre 间接断言：参数指定的项是枝节点。
\return 项的参数个数。
\since build 733
*/
inline PDefH(size_t, FetchArgumentN, const TermNode& term) ynothrowv
	ImplRet(AssertBranch(term), term.size() - 1)


//! \note 第一参数指定输入的项，其 Value 指定输出的值。
//@{
//! \sa LiftDelayed
//@{
/*!
\brief 求值以节点数据结构间接表示的项。
\since build 752

以 TermNode 按项访问值，若成功调用 LiftDelayed 替换值并返回要求重规约。
以项访问对规约以项转移的可能未求值的操作数是必要的。
*/
YF_API ReductionStatus
EvaluateDelayed(TermNode&);
/*!
\brief 求值指定的延迟求值项。
\return ReductionStatus::Retrying 。
\since build 761

提升指定的延迟求值项并规约。
*/
YF_API ReductionStatus
EvaluateDelayed(TermNode&, DelayedTerm&);
//@}

/*!
\exception BadIdentifier 标识符未声明。
\note 默认结果为 ReductionStatus::Clean 以保证强规范化性质。
*/
//@{
/*!
\brief 求值标识符。
\pre 间接断言：第三参数的数据指针非空。
\note 不验证标识符是否为字面量；仅以字面量处理时可能需要重规约。
\sa EvaluateDelayed
\sa LiteralHandler
\sa ReferenceTerm
\sa ResolveName
\sa TermReference
\since build 745

依次进行以下求值操作：
调用 ResolveName 根据指定名称查找值，若失败抛出未声明异常；
初始化值，进行引用折叠后重新引用为左值；
以 LiteralHandler 访问字面量处理器，若成功调用并返回字面量处理器的处理结果。
若未返回，根据节点表示的值进一步处理：
	对表示非 TokenValue 值的叶节点，调用 EvaluateDelayed 求值；
	对表示 TokenValue 值的叶节点，返回 ReductionStatus::Clean ；
	对枝节点视为列表，返回 ReductionStatus::Retained 。
初始化值的目标是第一参数的 Value 数据成员，包括以下操作：
	调用 ReferenceTerm 确保进一步操作解析得到的（保存在环境中的）项对象不是引用；
	若上一步得到的值是空列表或 TokenValue ，直接复制；
	否则，目标初始化为引用上一步得到的值的新创建 TermReference 值。
引用折叠通过调用 ReferenceTerm 再初始化 TermReference 实现，确保不构造引用的引用。
*/
YF_API ReductionStatus
EvaluateIdentifier(TermNode&, const ContextNode&, string_view);

/*!
\brief 求值叶节点记号。
\pre 断言：第三参数的数据指针非空。
\sa CategorizeLexeme
\sa ContextNode::EvaluateLiteral
\sa DeliteralizeUnchecked
\sa EvaluateIdentifier
\since build 736

处理非空字符串表示的节点记号。
依次进行以下求值操作。
对代码字面量，去除字面量边界分隔符后进一步求值。
对数据字面量，去除字面量边界分隔符作为字符串值。
对其它字面量，通过调用字面量遍处理。
最后求值非字面量的标识符。
*/
YF_API ReductionStatus
EvaluateLeafToken(TermNode&, ContextNode&, string_view);
//@}

/*!
\brief 规约合并项：检查项的第一个子项尝试作为操作符进行函数应用，并规范化。
\return 规约状态。
\throw ListReductionFailure 规约失败：枝节点的第一个子项不表示上下文处理器。
\sa ContextHandler
\sa Reduce
\since build 766

对枝节点尝试以第一个子项的 Value 数据成员为上下文处理器并调用，且当规约终止时规范化；
否则视为规约成功，没有其它作用。
若发生 ContextHandler 调用，调用前先转移处理器保证生存期，
	以允许处理器内部移除或修改之前占用的第一个子项（包括其中的 Value 数据成员）。
*/
YF_API ReductionStatus
ReduceCombined(TermNode&, ContextNode&);

/*!
\brief 规约提取名称的叶节点记号。
\exception BadIdentifier 标识符未声明。
\note 忽略名称查找失败，默认结果为 ReductionStatus::Clean 以保证强规范化性质。
\sa EvaluateLeafToken
\sa ReduceAgain
\sa TermToNamePtr
*/
YF_API ReductionStatus
ReduceLeafToken(TermNode&, ContextNode&);
//@}


/*!
\brief 解析名称：处理保留名称并查找名称。
\pre 断言：第二参数的数据指针非空。
\exception NPLException 访问共享重定向上下文失败。
\sa Environment::ResolveName
\since build 777

解析指定上下文中的名称。
*/
YF_API observer_ptr<ValueNode>
ResolveName(const ContextNode&, string_view);

/*!
\brief 解析环境。
\return 取得所有权的环境指针及是否具有所有权。
\note 只支持宿主值类型 \c shared_ptr<Environment> 或 \c weak_ptr<Environment> 。
\since build 798
*/
//@{
YF_API pair<shared_ptr<Environment>, bool>
ResolveEnvironment(ValueObject&);
//! \since build 800
inline PDefH(pair<shared_ptr<Environment> YPP_Comma bool>, ResolveEnvironment,
	TermNode& term)
	ImplRet(ResolveEnvironment(ReferenceTerm(term).Value))
//@}


/*!
\brief 设置默认解释：解释使用的公共处理遍。
\note 非强异常安全：加入遍可能提前设置状态而不在失败时回滚。
\sa ReduceCombined
\sa ReduceFirst
\sa ReduceHeadEmptyList
\sa ReduceLeafToken
\since build 736
*/
YF_API void
SetupDefaultInterpretation(ContextNode&, EvaluationPasses);


/*
\brief REPL 上下文。
\warning 非虚析构。
\since build 740

REPL 表示读取-求值-输出循环。
每个循环包括一次翻译。
这只是一个基本的可扩展实现。功能通过操作数据成员控制。
*/
class YF_API REPLContext
{
public:
	//! \brief 上下文根节点。
	ContextNode Root{};
	//! \brief 预处理节点：每次翻译时预先处理调用的公共例程。
	TermPasses Preprocess{};
	//! \brief 表项处理例程：每次翻译中规约回调处理调用的公共例程。
	EvaluationPasses ListTermPreprocess{};

	/*!
	\brief 构造：使用默认的解释。
	\note 参数指定是否启用对规约深度进行跟踪。
	\sa ListTermPreprocess
	\sa SetupDefaultInterpretation
	\sa SetupTraceDepth
	*/
	REPLContext(bool = {});

	/*!
	\brief 加载：从指定参数指定的来源读取并处理源代码。
	\exception std::invalid_argument 异常中立：由 ReadFrom 抛出。
	\sa ReadFrom
	\sa Reduce
	\since build 802
	*/
	//@{
	template<class _type>
	void
	LoadFrom(_type& input)
	{
		LoadFrom(input, Root);
	}
	template<class _type>
	void
	LoadFrom(_type& input, ContextNode& ctx) const
	{
		auto term(ReadFrom(input));

		Reduce(term, ctx);
	}
	//@}

	/*!
	\brief 执行循环：对非空输入进行翻译。
	\pre 断言：字符串的数据指针非空。
	\throw LoggedEvent 输入为空串。
	\sa Process
	*/
	//@{
	PDefH(TermNode, Perform, string_view unit)
		ImplRet(Perform(unit, Root))
	TermNode
	Perform(string_view, ContextNode&);
	//@}

	/*!
	\brief 准备规约项：分析输入并标记记号节点和预处理。
	\sa Preprocess
	\sa TokenizeTerm
	\since build 802
	*/
	//@{
	void
	Prepare(TermNode&) const;
	/*!
	\return 从参数输入读取的准备的项。
	\sa SContext::Analyze
	*/
	//@{
	TermNode
	Prepare(const TokenList&) const;
	TermNode
	Prepare(const Session&) const;
	//@}
	//@}

	/*!
	\brief 处理：准备规约项并进行规约。
	\sa Prepare
	\sa Reduce
	\since build 742
	*/
	//@{
	PDefH(void, Process, TermNode& term)
		ImplExpr(Process(term, Root))
	//! \since build 802
	//@{
	void
	Process(TermNode&, ContextNode&) const;
	template<class _type>
	TermNode
	Process(const _type& input)
	{
		return Process(input, Root);
	}
	template<class _type>
	TermNode
	Process(const _type& input, ContextNode& ctx) const
	{
		auto term(Prepare(input));

		Reduce(term, ctx);
		return term;
	}
	//@}
	//@}

	/*!
	\brief 读取：从指定参数指定的来源输入源代码并准备规约项。
	\return 从参数输入读取的准备的项。
	\sa Prepare
	\since build 802
	*/
	//@{
	//! \throw std::invalid_argument 流状态错误或缓冲区不存在。
	TermNode
	ReadFrom(std::istream&) const;
	TermNode
	ReadFrom(std::streambuf&) const;
	//@}
};

/*!
\brief 尝试加载源代码。
\exception NPLException 嵌套异常：加载失败。
\note 第二个参数表示来源，仅用于诊断消息。
\relates REPLContext
\since build 805
*/
template<typename... _tParams>
YB_NONNULL(2) void
TryLoadSouce(REPLContext& context, const char* name, _tParams&&... args)
{
	TryExpr(context.LoadFrom(yforward(args)...))
	CatchExpr(..., std::throw_with_nested(NPLException(
		ystdex::sfmt("Failed loading external unit '%s'.", name))));
}


/*!
\brief NPLA1 语法形式对应的功能实现。
\since build 732
*/
namespace Forms
{

/*!
\brief 判断字符串值是否可构成符号。
\since build 779
参考文法：

symbol? <object>
*/
YF_API bool
IsSymbol(const string&) ynothrow;

//! \since build 786
//@{
/*!
\brief 创建等于指定字符串值的记号值。
\note 不检查值是否符合符号要求。
*/
YF_API TokenValue
StringToSymbol(const string&);

//! \brief 取符号对应的名称字符串。
YF_API const string&
SymbolToString(const TokenValue&) ynothrow;
//@}


/*!
\pre 断言：项或容器对应枝节点。
\sa AssertBranch
\since build 733
*/
//@{
/*!
\note 保留求值留作保留用途，一般不需要被作为用户代码直接使用。
\since build 765

可使用 RegisterForm 注册上下文处理器，参考文法：
$retain|$retainN <expression>
*/
//@{
//! \brief 保留项：保留求值。
inline PDefH(ReductionStatus, Retain, const TermNode& term) ynothrowv
	ImplRet(AssertBranch(term), ReductionStatus::Retained)

/*!
\brief 保留经检查确保具有指定个数参数的项：保留求值。
\return 项的参数个数。
\throw ArityMismatch 项的参数个数不等于第二参数。
\sa FetchArgumentN
*/
YF_API size_t
RetainN(const TermNode&, size_t = 1);
//@}


//! \throw ParameterMismatch 匹配失败。
//@{
/*!
\pre 断言：字符串参数的数据指针非空。
\since build 794
*/
//@{
//! \brief 检查记号值是符合匹配条件的符号。
template<typename _func>
auto
CheckSymbol(string_view n, _func f) -> decltype(f())
{
	if(!n.empty() && IsNPLASymbol(n))
		return f();
	else
		throw ParameterMismatch(
			"Invalid token found for symbol parameter.");
}

//! \brief 检查记号值是符合匹配条件的参数符号。
template<typename _func>
auto
CheckParameterLeafToken(string_view n, _func f) -> decltype(f())
{
	if(n != "#ignore")
		CheckSymbol(n, f);
}
//@}

/*!
\note 不具有强异常安全保证。匹配失败时，其它的绑定状态未指定。
\sa LiftToSelf
\since build 776

递归遍历参数和操作数树进行结构化匹配。
若匹配失败，则抛出异常。
*/
//@{
/*!
\brief 使用操作数结构化匹配并绑定参数。
\throw ArityMismatch 子项数匹配失败。
\note 第一参数指定的上下文决定绑定的环境。
\sa MatchParameter

形式参数和操作数为项指定的表达式树。
第二参数指定形式参数，第三参数指定操作数。
进行其它操作前，对操作数调用 LiftToSelf 处理，但不处理形式参数。
进行匹配的算法递归搜索形式参数及其子项，要求参见 MatchParameter 。
若匹配成功，在第一参数指定的环境内绑定未被忽略的匹配的项。
对结尾序列总是匹配前缀为 . 的符号为目标按以下规则忽略或绑定：
子项为 . 时，对应操作数的结尾序列被忽略；
否则，绑定项的目标为移除前缀 . 后的符号。
非列表项的绑定使用以下规则：
若匹配成功，在第一参数指定的环境内绑定未被忽略的匹配的非列表项。
匹配要求如下：
若项是 #ignore ，则忽略操作数对应的项；
若项的值是符号，则操作数的对应的项应为非列表项。
*/
YF_API void
BindParameter(ContextNode&, const TermNode&, TermNode&);

/*!
\brief 匹配参数。
\exception std::bad_function 异常中立：参数指定的处理器为空。
\since build 786

进行匹配的算法递归搜索形式参数及其子项。
若匹配成功，调用参数指定的匹配处理器。
处理器为参数列表结尾的结尾序列处理器和值处理器，分别匹配以 . 起始的项和非列表项。
结尾序列处理器传入的字符串参数表示需绑定的表示结尾序列的列表标识符。
匹配要求如下：
若项是非空列表，则操作数的对应的项应为满足确定子项数的列表：
	若最后的子项为 . 起始的符号，则匹配操作数中结尾的任意个数的项作为结尾序列：
	其它子项一一匹配操作数的子项；
若项是空列表，则操作数的对应的项应为空列表；
否则，匹配非列表项。
*/
YF_API void
MatchParameter(const TermNode&, TermNode&,
	std::function<void(TNIter, TNIter, const TokenValue&)>,
	std::function<void(const TokenValue&, TermNode&&)>);
//@}
//@}
//@}


/*!
\brief 访问节点并调用一元函数。
\sa YSLib::EmplaceCallResult
\since build 756

确定项具有一个实际参数后展开调用参数指定的函数。
若被调用的函数返回类型非 void ，返回值作为项的值被构造。
调用 YSLib::EmplaceCallResult 对 ValueObject 及引用值处理不同。
若需以和其它类型的值类似的方式被包装，在第一个参数中构造 ValueObject 对象。
*/
//@{
template<typename _func, typename... _tParams>
void
CallUnary(_func&& f, TermNode& term, _tParams&&... args)
{
	RetainN(term);
	YSLib::EmplaceCallResult(term.Value, ystdex::invoke_nonvoid(
		ystdex::make_expanded<void(TermNode&, _tParams&&...)>(yforward(f)),
		YSLib::Deref(std::next(term.begin())), yforward(args)...));
}

template<typename _type, typename _func, typename... _tParams>
void
CallUnaryAs(_func&& f, TermNode& term, _tParams&&... args)
{
	Forms::CallUnary([&](TermNode& node){
		// XXX: Blocked. 'yforward' cause G++ 5.3 crash: internal compiler
		//	error: Segmentation fault.
		return ystdex::make_expanded<void(_type&, _tParams&&...)>(yforward(f))(
			NPL::AccessTerm<_type>(node), std::forward<_tParams&&>(args)...);
	}, term);
}
//@}

/*!
\brief 访问节点并调用二元函数。
\since build 760
*/
//@{
template<typename _func, typename... _tParams>
void
CallBinary(_func&& f, TermNode& term, _tParams&&... args)
{
	RetainN(term, 2);

	auto i(term.begin());
	auto& x(YSLib::Deref(++i));

	YSLib::EmplaceCallResult(term.Value, ystdex::invoke_nonvoid(
		ystdex::make_expanded<void(TermNode&, TermNode&, _tParams&&...)>(
		yforward(f)), x, YSLib::Deref(++i), yforward(args)...));
}

template<typename _type, typename _func, typename... _tParams>
void
CallBinaryAs(_func&& f, TermNode& term, _tParams&&... args)
{
	RetainN(term, 2);

	auto i(term.begin());
	auto& x(NPL::AccessTerm<_type>(YSLib::Deref(++i)));

	YSLib::EmplaceCallResult(term.Value, ystdex::invoke_nonvoid(
		ystdex::make_expanded<void(_type&, _type&, _tParams&&...)>(yforward(f)),
		x, NPL::AccessTerm<_type>(YSLib::Deref(++i)), yforward(args)...));
}
//@}

/*!
\brief 访问节点并以指定的初始值为基础逐项调用二元函数。
\note 为支持 std::bind 推断类型，和以上函数的情形不同，不支持省略参数。
\since build 758
*/
template<typename _type, typename _func, typename... _tParams>
void
CallBinaryFold(_func f, _type val, TermNode& term, _tParams&&... args)
{
	const auto n(FetchArgumentN(term));
	auto i(term.begin());
	const auto j(ystdex::make_transform(++i, [](TNIter it){
		return NPL::AccessTerm<_type>(YSLib::Deref(it));
	}));

	YSLib::EmplaceCallResult(term.Value, std::accumulate(j, std::next(j,
		typename std::iterator_traits<decltype(j)>::difference_type(n)), val,
		ystdex::bind1(f, std::placeholders::_2, yforward(args)...)));
}


/*!
\brief 保存函数展开调用的函数对象。
\todo 使用 C++1y lambda 表达式代替。

适配作为上下文处理器的除项以外可选参数的函数对象。
为适合作为上下文处理器，支持的参数列表类型实际存在限制：
参数列表以和元数相同数量的必须的 TermNode& 类型的参数起始；
之后是可选的 ContextNode& 可转换到的类型的参数。
*/
//@{
//! \since build 741
//@{
//! \sa Forms::CallUnary
template<typename _func>
struct UnaryExpansion
	: private ystdex::equality_comparable<UnaryExpansion<_func>>
{
	_func Function;

	//! \since build 787
	UnaryExpansion(_func f)
		: Function(f)
	{}

	/*!
	\brief 比较处理器相等。
	\since build 773
	*/
	friend PDefHOp(bool, ==, const UnaryExpansion& x, const UnaryExpansion& y)
		ImplRet(ystdex::examiners::equal_examiner::are_equal(x.Function,
			y.Function))

	//! \since build 760
	template<typename... _tParams>
	inline void
	operator()(_tParams&&... args) const
	{
		Forms::CallUnary(Function, yforward(args)...);
	}
};


//! \sa Forms::CallUnaryAs
template<typename _type, typename _func>
struct UnaryAsExpansion
	: private ystdex::equality_comparable<UnaryAsExpansion<_type, _func>>
{
	_func Function;

	//! \since build 787
	UnaryAsExpansion(_func f)
		: Function(f)
	{}

	/*!
	\brief 比较处理器相等。
	\since build 773
	*/
	friend PDefHOp(bool, ==, const UnaryAsExpansion& x,
		const UnaryAsExpansion& y)
		ImplRet(ystdex::examiners::equal_examiner::are_equal(x.Function,
			y.Function))

	//! \since build 760
	template<typename... _tParams>
	inline void
	operator()(_tParams&&... args) const
	{
		Forms::CallUnaryAs<_type>(Function, yforward(args)...);
	}
};
//@}


//! \since build 760
//@{
//! \sa Forms::CallBinary
template<typename _func>
struct BinaryExpansion
	: private ystdex::equality_comparable<BinaryExpansion<_func>>
{
	_func Function;

	//! \since build 787
	BinaryExpansion(_func f)
		: Function(f)
	{}

	/*!
	\brief 比较处理器相等。
	\since build 773
	*/
	friend PDefHOp(bool, ==, const BinaryExpansion& x, const BinaryExpansion& y)
		ImplRet(ystdex::examiners::equal_examiner::are_equal(x.Function,
			y.Function))

	template<typename... _tParams>
	inline void
	operator()(_tParams&&... args) const
	{
		Forms::CallBinary(Function, yforward(args)...);
	}
};


//! \sa Forms::CallBinaryAs
template<typename _type, typename _func>
struct BinaryAsExpansion
	: private ystdex::equality_comparable<BinaryAsExpansion<_type, _func>>
{
	_func Function;

	//! \since build 787
	BinaryAsExpansion(_func f)
		: Function(f)
	{}

	/*!
	\brief 比较处理器相等。
	\since build 773
	*/
	friend PDefHOp(bool, ==, const BinaryAsExpansion& x,
		const BinaryAsExpansion& y)
		ImplRet(ystdex::examiners::equal_examiner::are_equal(x.Function,
			y.Function))

	template<typename... _tParams>
	inline void
	operator()(_tParams&&... args) const
	{
		Forms::CallBinaryAs<_type>(Function, yforward(args)...);
	}
};
//@}
//@}


/*!
\brief 注册一元严格求值上下文处理器。
\since build 754
*/
//@{
template<typename _func>
void
RegisterStrictUnary(ContextNode& ctx, const string& name, _func f)
{
	RegisterStrict(ctx, name, UnaryExpansion<_func>(f));
}
template<typename _type, typename _func>
void
RegisterStrictUnary(ContextNode& ctx, const string& name, _func f)
{
	RegisterStrict(ctx, name, UnaryAsExpansion<_type, _func>(f));
}
//@}

/*!
\brief 注册二元严格求值上下文处理器。
\since build 760
*/
//@{
template<typename _func>
void
RegisterStrictBinary(ContextNode& ctx, const string& name, _func f)
{
	RegisterStrict(ctx, name, BinaryExpansion<_func>(f));
}
template<typename _type, typename _func>
void
RegisterStrictBinary(ContextNode& ctx, const string& name, _func f)
{
	RegisterStrict(ctx, name, BinaryAsExpansion<_type, _func>(f));
}
//@}


//! \pre 间接断言：第一参数指定的项是枝节点。
//@{
/*
\note 实现特殊形式。
\throw InvalidSyntax 语法错误。
*/
//@{
/*!
\brief 定义。
\exception ParameterMismatch 绑定匹配失败。
\throw InvalidSyntax 绑定的源为空。
\sa BindParameter
\sa Vau
\since build 780

实现修改环境的特殊形式。
使用 <definiend> 指定绑定目标，和 Vau 的 <formals> 格式相同。
剩余表达式 <expressions> 指定绑定的源。
返回未指定值。
限定第三参数后可使用 RegisterForm 注册上下文处理器。
*/
//@{
/*!
\note 不对剩余表达式进一步求值。

剩余表达式视为求值结果，直接绑定到 <definiend> 。
参考调用文法：
$deflazy! <definiend> <expressions>
*/
YF_API void
DefineLazy(TermNode&, ContextNode&);

/*!
\note 不支持递归绑定。

剩余表达式视为一个表达式进行求值后绑定到 <definiend> 。
参考调用文法：
$def! <definiend> <expressions>
*/
YF_API void
DefineWithNoRecursion(TermNode&, ContextNode&);

/*!
\note 支持直接递归和互相递归绑定。

解析可能递归绑定的名称，剩余表达式视为一个表达式进行求值后绑定到 <definiend> 。
$defrec! <definiend> <expressions>
*/
YF_API void
DefineWithRecursion(TermNode&, ContextNode&);
//@}

/*!
\brief 移除名称绑定。
\exception BadIdentifier 非强制时移除不存在的名称。
\throw InvalidSyntax 标识符不是符号。
\sa IsNPLASymbol
\sa RemoveIdentifier
\since build 786

移除名称和关联的值，返回是否被移除。
第三个参数表示是否强制。若非强制，移除不存在的名称抛出异常。
参考调用文法：
$undef! <symbol>
$undef-checked! <symbol>
*/
YF_API void
Undefine(TermNode&, ContextNode&, bool);


/*!
\brief 条件判断：根据求值的条件取表达式。
\sa ReduceChecked
\since build 750

求值第一子项作为测试条件，成立时取第二子项，否则当第三子项时取第三子项。
测试条件成立，当且仅当 <test> 非 #f 。
和 Kernel 不同而和 Scheme 类似，求值结果非 #f 的条件都成立，且支持省略第三操作数。
参考调用文法：
$if <test> <consequent> <alternate>
$if <test> <consequent>
*/
YF_API ReductionStatus
If(TermNode&, ContextNode&);

/*!
\exception ParameterMismatch 异常中立：由 BindParameter 抛出。
\sa EvaluateIdentifier
\sa ExtractParameters
\sa MatchParameter
\warning 返回闭包调用引用变量超出绑定目标的生存期引起未定义行为。
\todo 优化捕获开销。

使用 ExtractParameters 检查参数列表并捕获和绑定变量，
然后设置节点的值为表示 λ 抽象的上下文处理器。
可使用 RegisterForm 注册上下文处理器。
和 Scheme 等不同，参数以项而不是位置的形式被转移，函数应用时可能有副作用。
按引用捕获上下文中的绑定。被捕获的上下文中的绑定依赖宿主语言的生存期规则。
*/
//@{
/*!
\brief λ 抽象：求值为一个捕获当前上下文的严格求值的函数。
\since build 735

捕获的静态环境由当前动态环境隐式确定。
不保留环境的所有权。
参考调用文法：
$lambda <formals> <body>
*/
YF_API void
Lambda(TermNode&, ContextNode&);

/*!
\note 动态环境的上下文参数被捕获为一个 ystdex::ref<ContextNode> 对象。
\note 初始化的 <eformal> 表示动态环境的上下文参数，应为一个符号或 #ignore 。
\throw InvalidSyntax <eformal> 不符合要求。
\sa ReduceCheckedClosure
\since build 790

上下文中环境以外的数据成员总是被复制而不被转移，
	以避免求值过程中继续访问这些成员引起未定义行为。
*/
//@{
/*!
\brief vau 抽象：求值为一个捕获当前上下文的非严格求值的函数。

捕获的静态环境由当前动态环境隐式确定。
不保留环境的所有权。
参考调用文法：
$vau <formals> <eformal> <body>
*/
YF_API void
Vau(TermNode&, ContextNode&);

/*!
\brief 带环境的 vau 抽象：求值为一个捕获当前上下文的非严格求值的函数。
\sa ResolveEnvironment

捕获的静态环境由环境参数 <env> 求值后指定。
根据环境参数的类型为 \c shared_ptr<Environment> 或 \c weak_ptr<Environment>
	决定是否保留所有权。
参考调用文法：
$vaue <env> <formals> <eformal> <body>
*/
YF_API void
VauWithEnvironment(TermNode&, ContextNode&);
//@}
//@}


/*!
\sa ReduceChecked
\since build 754
*/
//@{
/*!
\brief 逻辑与。

非严格求值若干个子项，返回求值结果的逻辑与：
除第一个子项，没有其它子项时，返回 true ；否则从左到右逐个求值子项。
当子项全求值为 true 时返回最后一个子项的值，否则返回 false 。
参考调用文法：
$and? <test>...
*/
YF_API ReductionStatus
And(TermNode&, ContextNode&);

/*!
\brief 逻辑或。

非严格求值若干个子项，返回求值结果的逻辑或：
除第一个子项，没有其它子项时，返回 false ；否则从左到右逐个求值子项。
当子项全求值为 false 时返回 false，否则返回第一个不是 false 的子项的值。
参考调用文法：
$or? <test>...
*/
YF_API ReductionStatus
Or(TermNode&, ContextNode&);
//@}
//@}


/*!
\brief 调用 UTF-8 字符串的系统命令，并保存 int 类型的结果到项的值中。
\sa usystem
\since build 741

参考调用文法：
system <string>
*/
YF_API void
CallSystem(TermNode&);

/*!
\brief 接受两个参数，返回以第一个参数作为首项插入第二个参数创建的新的列表。
\return ReductionStatus::Retained 。
\throw InvalidSyntax 第二个参数不是列表。
\note NPLA 无 cons 对，所以要求创建的总是列表。
\since build 779

参考调用文法：
cons <object> <list>
*/
YF_API ReductionStatus
Cons(TermNode&);

/*!
\brief 比较两个子项表示的值引用相同的对象。
\sa YSLib::HoldSame
\since build 804

参考调用文法：
eq? <object1> <object2>
*/
YF_API void
Equal(TermNode&);

/*!
\brief 比较两个子项的值相等。
\sa YSLib::ValueObject
\since build 804

参考调用文法：
eql? <object1> <object2>
*/
YF_API void
EqualLeaf(TermNode&);

//! \since build 748
//@{
/*!
\brief 比较两个子项的值引用相同的对象。
\sa YSLib::HoldSame

参考调用文法：
eqr? <object1> <object2>
*/
YF_API void
EqualReference(TermNode&);

/*!
\brief 比较两个子项表示的值相等。
\sa YSLib::ValueObject

参考调用文法：
eqv? <object1> <object2>
*/
YF_API void
EqualValue(TermNode&);
//@}

//@{
/*!
\brief 对指定项按指定的环境求值。
\since build 787
\sa ReduceCheckedClosure
\sa ResolveEnvironment

以表达式 <expression> 和环境 <environment> 为指定的参数进行求值。
环境以 ContextNode 的引用表示。
参考调用文法：
eval <expression> <environment>
*/
YF_API ReductionStatus
Eval(TermNode&, ContextNode&);

/*!
\brief 创建参数指定的 REPL 的副本并在其中对翻译单元规约以求值。
\since build 772
*/
YF_API void
EvaluateUnit(TermNode&, const REPLContext&);
//@}

/*!
\brief 创建以参数指定的环境列表作为父环境的新环境。
\exception NPLException 异常中立：由 Environment 的构造函数抛出。
\sa Environment::CheckParent
\sa EnvironmentList
\since build 798
\todo 使用专用的异常类型。

取以指定的参数初始化新创建的父环境。
参考调用文法：
make-environment <environment>...
*/
YF_API void
MakeEnvironment(TermNode&);

/*!
\brief 取当前环境的引用。
\since build 785

取得的宿主值类型为 weak_ptr<Environment> 。
参考调用文法：
() get-current-environment
*/
YF_API void
GetCurrentEnvironment(TermNode&, ContextNode&);

/*!
\brief 求值标识符得到指称的实体。
\sa EvaluateIdentifier
\since build 757

在对象语言中实现函数接受一个 string 类型的参数项，返回值为指定的实体。
当名称查找失败时，返回的值为 ValueToken::Null 。
参考调用文法：
value-of <object>
*/
YF_API ReductionStatus
ValueOf(TermNode&, const ContextNode&);
//@}


//! \since build 785
//@{
/*!
\brief 包装合并子为应用子。

参考调用文法：
wrap <combiner>
*/
YF_API ContextHandler
Wrap(const ContextHandler&);

//! \exception NPLException 类型不符合要求。
//@{
/*!
\brief 包装操作子为应用子。

参考调用文法：
wrap1 <operative>
*/
YF_API ContextHandler
WrapOnce(const ContextHandler&);

/*!
\brief 从应用子中取操作子。

参考调用文法：
unwrap <applicative>
*/
YF_API ContextHandler
Unwrap(const ContextHandler&);
//@}
//@}

} // namespace Forms;

} // namesapce A1;

} // namespace NPL;

#endif

