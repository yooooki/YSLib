﻿/*
	© 2011-2015 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file FileIO.h
\ingroup YCLib
\brief 平台相关的文件访问和输入/输出接口。
\version r1831
\author FrankHB <frankhb1989@gmail.com>
\since build 616
\par 创建时间:
	2015-07-14 18:50:35 +0800
\par 修改时间:
	2015-12-26 02:08 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YCLib::FileIO
*/


#ifndef YCL_INC_FileIO_h_
#define YCL_INC_FileIO_h_ 1

#include "YModules.h"
#include YFM_YCLib_Platform
#include YFM_YBaseMacro
#include <ystdex/string.hpp> // for std::uint32_t, std::uint64_t, ynothrow,
//	ystdex::totally_ordered, std::FILE, ystdex::enable_for_string_class_t;
#include YFM_YCLib_Debug // for Nonnull, string, u16string, array;
#include <chrono> // for std::chrono::nanoseconds;
#include YFM_YCLib_Reference // for unique_ptr;
#include <ios> // for std::ios_base::sync_with_stdio;
#include <fstream> // for std::filebuf;
#if __GLIBCXX__
#	include <ext/stdio_filebuf.h> // for __gnu_cxx::stdio_filebuf;
#	include <ystdex/utility.hpp> // for ystdex::swap_underlying,
//	ystdex::exchange;
#endif
#include <system_error> // for std::system_error;

namespace platform
{

/*!
\brief 构造适合表示路径的字符串。
\note 字符类型非 \c char 时转换，假定为 UTF-8 编码。
\since build 634
*/
//@{
//! \pre 间接断言：参数非空。
inline PDefH(string, MakePathString, const char* s)
	ImplRet(Nonnull(s))
inline PDefH(const string&, MakePathString, const string& s)
	ImplRet(s)
//! \pre 间接断言：参数非空。
YF_API string
MakePathString(const char16_t*);
//! \since build 658
inline PDefH(string, MakePathString, u16string_view sv)
	ImplRet(MakePathString(sv.data()))
//@}


//! \since build 638
//@{
//! \brief 文件节点标识类型。
//@{
#if YCL_Win32
using FileNodeID = pair<std::uint32_t, std::uint64_t>;
#else
using FileNodeID = pair<std::uint64_t, std::uint64_t>;
#endif
//@}

//! \relates FileNodeID
yconstfn PDefHOp(bool, ==, const FileNodeID& x, const FileNodeID& y)
	ImplRet(x.first == y.first && x.second == y.second)
yconstfn PDefHOp(bool, !=, const FileNodeID& x, const FileNodeID& y)
	ImplRet(!(x == y))
//@}


//! \since build 628
using FileTime = std::chrono::nanoseconds;


/*!
\brief 文件模式类型。
\since build 626
*/
//@{
#if YCL_Win32
using mode_t = unsigned short;
#else
using mode_t = ::mode_t;
#endif
//@}


/*!
\brief 文件节点类别。
\since build 412
*/
enum class NodeCategory : std::uint_least32_t
{
	Empty = 0,
	//! \since build 474
	//@{
	Invalid = 1 << 0,
	Regular = 1 << 1,
	//@}
	Unknown = Invalid | Regular,
	//! \since build 474
	//@{
	Device = 1 << 9,
	Block = Device,
	Character = Device | 1 << 7,
	Communicator = 2 << 9,
	FIFO = Communicator | 1 << 6,
	Socket = Communicator | 2 << 6,
	//@}
	SymbolicLink = 1 << 12,
	MountPoint = 2 << 12,
	Junction = MountPoint,
	//! \since build 474
	//@{
	Link = SymbolicLink | Junction,
	//@}
	Directory = 1 << 15,
	//! \since build 474
	//@{
	Missing = 1 << 16,
	Special = Link | Missing
	//@}
};

//! \relates NodeCategory
//@{
//! \since build 543
DefBitmaskEnum(NodeCategory)

//! \brief 取指定模式对应的文件节点类型。
YF_API NodeCategory
CategorizeNode(mode_t) ynothrow;
//@}


/*!
\brief 文件描述符包装类。
\note 除非另行约定，具有无异常抛出保证的操作失败时可能设置 errno 。
\note 不支持的平台操作失败设置 errno 为 ENOSYS 。
\note 除非另行约定，无异常抛出的操作使用值初始化的返回类型表示失败结果。
\note 以 \c int 为返回值的操作返回 \c -1 表示失败。
\note 满足 NullablePointer 要求。
\see WG21/N4140 17.6.3.3[nullablepointer.requirements] 。
\since build 565
*/
class YF_API FileDescriptor : ystdex::totally_ordered<FileDescriptor>
{
public:
	/*!
	\brief 删除器。
	\since build 624
	*/
	struct YF_API Deleter
	{
		using pointer = FileDescriptor;

		void
		operator()(pointer) ynothrow;
	};

private:
	//! \since build 554
	int desc = -1;

public:
	yconstfn DefDeCtor(FileDescriptor)
	yconstfn
	FileDescriptor(int fd) ynothrow
		: desc(fd)
	{}
	//! \since build 644
	yconstfn
	FileDescriptor(nullptr_t) ynothrow
	{}
	/*!
	\brief 构造：使用标准流。
	\note 对空参数不设置 errno 。

	当参数为空时得到无效文件空描述符，否则调用 POSIX \c fileno 函数。
	*/
	FileDescriptor(std::FILE*) ynothrow;

	//! \since build 628
	PDefHOp(int, *, ) const ynothrow
		ImplRet(desc)

	//! \since build 625
	PDefHOp(FileDescriptor*, ->, ) ynothrow
		ImplRet(this)
	//! \since build 625
	PDefHOp(const FileDescriptor*, ->, ) const ynothrow
		ImplRet(this)

	explicit DefCvt(const ynothrow, bool, desc != -1)

	//! \since build 639
	friend yconstfn YB_PURE PDefHOp(bool,
		==, const FileDescriptor& x, const FileDescriptor& y) ynothrow
		ImplRet(x.desc == y.desc)

	//! \since build 639
	friend yconstfn YB_PURE PDefHOp(bool,
		<, const FileDescriptor& x, const FileDescriptor& y) ynothrow
		ImplRet(x.desc < y.desc)

	//! \since build 628
	//@{
	/*!
	\return 以 POSIX 时间相同历元的时间间隔。
	\throw FileOperationFailure 参数无效或文件修改时间查询失败。
	\note 当前 Windows 使用 \c ::GetFileTime 实现，其它只保证最高精确到秒。
	*/
	//@{
	//! \brief 取访问时间。
	FileTime
	GetAccessTime() const;
	/*!
	\brief 取节点类别。
	\return 失败时为 NodeCategory::Invalid ，否则为对应类别。
	\since build 638
	*/
	NodeCategory
	GetCategory() const ynothrow;
	//! \since build 637
	//@{
	/*!
	\brief 取旗标。
	\note 非 POSIX 平台：不支持操作。
	*/
	int
	GetFlags() const ynothrow;
	//! \brief 取模式。
	mode_t
	GetMode() const ynothrow;
	//@}
	//! \brief 取修改时间。
	FileTime
	GetModificationTime() const;
	//! \brief 取修改和访问时间。
	array<FileTime, 2>
	GetModificationAndAccessTime() const;
	//@}
	/*!
	\note 若存储分配失败，设置 errno 为 \c ENOMEM 。
	\since build 638
	*/
	//@{
	//! \brief 取文件系统节点标识。
	FileNodeID
	GetNodeID() const ynothrow;
	//! \brief 取链接数。
	size_t
	GetNumberOfLinks() const ynothrow;
	//@}
	/*!
	\brief 取大小。
	\return 以字节计算的文件大小。
	\note 非常规文件可能出错。
	\throw FileOperationFailure 参数无效或文件大小查询失败。
	*/
	std::uint64_t
	GetSize() const;
	//@}

	/*!
	\brief 设置访问时间。
	\throw FileOperationFailure 设置失败。
	\throw std::system_error 本机 API 调用失败。
	\note DS 平台：不支持操作。
	\since build 651
	*/
	void
	SetAccessTime(FileTime) const;
	/*!
	\note 非 POSIX 平台：不支持操作。
	\since build 637
	*/
	//@{
	/*!
	\brief 设置阻塞模式。
	\see http://pubs.opengroup.org/onlinepubs/9699919799/functions/fcntl.html 。
	\since build 625
	*/
	bool
	SetBlocking() const ynothrow;
	//! \brief 设置旗标。
	bool
	SetFlags(int) const ynothrow;
	//! \brief 设置访问模式。
	bool
	SetMode(mode_t) const ynothrow;
	//@}
	/*!
	\throw FileOperationFailure 设置失败。
	\throw std::system_error 本机 API 调用失败。
	\note DS 平台：不支持操作。
	\note Win32 平台：要求打开的文件具有写权限。
	\since build 651
	*/
	//@{
	//! \brief 设置修改时间。
	void
	SetModificationTime(FileTime) const;
	//! \brief 设置修改和访问时间。
	void
	SetModificationAndAccessTime(array<FileTime, 2>) const;
	//@}
	/*!
	\note 非 POSIX 平台：不支持操作。
	\since build 637
	*/
	//@{
	/*!
	\brief 设置非阻塞模式。
	\note 对不支持非阻塞的文件描述符， POSIX 未指定是否忽略 \c O_NONBLOCK 。
	\see http://pubs.opengroup.org/onlinepubs/9699919799/functions/fcntl.html 。
	\since build 624
	*/
	bool
	SetNonblocking() const ynothrow;
	//@}
	//! \since build 625
	//@{
	/*!
	\brief 调整文件至指定长度。
	\pre 指定文件需已经打开并可写。
	\note 不改变文件读写位置。

	若文件不足指定长度，扩展并使用空字节填充；否则保留起始指定长度的字节。
	*/
	bool
	SetSize(size_t) ynothrow;
	/*!
	\brief 设置翻译模式。
	\note 兼容 Win32 文本模式行为的平台：参数和返回值意义和语义同 \c setmode 函数。
	\note 其它平台：无作用。
	\since build 637
	*/
	int
	SetTranslationMode(int) const ynothrow;

	//! \note 每次读写首先清除 errno ；读写时遇 EINTR 时继续。
	//@{
	/*!
	\brief 循环读写文件。
	*/
	//@{
	/*!
	\note 每次读 0 字节时设置 errno 为 0 。
	\sa Read
	*/
	YB_NONNULL(2) size_t
	FullRead(void*, size_t) ynothrowv;

	/*!
	\note 每次写 0 字节时设置 errno 为 ENOSPC 。
	\sa Write
	*/
	YB_NONNULL(2) size_t
	FullWrite(const void*, size_t) ynothrowv;
	//@}

	/*!
	\brief 读写文件。
	\note 首先清除 errno 。
	\note Win64 平台：大小截断为 32 位。
	\return 若发生错误为 size_t(-1) ，否则为读取的字节数。
	*/
	//@{
	YB_NONNULL(2) size_t
	Read(void*, size_t) ynothrowv;

	YB_NONNULL(2) size_t
	Write(const void*, size_t) ynothrowv;
	//@}
	//@}
	//@}

	/*!
	\brief 第二参数内容写入第一参数指定的文件。
	\pre 最后参数指定的缓冲区大小不等于 0 。
	\throw FileOperationFailure 文件读写失败。
	\since build 634
	*/
	//@{
	//! \pre 参数指定的缓冲区非空且大小不等于 0 。
	static YB_NONNULL(3) void
	WriteContent(FileDescriptor, FileDescriptor, byte*, size_t);
	/*!
	\note 最后一个参数指定缓冲区大小的上限，若分配失败自动重新分配。
	\throw std::bad_alloc 缓冲区分配失败。
	*/
	static void
	WriteContent(FileDescriptor, FileDescriptor,
		size_t = yimpl(size_t(BUFSIZ << 4)));
	//@}
};


//! \since build 626
using UniqueFile = unique_ptr<FileDescriptor, FileDescriptor::Deleter>;


/*!
\brief 取默认权限。
\since build 638
*/
YF_API YB_STATELESS mode_t
DefaultPMode() ynothrow;

/*!
\brief 设置标准库流二进制输入/输出模式。
\pre 间接断言：参数非空。
\since build 599
*/
//@{
YF_API void
SetBinaryIO(std::FILE*) ynothrow;

/*!
\warning 改变默认日志默认发送器前，不应使用 std::cerr 和 std::clog
	等依赖 \c stderr 的流，以避免导致同步问题。
\sa FetchCommonLogger
\sa Logger::DefaultSendLog
*/
inline PDefH(void, SetupBinaryStdIO, std::FILE* in = stdin,
	std::FILE* out = stdout, bool sync = {}) ynothrow
	ImplExpr(SetBinaryIO(in), SetBinaryIO(out),
		std::ios_base::sync_with_stdio(sync))
//@}

/*!
\brief 重复尝试关闭流：设置 \c error 后关闭参数指定的流，必要时重试。
\pre 参数非空。
\return 非 EINTR 的错误。
\note 首先清除 errno ；遇 EINTR 时重试。
\note 使用 std::fclose 关闭流。
\since build 634
*/
YF_API YB_NONNULL(1) int
RetryClose(std::FILE*) ynothrow;


/*!
\brief ISO C++ 标准输入输出接口打开模式转换为 POSIX 文件打开模式。
\return 若失败为 0 ，否则为对应的值。
\since build 648
*/
//@{
//! \note 忽略二进制模式。
YF_API YB_STATELESS int
omode_conv(std::ios_base::openmode) ynothrow;

/*!
\note 扩展：不忽略二进制模式。
\note POSIX 平台：同 omode_conv 。
*/
YF_API YB_STATELESS int
omode_convb(std::ios_base::openmode) ynothrow;
//@}

/*!
\pre 断言：第一参数非空。
\note 若存储分配失败，设置 errno 为 \c ENOMEM 。
*/
//@{
/*!
\brief 测试路径可访问性。
\param path 路径，意义同 POSIX <tt>::open</tt> 。
\param amode 模式，基本语义同 POSIX.1 2004 ，具体行为取决于实现。 。
\note errno 在出错时会被设置，具体值由实现定义。
\since build 549
*/
//@{
YF_API YB_NONNULL(1) int
uaccess(const char* path, int amode) ynothrow;
YF_API YB_NONNULL(1) int
uaccess(const char16_t* path, int amode) ynothrow;
//@}

/*!
\param filename 文件名，意义同 POSIX \c ::open 。
\param oflag 打开旗标，基本语义同 POSIX.1 2004 ，具体行为取决于实现。
\param pmode 打开模式，基本语义同 POSIX.1 2004 ，具体行为取决于实现。
\since build 626
*/
//@{
//! \brief 以 UTF-8 文件名无缓冲打开文件。
YF_API YB_NONNULL(1) int
uopen(const char* filename, int oflag, mode_t pmode = 0) ynothrow;
//! \brief 以 UCS-2 文件名无缓冲打开文件。
YF_API YB_NONNULL(1) int
uopen(const char16_t* filename, int oflag, mode_t pmode = 0) ynothrow;
//@}

//! \param filename 文件名，意义同 std::fopen 。
//@{
/*!
\param mode 打开模式，基本语义同 ISO C11 ，具体行为取决于实现。
\pre 断言：<tt>mode && *mode != 0</tt> 。
*/
//@{
/*!
\brief 以 UTF-8 文件名打开文件。
\since build 299
*/
YF_API YB_NONNULL(1, 2) std::FILE*
ufopen(const char* filename, const char* mode) ynothrow;
/*!
\brief 以 UCS-2 文件名打开文件。
\since build 324
*/
YF_API YB_NONNULL(1, 2) std::FILE*
ufopen(const char16_t* filename, const char16_t* mode) ynothrow;
//@}
/*!
\param mode 打开模式，基本语义与 ISO C++11 对应，具体行为取决于实现。
\since build 616
*/
//@{
//! \brief 以 UTF-8 文件名打开文件。
YF_API YB_NONNULL(1) std::FILE*
ufopen(const char* filename, std::ios_base::openmode mode) ynothrow;
//! \brief 以 UCS-2 文件名打开文件。
YF_API YB_NONNULL(1) std::FILE*
ufopen(const char16_t* filename, std::ios_base::openmode mode) ynothrow;
//@}

//! \note 使用 ufopen 二进制只读模式打开测试实现。
//@{
//! \brief 判断指定 UTF-8 文件名的文件是否存在。
YF_API YB_NONNULL(1) bool
ufexists(const char*) ynothrow;
//! \brief 判断指定 UCS-2 文件名的文件是否存在。
YF_API YB_NONNULL(1) bool
ufexists(const char16_t*) ynothrow;
//@}
//@}

/*!
\brief 关闭管道流。
\param 基本语义同 POSIX.1 2004 的 \c ::pclose ，具体行为取决于实现。
\pre 参数非空，表示通过和 upopen 或使用相同实现打开的管道流。
\note DS 平台：不支持操作，总是失败并设置 errno 为 ENOSYS 。
\since build 566
*/
YF_API YB_NONNULL(1) int
upclose(std::FILE*) ynothrow;

//! \note 若存储分配失败，设置 errno 为 \c ENOMEM 。
//@{
/*!
\param filename 文件名，意义同 POSIX \c ::popen 。
\param mode 打开模式，基本语义同 POSIX.1 2004 ，具体行为取决于实现。
\pre 断言：\c filename 。
\pre 间接断言： \c mode 。
\warning 应使用 upclose 而不是 std::close 关闭管道流，否则行为可能未定义。
\note DS 平台：不支持操作，总是失败并设置 errno 为 ENOSYS 。
\since build 566
*/
//@{
//! \brief 以 UTF-8 文件名无缓冲打开管道流。
YF_API YB_NONNULL(1, 2) std::FILE*
upopen(const char* filename, const char* mode) ynothrow;
//! \brief 以 UCS-2 文件名无缓冲打开管道流。
YF_API YB_NONNULL(1, 2) std::FILE*
upopen(const char16_t* filename, const char16_t* mode) ynothrow;
//@}

/*!
\brief 取当前工作目录（ UCS-2 编码）复制至指定缓冲区中。
\param buf 缓冲区起始指针。
\param size 缓冲区长。
\return 若成功为 buf ，否则为空指针。
\note 当 <tt>!buf || size == 0</tt> 时失败，设置 errno 为 EINVAL 。
\note 指定的 size 不能容纳结果时失败，设置 errno 为 \c ERANGE 。
\since build 324
*/
YF_API YB_NONNULL(1) char16_t*
u16getcwd_n(char16_t* buf, size_t size) ynothrow;

/*!
\pre 断言：参数非空。
\return 操作是否成功。
\note errno 在出错时会被设置，具体值除以上明确的外，由实现定义。
\note 参数表示路径，使用 UTF-8 编码。
\note DS 使用 newlib 实现。 MinGW32 使用 MSVCRT 实现。 Android 使用 bionic 实现。
	其它 Linux 使用 GLibC 实现。
*/
//@{
/*!
\brief 切换当前工作路径至指定路径。
\note POSIX 平台：除路径和返回值外语义同 \c ::chdir 。
\since build 476
*/
YF_API YB_NONNULL(1) bool
uchdir(const char*) ynothrow;

/*!
\brief 按路径以默认权限新建一个目录。
\note 权限由实现定义： DS 使用最大权限； MinGW32 使用 \c ::_wmkdir 指定的默认权限。
\since build 475
*/
YF_API YB_NONNULL(1) bool
umkdir(const char*) ynothrow;

/*!
\brief 按路径删除一个空目录。
\note POSIX 平台：除路径和返回值外语义同 \c ::rmdir 。
\since build 475
*/
YF_API YB_NONNULL(1) bool
urmdir(const char*) ynothrow;

/*!
\brief 按路径删除一个非目录文件。
\note POSIX 平台：除路径和返回值外语义同 \c ::unlink 。
\note Win32 平台：支持移除只读文件，但删除打开的文件总是失败。
\since build 476
*/
YF_API YB_NONNULL(1) bool
uunlink(const char*) ynothrow;

/*!
\brief 按路径移除一个文件。
\note POSIX 平台：除路径和返回值外语义同 \c ::remove 。移除非空目录总是失败。
\note Win32 平台：支持移除空目录和只读文件，但删除打开的文件总是失败。
\see https://msdn.microsoft.com/en-us/library/kc07117k.aspx 。
\since build 476
*/
YF_API YB_NONNULL(1) bool
uremove(const char*) ynothrow;
//@}
//@}


//! \since build 616
//@{
#if __GLIBCXX__
template<typename _tChar, class _tTraits = std::char_traits<_tChar>>
class basic_filebuf : public yimpl(__gnu_cxx::stdio_filebuf<_tChar, _tTraits>)
{
public:
	using char_type = _tChar;
	using int_type = typename _tTraits::int_type;
	using pos_type = typename _tTraits::pos_type;
	using off_type = typename _tTraits::off_type;
	using traits = _tTraits;

	//! \since build 620
	DefDeCtor(basic_filebuf)
	using yimpl(__gnu_cxx::stdio_filebuf<_tChar, _tTraits>::stdio_filebuf);
#	if YB_IMPL_GNUCPP && !(YB_IMPL_GNUCPP >= 50000 && __GLIBCXX__ > 20140922)
	//! \since build 620
	//@{
	DefDelCopyCtor(basic_filebuf)
	basic_filebuf(basic_filebuf&& rhs)
		: __gnu_cxx::stdio_filebuf<_tChar, _tTraits>()
	{
		assign(std::move(rhs)),
		yunseq(this->_M_pback = rhs._M_pback,
			this->_M_codecvt = rhs._M_codecvt);
	}

	DefDelCopyAssignment(basic_filebuf)
	basic_filebuf&
	operator=(basic_filebuf&& rhs)
	{
		this->close();
		ystdex::swap_underlying(this->_M_file, rhs._M_file),
		assign(std::move(rhs));
		return *this;
	}

private:
	void
	assign(basic_filebuf&& rhs)
	{
	//	static_cast<std::basic_streambuf<_tChar, _tTraits>&>(*this) = rhs;
		yunseq(
		this->_M_mode
			= ystdex::exchange(rhs._M_mode, std::ios_base::openmode(0)),
		this->_M_state_beg = std::move(rhs._M_state_beg),
		this->_M_state_cur = std::move(rhs._M_state_cur),
		this->_M_state_last = std::move(rhs._M_state_last),
		this->_M_buf = ystdex::exchange(rhs._M_buf, {}),
		this->_M_buf_size = ystdex::exchange(rhs._M_buf_size, 1U),
		this->_M_buf_allocated = ystdex::exchange(rhs._M_buf_allocated, {}),
		this->_M_ext_buf = ystdex::exchange(rhs._M_ext_buf, {}),
		this->_M_ext_buf_size = ystdex::exchange(rhs._M_ext_buf_size, 0),
		this->_M_ext_next = ystdex::exchange(rhs._M_ext_next, {}),
		this->_M_ext_end = ystdex::exchange(rhs._M_ext_end, {}),
		this->_M_reading = ystdex::exchange(rhs._M_reading, {}),
		this->_M_writing = ystdex::exchange(rhs._M_writing, {}),
		this->_M_pback_cur_save = ystdex::exchange(rhs._M_pback_cur_save, {}),
		this->_M_pback_end_save = ystdex::exchange(rhs._M_pback_end_save, {}),
		this->_M_pback_init = ystdex::exchange(rhs._M_pback_init, {})
		);
		rhs._M_set_buffer(-1);
		yunseq(rhs._M_state_last = rhs._M_state_beg,
			rhs._M_state_cur = rhs._M_state_beg);
	}
	//@}
#	endif

public:
	//! \since build 627
	std::basic_filebuf<_tChar, _tTraits>*
	open(UniqueFile p, std::ios_base::openmode mode)
	{
		if(p)
		{
			this->_M_file.sys_open(*p.get(), mode);

			if(open_check(mode))
			{
				p.release();
				return this;
			}
		}
		return {};
	}
	template<typename _tPathChar>
	std::basic_filebuf<_tChar, _tTraits>*
	open(const _tPathChar* s, std::ios_base::openmode mode)
	{
		if(!this->is_open())
		{
			this->_M_file.sys_open(uopen(s, omode_convb(mode), DefaultPMode()),
				mode);
			if(open_check(mode))
				return this;
		}
		return {};
	}
	//! \since build 618
	template<class _tString,
		yimpl(typename = ystdex::enable_for_string_class_t<_tString>)>
	std::basic_filebuf<_tChar, _tTraits>*
	open(const _tString& s, std::ios_base::openmode mode)
	{
		return open(s.c_str(), mode);
	}

private:
	//! \since build 627
	bool
	open_check(std::ios_base::openmode mode)
	{
		if(this->is_open())
		{
			this->_M_allocate_internal_buffer();
			this->_M_mode = mode;
			yunseq(this->_M_reading = {}, this->_M_writing = {});
			this->_M_set_buffer(-1);
			yunseq(this->_M_state_cur = this->_M_state_beg,
				this->_M_state_last = this->_M_state_beg);
			if((mode & std::ios_base::ate) && this->seekoff(0,
				std::ios_base::end) == pos_type(off_type(-1)))
				this->close();
			else
				return true;
		}
		return {};
	}
};


//extern template class YF_API basic_filebuf<char>;
//extern template class YF_API basic_filebuf<wchar_t>;

using filebuf = basic_filebuf<char>;
using wfilebuf = basic_filebuf<wchar_t>;


//! \since build 619
//@{
template<typename _tChar, class _tTraits = std::char_traits<_tChar>>
class basic_ifstream : public std::basic_istream<_tChar, _tTraits>
{
public:
	using char_type = _tChar;
	using int_type = typename _tTraits::int_type;
	using pos_type = typename _tTraits::pos_type;
	using off_type = typename _tTraits::off_type;
	using traits_type = _tTraits;

private:
	using base_type = std::basic_istream<char_type, traits_type>;

	mutable basic_filebuf<_tChar, _tTraits> fbuf{};

public:
	basic_ifstream()
		: base_type({})
	{
		this->init(&fbuf);
	}
	//! \since build 627
	template<typename _tParam,
		yimpl(typename = ystdex::exclude_self_ctor_t<basic_ifstream, _tParam>)>
	explicit
	basic_ifstream(_tParam&& s,
		std::ios_base::openmode mode = std::ios_base::in)
		: base_type({})
	{
		this->init(&fbuf);
		this->open(yforward(s), mode);
	}
	DefDelCopyCtor(basic_ifstream)
	basic_ifstream(basic_ifstream&& rhs)
		: base_type(std::move(rhs)),
		fbuf(std::move(rhs.fbuf))
	{
		base_type::set_rdbuf(&fbuf);
	}
	DefDeDtor(basic_ifstream)

	DefDelCopyAssignment(basic_ifstream)
	basic_ifstream&
	operator=(basic_ifstream&& rhs)
	{
		base_type::operator=(std::move(rhs));
		fbuf = std::move(rhs.fbuf);
		return *this;
	}

#	if YB_IMPL_GNUCPP && YB_IMPL_GNUCPP >= 50000 && __GLIBCXX__ > 20140922
	void
	swap(basic_ifstream& rhs)
	{
		base_type::swap(rhs),
		fbuf.swap(rhs.fbuf);
	}
#	endif

	void
	close()
	{
		if(!fbuf.close())
			this->setstate(std::ios_base::failbit);
	}

	bool
	is_open() const
	{
		return fbuf.is_open();
	}

	//! \since build 627
	template<typename _tParam>
	void
	open(_tParam&& s,
		std::ios_base::openmode mode = std::ios_base::in)
	{
		if(fbuf.open(yforward(s), mode))
			this->clear();
		else
			this->setstate(std::ios_base::failbit);
	}

	std::basic_filebuf<_tChar, _tTraits>*
	rdbuf() const
	{
		return &fbuf;
	}
};

#	if __GLIBCXX__ > 20140922
template<typename _tChar, class _tTraits>
inline DefSwap(, basic_ifstream<_tChar YPP_Comma _tTraits>)
#	endif


template<typename _tChar, class _tTraits = std::char_traits<_tChar>>
class basic_ofstream : public std::basic_ostream<_tChar, _tTraits>
{
public:
	using char_type = _tChar;
	using int_type = typename _tTraits::int_type;
	using pos_type = typename _tTraits::pos_type;
	using off_type = typename _tTraits::off_type;
	using traits_type = _tTraits;

private:
	using base_type = std::basic_ostream<char_type, traits_type>;

	//! \since build 617
	mutable basic_filebuf<_tChar, _tTraits> fbuf{};

public:
	basic_ofstream()
		: base_type({})
	{
		this->init(&fbuf);
	}
	//! \since build 627
	template<typename _tParam,
		yimpl(typename = ystdex::exclude_self_ctor_t<basic_ofstream, _tParam>)>
	explicit
	basic_ofstream(_tParam&& s,
		std::ios_base::openmode mode = std::ios_base::out)
		: base_type({})
	{
		this->init(&fbuf);
		this->open(yforward(s), mode);
	}
	DefDelCopyCtor(basic_ofstream)
	basic_ofstream(basic_ofstream&& rhs)
		: base_type(std::move(rhs)),
		fbuf(std::move(rhs.fbuf))
	{
		base_type::set_rdbuf(&fbuf);
	}
	DefDeDtor(basic_ofstream)

	DefDelCopyAssignment(basic_ofstream)
	basic_ofstream&
	operator=(basic_ofstream&& rhs)
	{
		base_type::operator=(std::move(rhs));
		fbuf = std::move(rhs.fbuf);
		return *this;
	}

#	if YB_IMPL_GNUCPP && YB_IMPL_GNUCPP >= 50000 && __GLIBCXX__ > 20140922
	void
	swap(basic_ofstream& rhs)
	{
		base_type::swap(rhs),
		fbuf.swap(rhs.fbuf);
	}
#	endif

	void
	close()
	{
		if(!fbuf.close())
			this->setstate(std::ios_base::failbit);
	}

	bool
	is_open() const
	{
		return fbuf.is_open();
	}

	//! \since build 627
	template<typename _tParam>
	void
	open(_tParam&& s, std::ios_base::openmode mode = std::ios_base::out)
	{
		if(fbuf.open(yforward(s), mode))
			this->clear();
		else
			this->setstate(std::ios_base::failbit);
	}

	std::basic_filebuf<_tChar, _tTraits>*
	rdbuf() const
	{
		return &fbuf;
	}
};

#	if __GLIBCXX__ > 20140922
template<typename _tChar, class _tTraits>
inline DefSwap(, basic_ofstream<_tChar YPP_Comma _tTraits>)
#	endif
//@}


template<typename _tChar, class _tTraits = std::char_traits<_tChar>>
class basic_fstream : public std::basic_iostream<_tChar, _tTraits>
{
public:
	using char_type = _tChar;
	using int_type = typename _tTraits::int_type;
	using pos_type = typename _tTraits::pos_type;
	using off_type = typename _tTraits::off_type;
	using traits_type = _tTraits;

private:
	using base_type = std::basic_iostream<char_type, traits_type>;

	//! \since build 617
	mutable basic_filebuf<_tChar, _tTraits> fbuf{};

public:
	basic_fstream()
		: base_type({})
	{
		this->init(&fbuf);
	}
	template<typename _tParam,
		yimpl(typename = ystdex::exclude_self_ctor_t<basic_fstream, _tParam>)>
	explicit
	basic_fstream(_tParam&& s,
		std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
		: base_type({})
	{
		this->init(&fbuf);
		this->open(yforward(s), mode);
	}
	DefDelCopyCtor(basic_fstream)
	basic_fstream(basic_fstream&& rhs)
		: base_type(std::move(rhs)),
		fbuf(std::move(rhs.fbuf))
	{
		base_type::set_rdbuf(&fbuf);
	}
	DefDeDtor(basic_fstream)

	DefDelCopyAssignment(basic_fstream)
	basic_fstream&
	operator=(basic_fstream&& rhs)
	{
		base_type::operator=(std::move(rhs));
		fbuf = std::move(rhs.fbuf);
		return *this;
	}

#	if YB_IMPL_GNUCPP && YB_IMPL_GNUCPP >= 50000 && __GLIBCXX__ > 20140922
	void
	swap(basic_fstream& rhs)
	{
		base_type::swap(rhs),
		fbuf.swap(rhs.fbuf);
	}
#	endif

	void
	close()
	{
		if(!fbuf.close())
			this->setstate(std::ios_base::failbit);
	}

	bool
	is_open() const
	{
		return fbuf.is_open();
	}

	//! \since build 617
	template<typename _tParam>
	void
	open(_tParam&& s,
		std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
	{
		if(fbuf.open(yforward(s), mode))
			this->clear();
		else
			this->setstate(std::ios_base::failbit);
	}

	std::basic_filebuf<_tChar, _tTraits>*
	rdbuf() const
	{
		return &fbuf;
	}
};

#	if __GLIBCXX__ > 20140922
template<typename _tChar, class _tTraits>
inline DefSwap(, basic_fstream<_tChar YPP_Comma _tTraits>)
#	endif


//extern template class YF_API basic_fstream<char>;
//extern template class YF_API basic_fstream<wchar_t>;

//! \since build 619
//@{
using ifstream = basic_ifstream<char>;
using ofstream = basic_ofstream<char>;
//! \since build 616
using fstream = basic_fstream<char>;
using wifstream = basic_ifstream<wchar_t>;
using wofstream = basic_ofstream<wchar_t>;
//@}
using wfstream = basic_fstream<wchar_t>;
#else
// TODO: Use VC++ extensions to support %char16_t path initialization.
using std::basic_filebuf;
using std::filebuf;
using std::wfilebuf;
//! \since build 619
//@{
using std::basic_ifstream;
using std::basic_ofstream;
//! \since build 616
using std::basic_fstream;
using std::ifstream;
using std::ofstream;
//! \since build 616
using std::fstream;
using std::wifstream;
using std::wofstream;
//@}
using std::wfstream;
#endif
//@}


/*!
\brief 表示文件操作失败的异常。
\since build 411
*/
class YF_API FileOperationFailure : public std::system_error
{
public:
	//! \since build 538
	using system_error::system_error;

	//! \since build 586
	DefDeCopyCtor(FileOperationFailure)
	/*!
	\brief 虚析构：类定义外默认实现。
	\since build 586
	*/
	~FileOperationFailure() override;
};


/*!
\build 抛出由 errno 和参数指定的 FileOperationFailure 对象。
\throw FileOperationFailure errno 和指定参数构造的异常。
\relates FileOperationFaiure
\since build 654
*/
template<typename _tParam>
YB_NORETURN void
ThrowFileOperationFailure(_tParam&& arg)
{
	ystdex::throw_error<FileOperationFailure>(errno, yforward(arg));
}


//! \exception FileOperationFailure 参数无效或文件时间查询失败。
//@{
/*!
\sa FileDescriptor::GetAccessTime
\since build 631
*/
//@{
inline YB_NONNULL(1) PDefH(FileTime, GetFileAccessTimeOf, std::FILE* fp)
	ImplRet(FileDescriptor(fp).GetAccessTime())
/*!
\pre 断言：第一参数非空。
\note 最后一个参数指定若文件系统支持，访问链接的文件而不是链接自身。
*/
//@{
YF_API YB_NONNULL(1) FileTime
GetFileAccessTimeOf(const char*, bool = {});
YF_API YB_NONNULL(1) FileTime
GetFileAccessTimeOf(const char16_t*, bool = {});
//@}
//@}

//@{
/*!
\sa FileDescriptor::GetModificationTime
\since build 628
*/
//@{
inline YB_NONNULL(1) PDefH(FileTime, GetFileModificationTimeOf, std::FILE* fp)
	ImplRet(FileDescriptor(fp).GetModificationTime())
/*!
\pre 断言：第一参数非空。
\note 最后一个参数指定若文件系统支持，访问链接的文件而不是链接自身。
*/

//@{
YF_API YB_NONNULL(1) FileTime
GetFileModificationTimeOf(const char*, bool = {});
YF_API YB_NONNULL(1) FileTime
GetFileModificationTimeOf(const char16_t*, bool = {});
//@}
//@}

//@{
/*!
\sa FileDescriptor::GetModificationAndAccessTime
\since build 631
*/
//@{
inline YB_NONNULL(1) PDefH(array<FileTime YPP_Comma 2>,
	GetFileModificationAndAccessTimeOf, std::FILE* fp)
	ImplRet(FileDescriptor(fp).GetModificationAndAccessTime())
/*!
\pre 断言：第一参数非空。
\note 最后一个参数指定若文件系统支持，访问链接的文件而不是链接自身。
*/
//@{
YF_API YB_NONNULL(1) array<FileTime, 2>
GetFileModificationAndAccessTimeOf(const char*, bool = {});
YF_API YB_NONNULL(1) array<FileTime, 2>
GetFileModificationAndAccessTimeOf(const char16_t*, bool = {});
//@}
//@}
//@}

/*!
\brief 取路径指定的文件链接数。
\return 若成功为连接数，否则为 0 。
\note 最后参数表示跟踪连接。
\since build 660
*/
YB_NONNULL(1) size_t
FetchNumberOfLinks(const char*, bool = true) ynothrow;
YB_NONNULL(1) size_t
FetchNumberOfLinks(const char16_t*, bool = true) ynothrow;


/*!
\brief 尝试删除指定路径的文件后再以指定路径和模式创建常规文件。
\pre 间接断言：第一参数非空。
\note 参数依次表示目标路径、打开模式、覆盖目标允许的链接数和是否允许共享覆盖。
\note 第二参数被限制 Mode::User 内。
\note 若目标已存在，创建或覆盖文件保证目标文件链接数不超过第三参数指定的值。
\note 若目标已存在、链接数大于 1 且不允许写入共享则先删除目标。
\note 忽略目标不存在导致的删除失败。
\throw FileOperationFailure 创建目标失败。
\since build 639
*/
YF_API YB_NONNULL(1) UniqueFile
EnsureUniqueFile(const char*, mode_t = DefaultPMode(), size_t = 1, bool = {});
//@}

/*!
\brief 比较文件内容相等。
\throw FileOperationFailure 文件按流打开失败。
\warning 读取失败时即截断返回，因此需要另行比较文件大小。
\sa IsNodeShared
\since build 658

首先比较文件节点，若为相同文件直接相等，否则清除 errno ，打开文件读取内容并比较。
*/
//@{
//! \note 间接断言：参数非空。
YF_API YB_NONNULL(1, 2) bool
HaveSameContents(const char*, const char*, mode_t = DefaultPMode());
/*!
\note 间接断言：文件指针非空。
\note 使用表示文件名称的字符串，仅用于在异常消息中显示（若为空则省略）。
\note 参数表示的文件已以可读形式打开，否则按流打开失败。
\note 视文件起始于当前读位置。
\since build 658
*/
YF_API YB_NONNULL(3, 4) bool
HaveSameContents(UniqueFile, UniqueFile, const char*, const char*);
//@}

/*!
\brief 判断参数是否表示共享的文件节点。
\note 可能设置 errno 。
\nsince build 638
*/
//@{
yconstfn YB_PURE PDefH(bool, IsNodeShared, const FileNodeID& x,
	const FileNodeID& y) ynothrow
	ImplRet(x != FileNodeID() && x == y)
/*!
\pre 间接断言：字符串参数非空。
\note 最后参数表示跟踪连接。
\since build 660
*/
//@{
YF_API YB_NONNULL(1, 2) bool
IsNodeShared(const char*, const char*, bool = true) ynothrowv;
YF_API YB_NONNULL(1, 2) bool
IsNodeShared(const char16_t*, const char16_t*, bool = true) ynothrowv;
//@}
/*!
\note 取节点失败视为不共享。
\sa FileDescriptor::GetNodeID
*/
bool
IsNodeShared(FileDescriptor, FileDescriptor) ynothrow;
//@}

} // namespace platform;

#endif
