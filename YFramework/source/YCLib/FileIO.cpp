﻿/*
	© 2011-2017 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file FileIO.cpp
\ingroup YCLib
\brief 平台相关的文件访问和输入/输出接口。
\version r3110
\author FrankHB <frankhb1989@gmail.com>
\since build 615
\par 创建时间:
	2015-07-14 18:53:12 +0800
\par 修改时间:
	2017-08-09 08:57 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YCLib::FileIO
*/


#undef __STRICT_ANSI__ // for ::fileno, ::pclose, ::popen, ::_wfopen;
#include "YCLib/YModules.h"
#include YFM_YCLib_FileIO // for std::is_same, ystdex::underlying_type_t,
//	RetryOnInterrupted, std::errc::function_not_supported, YCL_CallF_CAPI,
//	std::is_integral, ystdex::invoke, Nonnull, ystdex::temporary_buffer;
#include YFM_YCLib_NativeAPI // for Mode, ::HANDLE, struct ::stat,
//	platform_ex::cstat, platform_ex::estat, ::futimens, OpenMode,
//	YCL_CallGlobal, ::close, ::fcntl, F_GETFL, ::setmode, ::fchmod, ::_chsize,
//	::ftruncate, ::fsync, ::_wgetcwd, ::getcwd, ::chdir, ::rmdir, ::unlink,
//	!defined(__STRICT_ANSI__) API, ::GetCurrentDirectoryW;
#include YFM_YCLib_FileSystem // for NodeCategory::*, CategorizeNode;
#include <ystdex/functional.hpp> // for ystdex::compose, ystdex::addrof;
#include <ystdex/streambuf.hpp> // for ystdex::streambuf_equal;
#if YCL_DS
#	include "CHRLib/YModules.h"
#	include YFM_CHRLib_CharacterProcessing // for CHRLib::MakeMBCS,
//	CHRLib::MakeUCS2LE;

#elif YCL_Win32
#	if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
// At least one headers of <stdlib.h>, <stdio.h>, <Windows.h>, <Windef.h>
//	(and probably more) should have been included to make the MinGW-W64 macro
//	available if it is really being used.
#		undef _fileno
#	endif
#	include YFM_Win32_YCLib_MinGW32 // for platform_ex::FileAttributes,
//	platform_ex::GetErrnoFromWin32, platform_ex::QueryFileLinks,
//	platform_ex::QueryFileNodeID, platform_ex::QueryFileTime,
//	platform_ex::ConvertTime, platform_ex::SetFileTime, platform_ex::UnlockFile,
//	platform_ex::LockFile, platform_ex::TryLockFile;
#	include YFM_Win32_YCLib_NLS // for platform_ex::UTF8ToWCS,
//	platform_ex::WCSToUTF8;

//! \since build 639
using platform_ex::FileAttributes;
using platform_ex::GetErrnoFromWin32;
//! \since build 639
using platform_ex::QueryFileLinks;
//! \since build 632
//@{
using platform_ex::QueryFileNodeID;
using platform_ex::QueryFileTime;
//! \since build 540
using platform_ex::UTF8ToWCS;
using platform_ex::ConvertTime;
//@}
//! \since build 706
using platform_ex::MakePathStringW;
#elif YCL_API_POSIXFileSystem
#	include "CHRLib/YModules.h"
#	include YFM_CHRLib_CharacterProcessing // for CHRLib::MakeMBCS,
//	CHRLib::MakeUCS2LE;
#	include <sys/file.h> // for ::flock, LOCK_*;

//! \since build 475
using namespace CHRLib;
#else
#	error "Unsupported platform found."
#endif

namespace platform
{

//! \since build 627
static_assert(std::is_same<mode_t, ystdex::underlying_type_t<Mode>>::value,
	"Mismatched mode types found.");

namespace
{

//! \since build 703
using platform_ex::estat;
//! \since build 719
using platform_ex::cstat;
#if YCL_Win32
//! \since build 704
using platform_ex::ToHandle;

//! \since build 660
void
QueryFileTime(const char* path, ::FILETIME* p_ctime, ::FILETIME* p_atime,
	::FILETIME* p_mtime, bool follow_reparse_point)
{
	QueryFileTime(MakePathStringW(path).c_str(), p_ctime, p_atime, p_mtime,
		follow_reparse_point);
}
//! \since build 632
//@{
void
QueryFileTime(int fd, ::FILETIME* p_ctime, ::FILETIME* p_atime,
	::FILETIME* p_mtime)
{
	QueryFileTime(ToHandle(fd), p_ctime, p_atime, p_mtime);
}

// TODO: Blocked. Use C++14 generic lambda expressions.
yconstexpr const struct
{
	template<typename _tParam, typename... _tParams>
	FileTime
	operator()(_tParam arg, _tParams... args) const
	{
		::FILETIME atime{};

		QueryFileTime(arg, {}, &atime, {}, args...);
		return ConvertTime(atime);
	}
} get_st_atime{};
yconstexpr const struct
{
	template<typename _tParam, typename... _tParams>
	FileTime
	operator()(_tParam arg, _tParams... args) const
	{
		::FILETIME mtime{};

		QueryFileTime(arg, {}, {}, &mtime, args...);
		return ConvertTime(mtime);
	}
} get_st_mtime{};
yconstexpr const struct
{
	template<typename _tParam, typename... _tParams>
	array<FileTime, 2>
	operator()(_tParam arg, _tParams... args) const
	{
		::FILETIME mtime{}, atime{};

		QueryFileTime(arg, {}, &atime, &mtime, args...);
		return array<FileTime, 2>{ConvertTime(mtime), ConvertTime(atime)};
	}
} get_st_matime{};
//@}

//! \since build 639
//@{
YB_NONNULL(1) bool
UnlinkWithAttr(const wchar_t* path, FileAttributes attr) ynothrow
{
	return !(attr & FileAttributes::ReadOnly) || ::SetFileAttributesW(path,
		attr & ~FileAttributes::ReadOnly) ? ::_wunlink(path) == 0
		: (errno = GetErrnoFromWin32(), false);
}

template<typename _func>
YB_NONNULL(2) bool
CallFuncWithAttr(_func f, const char* path)
{
	const auto& wpath(MakePathStringW(path));
	const auto& wstr(wpath.c_str());
	const auto attr(FileAttributes(::GetFileAttributesW(wstr)));

	return attr != platform_ex::Invalid ? f(wstr, attr)
		: (errno = GetErrnoFromWin32(), false);
}
//@}
#else
inline PDefH(::timespec, ToTimeSpec, FileTime ft) ynothrow
	ImplRet({std::time_t(ft.count() / 1000000000LL),
		long(ft.count() % 1000000000LL)})

//! \since build 719
void
SetFileTime(int fd, const ::timespec(&times)[2])
{
#if YCL_DS
	// XXX: Hack.
#ifndef UTIME_OMIT
#	define UTIME_OMIT (-1L)
#endif
	yunused(fd), yunused(times);
	ystdex::throw_error(std::errc::function_not_supported, yfsig);
#else
	YCL_CallF_CAPI(, ::futimens, fd, times);
#endif
}

//! \since build 631
//@{
const auto get_st_atime([](struct ::stat& st){
	return FileTime(st.st_atime);
});
const auto get_st_mtime([](struct ::stat& st){
	return FileTime(st.st_mtime);
});
const auto get_st_matime([](struct ::stat& st){
	return array<FileTime, 2>{FileTime(st.st_mtime), FileTime(st.st_atime)};
});
//@}

//! \since build 638
//@{
static_assert(std::is_integral<::dev_t>(),
	"Nonconforming '::dev_t' type found.");
static_assert(std::is_unsigned<::ino_t>(),
	"Nonconforming '::ino_t' type found.");

inline PDefH(FileNodeID, get_file_node_id, struct ::stat& st) ynothrow
	ImplRet({std::uint64_t(st.st_dev), std::uint64_t(st.st_ino)})
//@}
//! \since build 719
inline YB_NONNULL(2, 4) PDefH(void, cstat, struct ::stat& st,
	const char16_t* path, bool follow_link, const char* sig)
	ImplRet(cstat(st, MakePathString(path).c_str(), follow_link, sig))
//! \since build 632
inline YB_NONNULL(2) PDefH(int, estat, struct ::stat& st, const char16_t* path,
	bool follow_link)
	ImplRet(estat(st, MakePathString(path).c_str(), follow_link))
#endif

//! \since build 632
template<typename _func, typename... _tParams>
auto
FetchFileTime(_func f, _tParams... args)
#if YCL_Win32
	-> ystdex::result_of_t<_func(_tParams&...)>
#else
	-> ystdex::result_of_t<_func(struct ::stat&)>
#endif
{
#if YCL_Win32
	// NOTE: The %::FILETIME has resolution of 100 nanoseconds.
	// XXX: Error handling for indirect calls.
	return f(args...);
#else
	// TODO: Get more precise time count.
	struct ::stat st;

	cstat(st, args..., yfsig);
	return f(st);
#endif
}

//! \since build 765
template<int _vErr, typename _fCallable, class _tObj, typename _tByteBuf,
	typename... _tParams>
_tByteBuf
FullReadWrite(_fCallable f, _tObj&& obj, _tByteBuf ptr, size_t nbyte,
	_tParams&&... args)
{
	while(nbyte > 0)
	{
		const auto n(ystdex::invoke(f, obj, ptr, nbyte, yforward(args)...));

		if(n == size_t(-1))
			break;
		if(n == 0)
		{
			errno = _vErr;
			break;
		}
		yunseq(ptr += n, nbyte -= n);
	}
	return ptr;
}

//! \since build 709
//@{
#if YCL_Win64
using rwsize_t = unsigned;
#else
using rwsize_t = size_t;
#endif

template<typename _func, typename _tBuf>
size_t
SafeReadWrite(_func f, int fd, _tBuf buf, rwsize_t nbyte) ynothrowv
{
	YAssert(fd != -1, "Invalid file descriptor found.");
	return size_t(RetryOnInterrupted([&]{
		return f(fd, Nonnull(buf), nbyte);
	}));
}
//@}

#if !YCL_DS
//! \since build 721
YB_NONNULL(2) void
UnlockFileDescriptor(int fd, const char* sig) ynothrowv
{
#	if YCL_Win32
	const auto res(platform_ex::UnlockFile(ToHandle(fd)));

	if(YB_UNLIKELY(!res))
		YCL_Trace_Win32E(Descriptions::Warning, UnlockFileEx, sig);
	YAssert(res, "Narrow contract violated.");
#	else
	const auto res(YCL_TraceCall_CAPI(::flock, sig, fd, LOCK_UN));

	YAssert(res == 0, "Narrow contract violated.");
	yunused(res);
#	endif
}
#endif

//! \since build 660
bool
IsNodeShared_Impl(const char* a, const char* b, bool follow_link) ynothrow
{
#if YCL_Win32
	return CallNothrow({}, [=]{
		return QueryFileNodeID(MakePathStringW(a).c_str(), follow_link)
			== QueryFileNodeID(MakePathStringW(b).c_str(), follow_link);
	});
#else
	struct ::stat st;

	if(estat(st, a, follow_link) != 0)
		return {};

	const auto id(get_file_node_id(st));

	if(estat(st, b, follow_link) != 0)
		return {};

	return IsNodeShared(id, get_file_node_id(st));
#endif
}
//! \since build 660
bool
IsNodeShared_Impl(const char16_t* a, const char16_t* b, bool follow_link)
	ynothrow
{
#if YCL_Win32
	return CallNothrow({}, [=]{
		return QueryFileNodeID(wcast(a), follow_link)
			== QueryFileNodeID(wcast(b), follow_link);
	});
#else
	return IsNodeShared_Impl(MakePathString(a).c_str(),
		MakePathString(b).c_str(), follow_link);
#endif
}

//! \since build 701
template<typename _tChar>
YB_NONNULL(1, 2) pair<UniqueFile, UniqueFile>
HaveSameContents_Impl(const _tChar* path_a, const _tChar* path_b, mode_t mode)
{
	if(UniqueFile p_a{uopen(path_a, int(OpenMode::ReadRaw), mode)})
		if(UniqueFile p_b{uopen(path_b, int(OpenMode::ReadRaw), mode)})
			return {std::move(p_a), std::move(p_b)};
	return {};
}

} // unnamed namespace;


string
MakePathString(const char16_t* s)
{
#if YCL_Win32
	return platform_ex::WCSToUTF8(wcast(s));
#else
	return CHRLib::MakeMBCS(s);
#endif
}


NodeCategory
CategorizeNode(mode_t st_mode) ynothrow
{
	auto res(NodeCategory::Empty);
	const auto m(Mode(st_mode) & Mode::FileType);

	if((m & Mode::Directory) == Mode::Directory)
		res |= NodeCategory::Directory;
#if !YCL_Win32
	if((m & Mode::Link) == Mode::Link)
		res |= NodeCategory::Link;
#endif
	if((m & Mode::Regular) == Mode::Regular)
		res |= NodeCategory::Regular;
	if(YB_UNLIKELY((m & Mode::Character) == Mode::Character))
		res |= NodeCategory::Character;
#if !YCL_Win32
	else if(YB_UNLIKELY((m & Mode::Block) == Mode::Block))
		res |= NodeCategory::Block;
#endif
	if(YB_UNLIKELY((m & Mode::FIFO) == Mode::FIFO))
		res |= NodeCategory::FIFO;
#if !YCL_Win32
	if((m & Mode::Socket) == Mode::Socket)
		res |= NodeCategory::Socket;
#endif
	return res;
}


void
FileDescriptor::Deleter::operator()(pointer p) const ynothrow
{
	if(p && YB_UNLIKELY(YCL_CallGlobal(close, *p) < 0))
		// NOTE: This is not a necessarily error. See $2016-04
		//	@ %Documentation::Workflow::Annual2016.
		YTraceDe(Descriptions::Warning,
			"Falied closing file descriptor, errno = %d.", errno);
}


FileDescriptor::FileDescriptor(std::FILE* fp) ynothrow
	: desc(fp ? YCL_CallGlobal(fileno, fp) : -1)
{}

FileTime
FileDescriptor::GetAccessTime() const
{
	return FetchFileTime(get_st_atime, desc);
}
NodeCategory
FileDescriptor::GetCategory() const ynothrow
{
#if YCL_Win32
	return platform_ex::CategorizeNode(ToHandle(desc));
#else
	struct ::stat st;

	return estat(st, desc) == 0 ? CategorizeNode(st.st_mode)
		: NodeCategory::Invalid;
#endif
}
int
FileDescriptor::GetFlags() const
{
#if YCL_API_POSIXFileSystem
	return YCL_CallF_CAPI(, ::fcntl, desc, F_GETFL);
#else
	ystdex::throw_error(std::errc::function_not_supported, yfsig);
#endif
}
mode_t
FileDescriptor::GetMode() const
{
	struct ::stat st;

	cstat(st, desc, yfsig);
	return st.st_mode;
}
FileTime
FileDescriptor::GetModificationTime() const
{
	return FetchFileTime(get_st_mtime, desc);
}
array<FileTime, 2>
FileDescriptor::GetModificationAndAccessTime() const
{
	return FetchFileTime(get_st_matime, desc);
}
FileNodeID
FileDescriptor::GetNodeID() const ynothrow
{
#if YCL_Win32
	return CallNothrow({}, [=]{
		return QueryFileNodeID(ToHandle(desc));
	});
#else
	struct ::stat st;

	return estat(st, desc) == 0 ? get_file_node_id(st) : FileNodeID();
#endif
}
size_t
FileDescriptor::GetNumberOfLinks() const ynothrow
{
#if YCL_Win32
	return CallNothrow({}, [=]{
		return QueryFileLinks(ToHandle(desc));
	});
#else
	struct ::stat st;
	static_assert(std::is_unsigned<decltype(st.st_nlink)>(),
		"Unsupported 'st_nlink' type found.");

	return estat(st, desc) == 0 ? size_t(st.st_nlink) : size_t();
#endif
}
std::uint64_t
FileDescriptor::GetSize() const
{
#if YCL_Win32
	return platform_ex::QueryFileSize(ToHandle(desc));
#else
	struct ::stat st;

	YCL_CallF_CAPI(, ::fstat, desc, &st);

	// XXX: No negative file size should be found. See also:
	//	http://stackoverflow.com/questions/12275831/why-is-the-st-size-field-in-struct-stat-signed.
	if(st.st_size >= 0)
		return std::uint64_t(st.st_size);
	throw std::invalid_argument("Negative file size found.");
#endif
}

void
FileDescriptor::SetAccessTime(FileTime ft) const
{
#if YCL_Win32
	auto atime(ConvertTime(ft));

	platform_ex::SetFileTime(ToHandle(desc), {}, &atime, {});
#else
	const ::timespec times[]{ToTimeSpec(ft), {yimpl(0), UTIME_OMIT}};

	SetFileTime(desc, times);
#endif
}
bool
FileDescriptor::SetBlocking() const
{
#if YCL_API_POSIXFileSystem
	// NOTE: Read-modify-write operation is needed for compatibility.
	//	See http://pubs.opengroup.org/onlinepubs/9699919799/functions/fcntl.html.
	const auto oflags((OpenMode(GetFlags())));

	return bool(oflags & OpenMode::Nonblocking)
		? (SetFlags(int(oflags & ~OpenMode::Nonblocking)), true) : false;
#else
	ystdex::throw_error(std::errc::function_not_supported, yfsig);
#endif
}
void
FileDescriptor::SetFlags(int flags) const
{
#if YCL_API_POSIXFileSystem
	YCL_CallF_CAPI(, ::fcntl, desc, F_SETFL, flags);
#else
	// TODO: Try using NT6 %::SetFileInformationByHandle for Win32.
	yunused(flags);
	ystdex::throw_error(std::errc::function_not_supported, yfsig);
#endif
}
void
FileDescriptor::SetMode(mode_t mode) const
{
#if YCL_API_POSIXFileSystem
	YCL_CallF_CAPI(, ::fchmod, desc, mode);
#else
	yunused(mode);
	ystdex::throw_error(std::errc::function_not_supported, yfsig);
#endif
}
void
FileDescriptor::SetModificationTime(FileTime ft) const
{
#if YCL_Win32
	auto mtime(ConvertTime(ft));

	platform_ex::SetFileTime(ToHandle(desc), {}, {}, &mtime);
#else
	const ::timespec times[]{{yimpl(0), UTIME_OMIT}, ToTimeSpec(ft)};

	SetFileTime(desc, times);
#endif
}
void
FileDescriptor::SetModificationAndAccessTime(array<FileTime, 2> fts) const
{
#if YCL_Win32
	auto atime(ConvertTime(fts[0])), mtime(ConvertTime(fts[1]));

	platform_ex::SetFileTime(ToHandle(desc), {}, &atime, &mtime);
#else
	const ::timespec times[]{ToTimeSpec(fts[0]), ToTimeSpec(fts[1])};

	SetFileTime(desc, times);
#endif
}
bool
FileDescriptor::SetNonblocking() const
{
#if YCL_API_POSIXFileSystem
	// NOTE: Read-modify-write operation is need for compatibility.
	//	See http://pubs.opengroup.org/onlinepubs/9699919799/functions/fcntl.html.
	const auto oflags((OpenMode(GetFlags())));

	return !bool(oflags & OpenMode::Nonblocking)
		? (SetFlags(int(oflags | OpenMode::Nonblocking)), true) : false;
#else
	ystdex::throw_error(std::errc::function_not_supported, yfsig);
#endif
}
bool
FileDescriptor::SetSize(size_t size) ynothrow
{
#if YCL_Win32
	return ::_chsize(desc, long(size)) == 0;
#else
	// FIXME: It seems that on Android 32-bit size is always used even if
	//	'_FILE_OFFSET_BITS == 1'. Thus it is not safe and would better to be
	//	ignored slightly.
	return ::ftruncate(desc, ::off_t(size)) == 0;
#endif
}
int
FileDescriptor::SetTranslationMode(int mode) const ynothrow
{
	// TODO: Other platforms.
#if YCL_Win32 || (defined(_NEWLIB_VERSION) && defined(__SCLE))
	return YCL_CallGlobal(setmode, desc, mode);
#else
	// NOTE: No effect.
	yunused(mode);
	return 0;
#endif
}

void
FileDescriptor::Flush()
{
#if YCL_Win32
	YCL_CallF_CAPI(, ::_commit, desc);
#else
	// XXX: http://austingroupbugs.net/view.php?id=672.
	YCL_CallF_CAPI(, ::fsync, desc);
#endif
}

size_t
FileDescriptor::FullRead(void* buf, size_t nbyte) ynothrowv
{
	using namespace std::placeholders;
	const auto p_buf(static_cast<char*>(buf));

	return size_t(FullReadWrite<0>(&FileDescriptor::Read, this, p_buf, nbyte)
		- p_buf);
}

size_t
FileDescriptor::FullWrite(const void* buf, size_t nbyte) ynothrowv
{
	using namespace std::placeholders;
	const auto p_buf(static_cast<const char*>(buf));

	return size_t(FullReadWrite<ENOSPC>(&FileDescriptor::Write, this, p_buf,
		nbyte) - p_buf);
}

size_t
FileDescriptor::Read(void* buf, size_t nbyte) ynothrowv
{
	return SafeReadWrite(YCL_ReservedGlobal(read), desc, buf, rwsize_t(nbyte));
}

size_t
FileDescriptor::Write(const void* buf, size_t nbyte) ynothrowv
{
	return SafeReadWrite(YCL_ReservedGlobal(write), desc, buf, rwsize_t(nbyte));
}

void
FileDescriptor::WriteContent(FileDescriptor ofd, FileDescriptor ifd,
	byte* buf, size_t size)
{
	YAssertNonnull(ifd),
	YAssertNonnull(ofd),
	YAssertNonnull(buf),
	YAssert(size != 0, "Invalid size found.");

	ystdex::retry_on_cond([&](size_t len){
		if(len == size_t(-1))
			YCL_Raise_SysE(, "Failed reading source file '" + to_string(*ifd),
				yfsig);
		if(ofd.FullWrite(buf, len) == size_t(-1))
			YCL_Raise_SysE(,
				"Failed writing destination file '" + to_string(*ofd), yfsig);
		return len != 0;
	}, &FileDescriptor::Read, &ifd, buf, size);
}
void
FileDescriptor::WriteContent(FileDescriptor ofd, FileDescriptor ifd,
	size_t size)
{
	ystdex::temporary_buffer<byte> buf(size);

	WriteContent(ofd, ifd, buf.get().get(), buf.size());
}

void
FileDescriptor::lock()
{
#if YCL_DS
#elif YCL_Win32
	platform_ex::LockFile(ToHandle(desc));
#else
	YCL_CallF_CAPI(, ::flock, desc, LOCK_EX);
#endif
}

void
FileDescriptor::lock_shared()
{
#if YCL_DS
#elif YCL_Win32
	platform_ex::LockFile(ToHandle(desc), 0, std::uint64_t(-1), {});
#else
	YCL_CallF_CAPI(, ::flock, desc, LOCK_SH);
#endif
}

bool
FileDescriptor::try_lock() ynothrowv
{
#if YCL_DS
	return true;
#elif YCL_Win32
	return platform_ex::TryLockFile(ToHandle(desc), 0, std::uint64_t(-1));
#else
	return YCL_TraceCallF_CAPI(::flock, desc, LOCK_EX | LOCK_NB) != -1;
#endif
}

bool
FileDescriptor::try_lock_shared() ynothrowv
{
#if YCL_DS
	return true;
#elif YCL_Win32
	return platform_ex::TryLockFile(ToHandle(desc), 0, std::uint64_t(-1), {});
#else
	return YCL_TraceCallF_CAPI(::flock, desc, LOCK_SH | LOCK_NB) != -1;
#endif
}

void
FileDescriptor::unlock() ynothrowv
{
#if YCL_DS
#else
	UnlockFileDescriptor(desc, yfsig);
#endif
}

void
FileDescriptor::unlock_shared() ynothrowv
{
#if YCL_DS
#else
	UnlockFileDescriptor(desc, yfsig);
#endif
}


mode_t
DefaultPMode() ynothrow
{
#if YCL_Win32
	// XXX: For compatibility with newer version of MSVCRT, no %_umask call
	//	would be considered. See https://msdn.microsoft.com/en-us/library/z0kc8e3z.aspx.
	return mode_t(Mode::UserReadWrite);
#else
	return mode_t(Mode::UserReadWrite | Mode::GroupReadWrite | Mode::OtherRead);
#endif
}

void
SetBinaryIO(std::FILE* stream) ynothrowv
{
#if YCL_Win32
	FileDescriptor(Nonnull(stream)).SetTranslationMode(int(OpenMode::Binary));
#else
	// NOTE: No effect.
	Nonnull(stream);
#endif
}


int
omode_conv(std::ios_base::openmode mode) ynothrow
{
	using namespace std;
	auto res(OpenMode::None);

	switch(unsigned((mode &= ~ios_base::ate)
		& ~(ios_base::binary | ios_nocreate | ios_noreplace)))
	{
	case ios_base::out:
		res = OpenMode::Create | OpenMode::Write;
		break;
	case ios_base::out | ios_base::trunc:
		res = OpenMode::Create | OpenMode::WriteTruncate;
		break;
	case ios_base::out | ios_base::app:
	case ios_base::app:
		res = OpenMode::Create | OpenMode::WriteAppend;
		break;
	case ios_base::in:
		res = OpenMode::Read;
		break;
	case ios_base::in | ios_base::out:
		res = OpenMode::ReadWrite;
		break;
	case ios_base::in | ios_base::out | ios_base::trunc:
		res = OpenMode::Create | OpenMode::ReadWriteTruncate;
		break;
	case ios_base::in | ios_base::out | ios_base::app:
	case ios_base::in | ios_base::app:
		res = OpenMode::Create | OpenMode::ReadWriteAppend;
		break;
	default:
		return int(res);
	}
	// XXX: Order is significant.
	if(mode & ios_noreplace)
		res |= OpenMode::CreateExclusive;
	// NOTE: %O_EXCL without %O_CREAT leads to undefined behavior in POSIX.
	if(mode & ios_nocreate)
		res &= ~OpenMode::CreateExclusive;
	return int(res);
}

int
omode_convb(std::ios_base::openmode mode) ynothrow
{
	return omode_conv(mode)
		| int(mode & std::ios_base::binary ? OpenMode::Binary : OpenMode::Text);
}


int
uaccess(const char* path, int amode) ynothrowv
{
	YAssertNonnull(path);
#if YCL_Win32
	return CallNothrow(-1, [=]{
		return ::_waccess(MakePathStringW(path).c_str(), amode);
	});
#else
	return ::access(path, amode);
#endif
}
int
uaccess(const char16_t* path, int amode) ynothrowv
{
	YAssertNonnull(path);
#if YCL_Win32
	return ::_waccess(wcast(path), amode);
#else
	return CallNothrow(-1, [=]{
		return ::access(MakePathString(path).c_str(), amode);
	});
#endif
}

int
uopen(const char* filename, int oflag, mode_t pmode) ynothrowv
{
	YAssertNonnull(filename);
#if YCL_Win32
	return CallNothrow(-1, [=]{
		return ::_wopen(MakePathStringW(filename).c_str(), oflag, int(pmode));
	});
#else
	return ::open(filename, oflag, pmode);
#endif
}
int
uopen(const char16_t* filename, int oflag, mode_t pmode) ynothrowv
{
	YAssertNonnull(filename);
#if YCL_Win32
	return ::_wopen(wcast(filename), oflag, int(pmode));
#else
	return CallNothrow(-1, [=]{
		return ::open(MakePathString(filename).c_str(), oflag, pmode);
	});
#endif
}

std::FILE*
ufopen(const char* filename, const char* mode) ynothrowv
{
	YAssertNonnull(filename);
	YAssert(Deref(mode) != char(), "Invalid argument found.");
#if YCL_Win32
	return CallNothrow({}, [=]{
		return ::_wfopen(MakePathStringW(filename).c_str(),
			MakePathStringW(mode).c_str());
	});
#else
	return std::fopen(filename, mode);
#endif
}
std::FILE*
ufopen(const char16_t* filename, const char16_t* mode) ynothrowv
{
	YAssertNonnull(filename);
	YAssert(Deref(mode) != char(), "Invalid argument found.");
#if YCL_Win32
	return ::_wfopen(wcast(filename), wcast(mode));
#else
	return CallNothrow({}, [=]{
		return std::fopen(MakePathString(filename).c_str(),
			MakePathString(mode).c_str());
	});
#endif
}
std::FILE*
ufopen(const char* filename, std::ios_base::openmode mode) ynothrowv
{
	if(const auto c_mode = ystdex::openmode_conv(mode))
		return ufopen(filename, c_mode);
	return {};
}
std::FILE*
ufopen(const char16_t* filename, std::ios_base::openmode mode) ynothrowv
{
	YAssertNonnull(filename);
	if(const auto c_mode = ystdex::openmode_conv(mode))
		return CallNothrow({}, [=]{
#if YCL_Win32
			return ::_wfopen(wcast(filename), MakePathStringW(c_mode).c_str());
#else
			return std::fopen(MakePathString(filename).c_str(), c_mode);
#endif
		});
	return {};
}

bool
ufexists(const char* filename) ynothrowv
{
#if YCL_Win32
	return ystdex::call_value_or(ystdex::compose(std::fclose,
		ystdex::addrof<>()), ufopen(filename, "rb"), yimpl(1)) == 0;
#else
	return ystdex::fexists(filename);
#endif
}
bool
ufexists(const char16_t* filename) ynothrowv
{
	return ystdex::call_value_or(ystdex::compose(std::fclose,
		ystdex::addrof<>()), ufopen(filename, u"rb"), yimpl(1)) == 0;
}

char*
ugetcwd(char* buf, size_t size) ynothrowv
{
	YAssertNonnull(buf);
	if(size != 0)
	{
		using namespace std;

#if YCL_Win32
		try
		{
			const auto p(make_unique<wchar_t[]>(size));

			if(const auto cwd = ::_wgetcwd(p.get(), int(size)))
			{
				const auto res(MakePathString(ucast(cwd)));
				const auto len(res.length());

				if(size < len + 1)
					errno = ERANGE;
				else
					return ystdex::ntctscpy(buf, res.data(), len);
			}
		}
		CatchExpr(std::bad_alloc&, errno = ENOMEM);
#else
		// NOTE: POSIX.1 2004 has no guarantee about slashes. POSIX.1 2013
		//	mandates there are no redundant slashes. See http://pubs.opengroup.org/onlinepubs/009695399/functions/getcwd.html.
		return ::getcwd(buf, size);
#endif
	}
	else
		errno = EINVAL;
	return {};
}
char16_t*
ugetcwd(char16_t* buf, size_t len) ynothrowv
{
	YAssertNonnull(buf);
	if(len != 0)
	{
		using namespace std;

#if YCL_Win32
		// NOTE: Win32 guarantees there will be a separator if and only if when
		//	the result is root directory for ::_wgetcwd, and actually it is
		//	the same in ::%GetCurrentDirectoryW.
		const auto n(::GetCurrentDirectoryW(len, wcast(buf)));

		if(n != 0)
			return buf;
		errno = GetErrnoFromWin32();
		return {};
#else
		// XXX: Alias by %char array is safe.
		if(const auto cwd
			= ::getcwd(ystdex::aligned_store_cast<char*>(buf), len))
			try
			{
				const auto res(platform_ex::MakePathStringU(cwd));
				const auto rlen(res.length());

				if(len < rlen + 1)
					errno = ERANGE;
				else
					return ystdex::ntctscpy(buf, res.data(), rlen);
			}
			CatchExpr(std::bad_alloc&, errno = ENOMEM)
#endif
	}
	else
		errno = EINVAL;
	return {};
}

#define YCL_Impl_FileSystem_ufunc_1(_n) \
bool \
_n(const char* path) ynothrowv \
{ \
	YAssertNonnull(path); \

#if YCL_Win32
#	define YCL_Impl_FileSystem_ufunc_2(_fn, _wfn) \
	return CallNothrow({}, [&]{ \
		return _wfn(MakePathStringW(path).c_str()) == 0; \
	}); \
}
#else
#	define YCL_Impl_FileSystem_ufunc_2(_fn, _wfn) \
	return _fn(path) == 0; \
}
#endif

#define YCL_Impl_FileSystem_ufunc(_n, _fn, _wfn) \
	YCL_Impl_FileSystem_ufunc_1(_n) \
	YCL_Impl_FileSystem_ufunc_2(_fn, _wfn)

YCL_Impl_FileSystem_ufunc(uchdir, ::chdir, ::_wchdir)

YCL_Impl_FileSystem_ufunc_1(umkdir)
#if YCL_Win32
	YCL_Impl_FileSystem_ufunc_2(_unused_, ::_wmkdir)
#else
	return ::mkdir(path, mode_t(Mode::Access)) == 0;
}
#endif

YCL_Impl_FileSystem_ufunc(urmdir, ::rmdir, ::_wrmdir)

YCL_Impl_FileSystem_ufunc_1(uunlink)
#if YCL_Win32
	return CallNothrow({}, [=]{
		return CallFuncWithAttr(UnlinkWithAttr, path);
	});
}
#else
YCL_Impl_FileSystem_ufunc_2(::unlink, )
#endif

YCL_Impl_FileSystem_ufunc_1(uremove)
#if YCL_Win32
	// NOTE: %::_wremove is same to %::_wunlink on Win32 which cannot delete
	//	empty directories.
	return CallNothrow({}, [=]{
		return CallFuncWithAttr([](const wchar_t* wstr, FileAttributes attr)
			YB_NONNULL(1) ynothrow{
			return attr & FileAttributes::Directory ? ::_wrmdir(wstr) == 0
				: UnlinkWithAttr(wstr, attr);
		}, path);
	});
}
#else
YCL_Impl_FileSystem_ufunc_2(std::remove, )
#endif

#undef YCL_Impl_FileSystem_ufunc_1
#undef YCL_Impl_FileSystem_ufunc_2
#undef YCL_Impl_FileSystem_ufunc


#if YCL_Win32
template<>
YF_API string
FetchCurrentWorkingDirectory(size_t init)
{
	return MakePathString(FetchCurrentWorkingDirectory<char16_t>(init));
}
template<>
YF_API u16string
FetchCurrentWorkingDirectory(size_t)
{
	u16string res;
	unsigned long len, rlen(0);

	// NOTE: Retry is necessary to prevent failure due to modification of
	//	current directory from other threads.
	ystdex::retry_on_cond([&]() -> bool{
		if(rlen < len)
		{
			res.pop_back();
			return {};
		}
		if(rlen != 0)
			return true;
		YCL_Raise_Win32E("GetCurrentDirectoryW", yfsig);
	}, [&]{
		if((len = ::GetCurrentDirectoryW(0, {})) != 0)
		{
			res.resize(size_t(len + 1));
			rlen = ::GetCurrentDirectoryW(len, wcast(&res[0]));
		}
	});
	res.resize(rlen);
	return res;
}
#endif


FileTime
GetFileAccessTimeOf(const char* filename, bool follow_link)
{
	return FetchFileTime(get_st_atime, filename, follow_link);
}
FileTime
GetFileAccessTimeOf(const char16_t* filename, bool follow_link)
{
	return FetchFileTime(get_st_atime, wcast(filename), follow_link);
}

FileTime
GetFileModificationTimeOf(const char* filename, bool follow_link)
{
	return FetchFileTime(get_st_mtime, filename, follow_link);
}
FileTime
GetFileModificationTimeOf(const char16_t* filename, bool follow_link)
{
	return FetchFileTime(get_st_mtime, wcast(filename), follow_link);
}

array<FileTime, 2>
GetFileModificationAndAccessTimeOf(const char* filename, bool follow_link)
{
	return FetchFileTime(get_st_matime, filename, follow_link);
}
array<FileTime, 2>
GetFileModificationAndAccessTimeOf(const char16_t* filename, bool follow_link)
{
	return FetchFileTime(get_st_matime, wcast(filename), follow_link);
}

YB_NONNULL(1) size_t
FetchNumberOfLinks(const char* path, bool follow_link)
{
#if YCL_Win32
	return QueryFileLinks(MakePathStringW(path).c_str(), follow_link);
#else
	struct ::stat st;
	static_assert(std::is_unsigned<decltype(st.st_nlink)>(),
		"Unsupported 'st_nlink' type found.");

	cstat(st, path, follow_link, yfsig);
	return st.st_nlink;
#endif
}
YB_NONNULL(1) size_t
FetchNumberOfLinks(const char16_t* path, bool follow_link)
{
#if YCL_Win32
	return QueryFileLinks(wcast(path), follow_link);
#else
	return FetchNumberOfLinks(MakePathString(path).c_str(), follow_link);
#endif
}


UniqueFile
EnsureUniqueFile(const char* dst, mode_t mode, size_t allowed_links,
	bool share)
{
	static yconstexpr const auto oflag(OpenMode::Create
		| OpenMode::WriteTruncate | OpenMode::Binary);

	mode &= mode_t(Mode::User);
	if(UniqueFile
		p_file{uopen(dst, int(oflag | OpenMode::Exclusive), mode)})
		return p_file;
	if(allowed_links != 0 && errno == EEXIST)
	{
		const auto n_links(FetchNumberOfLinks(dst, true));

		// FIXME: Blocked. TOCTTOU access.
		if(!(allowed_links < n_links)
			&& (n_links == 1 || share || uunlink(dst) || errno == ENOENT))
			if(UniqueFile p_file{uopen(dst, int(oflag), mode)})
				return p_file;
	}
	YCL_Raise_SysE(, "::open", yfsig);
}

bool
HaveSameContents(const char* path_a, const char* path_b, mode_t mode)
{
	auto pr(HaveSameContents_Impl(path_a, path_b, mode));

	return pr.first ? HaveSameContents(std::move(pr.first),
		std::move(pr.second), path_a, path_b) : false;
}
bool
HaveSameContents(const char16_t* path_a, const char16_t* path_b, mode_t mode)
{
	auto pr(HaveSameContents_Impl(path_a, path_b, mode));

	// TODO: Support names in exception messages.
	return pr.first ? HaveSameContents(std::move(pr.first),
		std::move(pr.second), {}, {}) : false;
}
bool
HaveSameContents(UniqueFile p_a, UniqueFile p_b, const char* name_a,
	const char* name_b)
{
	if(!(bool(Nonnull(p_a)->GetCategory() & NodeCategory::Directory)
		|| bool(Nonnull(p_b)->GetCategory() & NodeCategory::Directory)))
	{
		if(IsNodeShared(p_a.get(), p_b.get()))
			return true;

		filebuf fb_a, fb_b;

		errno = 0;
		// FIXME: Implement for streams without open-by-raw-file extension.
		// TODO: Throw a nested exception with %errno if 'errno != 0'.
		if(!fb_a.open(std::move(p_a),
			std::ios_base::in | std::ios_base::binary))
			ystdex::throw_error(std::errc::bad_file_descriptor,
				name_a ? ("Failed opening first file '" + string(name_a) + '\'')
				.c_str() : "Failed opening first file");
		// TODO: Throw a nested exception with %errno if 'errno != 0'.
		if(!fb_b.open(std::move(p_b),
			std::ios_base::in | std::ios_base::binary))
			ystdex::throw_error(std::errc::bad_file_descriptor,
				name_b ? ("Failed opening second file '" + string(name_b)
				+ '\'').c_str() : "Failed opening second file");
		return ystdex::streambuf_equal(fb_a, fb_b);
	}
	return {};
}

bool
IsNodeShared(const char* a, const char* b, bool follow_link) ynothrowv
{
	return a == b || ystdex::ntctscmp(Nonnull(a), Nonnull(b)) == 0
		|| IsNodeShared_Impl(a, b, follow_link);
}
bool
IsNodeShared(const char16_t* a, const char16_t* b, bool follow_link) ynothrowv
{
	return a == b || ystdex::ntctscmp(Nonnull(a), Nonnull(b)) == 0
		|| IsNodeShared_Impl(a, b, follow_link);
}
bool
IsNodeShared(FileDescriptor x, FileDescriptor y) ynothrow
{
	const auto id(x.GetNodeID());

	return id != FileNodeID() && id == y.GetNodeID();
}

} // namespace platform;

namespace platform_ex
{

#if YCL_Win32
wstring
MakePathStringW(const char* s)
{
	return platform_ex::UTF8ToWCS(s);
}
#else
u16string
MakePathStringU(const char* s)
{
	return CHRLib::MakeUCS2LE(s);
}
#endif

} // namespace platform_ex;

