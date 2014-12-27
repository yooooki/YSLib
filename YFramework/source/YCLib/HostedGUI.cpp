﻿/*
	© 2013-2014 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file HostedGUI.cpp
\ingroup YCLib
\ingroup YCLibLimitedPlatforms
\brief 宿主 GUI 接口。
\version r727
\author FrankHB <frankhb1989@gmail.com>
\since build 427
\par 创建时间:
	2013-07-10 11:31:05 +0800
\par 修改时间:
	2014-12-18 09:52 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YCLib::Win32GUI
*/


#include "YCLib/YModules.h"
#include YFM_YCLib_HostedGUI
#include YFM_YSLib_Core_YCoreUtilities // for YSLib::CheckPositiveScalar;
#if YCL_Win32
#	include YFM_MinGW32_YCLib_MinGW32
#	include <ystdex/exception.h> // for ystdex::unimplemented;
#elif YCL_Android
#	include YFM_Android_YCLib_Android
#	include <android/native_window.h>
#	include "YSLib/Service/YModules.h"
#	include YFM_YSLib_Service_YGDI
#endif

using namespace YSLib;
using namespace Drawing;

#if YF_Hosted

namespace platform_ex
{

namespace
{

#	if YCL_Win32
//! \since build 388
void
ResizeWindow(::HWND h_wnd, SDst w, SDst h)
{
	if(YB_UNLIKELY(!::SetWindowPos(h_wnd, {}, 0, 0, w, h,
		SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER
		| SWP_NOSENDCHANGING | SWP_NOZORDER)))
		YF_Raise_Win32Exception("SetWindowPos @ ResizeWindow");
}

//! \since build 427
::RECT
FetchWindowRect(::HWND h_wnd)
{
	::RECT rect;

	if(YB_UNLIKELY(!::GetWindowRect(h_wnd, &rect)))
		YF_Raise_Win32Exception("GetWindowRect");
	return rect;
}

//! \since build 445
//@{
Size
FetchSizeFromBounds(const ::RECT& rect)
{
	YAssert(rect.right - rect.left >= 0 && rect.bottom - rect.top >= 0,
		"Invalid boundary found.");
	return {rect.right - rect.left, rect.bottom - rect.top};
}

inline ::DWORD
FetchWindowStyle(::HWND h_wnd)
{
	return ::GetWindowLongW(h_wnd, GWL_STYLE);
}

void
AdjustWindowBounds(::RECT& rect, ::HWND h_wnd, bool b_menu = false)
{
	if(YB_UNLIKELY(!::AdjustWindowRect(&rect, FetchWindowStyle(h_wnd), b_menu)))
		YF_Raise_Win32Exception("AdjustWindowRect");
	YAssert(rect.right - rect.left >= 0 && rect.bottom - rect.top >= 0,
		"Invalid boundary found.");
}

void
SetWindowBounds(::HWND h_wnd, int x, int y, int cx, int cy)
{
	if(YB_UNLIKELY(!::SetWindowPos(h_wnd, {}, x, y, cx, cy,
		SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE
		| SWP_NOOWNERZORDER | SWP_NOSENDCHANGING | SWP_NOZORDER)))
		YF_Raise_Win32Exception("SetWindowPos @ SetWindowBounds");
}
//@}
#	elif YCL_Android
//! \since build 498
SDst
CheckStride(SDst buf_stride, SDst w)
{
	if(buf_stride < w)
		// XXX: Use more specified exception type.
		throw std::runtime_error("Stride is small than width.");
	return buf_stride;
}
#	endif

} // unnamed namespace;


#	if YCL_Win32
void
BindDefaultWindowProc(NativeWindowHandle h_wnd, MessageMap& m, unsigned msg,
	EventPriority prior)
{
	m[msg].Add([=](::WPARAM w_param, ::LPARAM l_param){
		::DefWindowProcW(h_wnd, msg, w_param, l_param);
	}, prior);
}


Rect
WindowReference::GetBounds() const
{
	const auto& rect(FetchWindowRect(GetNativeHandle()));

	return {rect.left, rect.top, FetchSizeFromBounds(rect)};
}
Rect
WindowReference::GetClientBounds() const
{
	return {GetClientLocation(), GetClientSize()};
}
Point
WindowReference::GetClientLocation() const
{
	::POINT point{0, 0};

	if(YB_UNLIKELY(!::ClientToScreen(GetNativeHandle(), &point)))
		YF_Raise_Win32Exception("ClientToScreen");
	return {point.x, point.y};
}
Size
WindowReference::GetClientSize() const
{
	::RECT rect;

	if(YB_UNLIKELY(!::GetClientRect(GetNativeHandle(), &rect)))
		YF_Raise_Win32Exception("GetClientRect");
	return {rect.right, rect.bottom};
}
Point
WindowReference::GetCursorLocation() const
{
	::POINT cursor;

	::GetCursorPos(&cursor);
	::ScreenToClient(GetNativeHandle(), &cursor);
	return {cursor.x, cursor.y};
}
Point
WindowReference::GetLocation() const
{
	const auto& rect(FetchWindowRect(GetNativeHandle()));

	return {rect.left, rect.top};
}
YSLib::Drawing::AlphaType
WindowReference::GetOpacity() const
{
	ystdex::byte a;

	if(YB_UNLIKELY(!::GetLayeredWindowAttributes(GetNativeHandle(), {}, &a,
		{})))
		YF_Raise_Win32Exception("GetLayeredWindowAttributes");
	return a;
}
Size
WindowReference::GetSize() const
{
	return FetchSizeFromBounds(FetchWindowRect(GetNativeHandle()));
}

void
WindowReference::SetClientBounds(const Rect& r)
{
	::RECT rect{r.X, r.Y, CheckScalar<SPos>(r.X + r.Width, "width"),
		CheckScalar<SPos>(r.Y + r.Height, "height")};
	const auto h_wnd(GetNativeHandle());

	AdjustWindowBounds(rect, h_wnd);
	SetWindowBounds(h_wnd, rect.left, rect.top, rect.right - rect.left,
		rect.bottom - rect.top);
}
void
WindowReference::SetOpacity(YSLib::Drawing::AlphaType a)
{
	if(YB_UNLIKELY(!::SetLayeredWindowAttributes(GetNativeHandle(), 0, a,
		LWA_ALPHA)))
		YF_Raise_Win32Exception("SetLayeredWindowAttributes");
}
WindowReference
WindowReference::GetParent() const
{
	// TODO: Implementation.
	throw ystdex::unimplemented();
}

void
WindowReference::SetText(const wchar_t* str)
{
	if(YB_UNLIKELY(!::SetWindowTextW(GetNativeHandle(), str)))
		YF_Raise_Win32Exception("SetWindowTextW");
}

void
WindowReference::Close()
{
	if(YB_UNLIKELY(!::SendNotifyMessageW(GetNativeHandle(), WM_CLOSE, 0, 0)))
		YF_Raise_Win32Exception("SendNotifyMessageW");
}

void
WindowReference::Invalidate()
{
	if(YB_UNLIKELY(!::InvalidateRect(GetNativeHandle(), {}, false)))
		YF_Raise_Win32Exception("InvalidateRect");
}

void
WindowReference::Move(const Point& pt)
{
	if(YB_UNLIKELY(!::SetWindowPos(GetNativeHandle(), {}, pt.X, pt.Y, 0, 0,
		SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW
		| SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOZORDER)))
		YF_Raise_Win32Exception("SetWindowPos @ WindowReference::Move");
}

void
WindowReference::Resize(const Size& s)
{
	ResizeWindow(GetNativeHandle(), s.Width, s.Height);
}

void
WindowReference::ResizeClient(const Size& s)
{
	::RECT rect{0, 0, CheckScalar<SPos>(s.Width, "width"),
		CheckScalar<SPos>(s.Height, "height")};

	AdjustWindowBounds(rect, GetNativeHandle());
	ResizeWindow(GetNativeHandle(), rect.right - rect.left,
		rect.bottom - rect.top);
}

bool
WindowReference::Show(int n_cmd_show) ynothrow
{
	return ::ShowWindowAsync(GetNativeHandle(), n_cmd_show) != 0;
}
#	elif YCL_Android
SDst
WindowReference::GetWidth() const
{
	return CheckPositiveScalar<SDst>(
		::ANativeWindow_getWidth(GetNativeHandle()), "width");
}
SDst
WindowReference::GetHeight() const
{
	return CheckPositiveScalar<SDst>(::ANativeWindow_getHeight(
		GetNativeHandle()), "height");
}
#	endif


#	if YCL_Win32
NativeWindowHandle
CreateNativeWindow(const wchar_t* class_name, const Drawing::Size& s,
	const wchar_t* title, ::DWORD wstyle, ::DWORD wstyle_ex)
{
	::RECT rect{0, 0, CheckScalar<SPos>(s.Width, "width"),
		CheckScalar<SPos>(s.Height, "height")};

	::AdjustWindowRect(&rect, wstyle, false);
	return ::CreateWindowExW(wstyle_ex, class_name, title, wstyle,
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom
		- rect.top, {}/*HWND_DESKTOP*/, {}, ::GetModuleHandleW({}), {});
}
#	elif YCL_Android
void
UpdateContentTo(NativeWindowHandle h_wnd, const Rect& r, const ConstGraphics& g)
{
	::ANativeWindow_Buffer abuf;
	::ARect arect{r.X, r.Y, r.X + r.Width, r.Y + r.Height};

	::ANativeWindow_lock(Nonnull(h_wnd), &abuf, &arect);
	CopyTo(static_cast<BitmapPtr>(abuf.bits), g,
		WindowReference(h_wnd).GetSize(), r.GetPoint(), {}, r.GetSize());
	::ANativeWindow_unlockAndPost(h_wnd);
}


class ScreenBufferData : public CompactPixmap
{
public:
	ScreenBufferData(const Size&, SDst);

	DefDeMoveCtor(ScreenBufferData)
};

ScreenBufferData::ScreenBufferData(const Size& s, SDst buf_stride)
	: CompactPixmap({}, CheckStride(buf_stride, s.Width), s.Height)
{}
#	endif


#	if YCL_Win32
ScreenBuffer::ScreenBuffer(const Size& s)
	: size(s), hBitmap([this]{
		// NOTE: Bitmap format is hard coded here for explicit buffer
		//	compatibility. %::CreateCompatibleBitmap is not fit for unknown
		//	windows.
		::BITMAPINFO bmi{{sizeof(::BITMAPINFOHEADER), CheckPositiveScalar<SPos>(
			size.Width, "width"),  -CheckPositiveScalar<SPos>(size.Height,
			"height") - 1, 1, 32, BI_RGB, ::DWORD(sizeof(Pixel) * size.Width
			* size.Height), 0, 0, 0, 0}, {}};

		return ::CreateDIBSection({}, &bmi, DIB_RGB_COLORS,
			&reinterpret_cast<void*&>(pBuffer), {}, 0);
	}())
{}
ScreenBuffer::ScreenBuffer(ScreenBuffer&& sbuf) ynothrow
	: size(sbuf.size), hBitmap(sbuf.hBitmap)
{
	sbuf.hBitmap = {};
}
#	elif YCL_Android
ScreenBuffer::ScreenBuffer(const Size& s)
	: ScreenBuffer(s, s.Width)
{}
ScreenBuffer::ScreenBuffer(const Size& s, SDst buf_stride)
	: p_impl(new ScreenBufferData(s, buf_stride)), width(s.Width)
{}
ScreenBuffer::ScreenBuffer(ScreenBuffer&& sbuf) ynothrow
	: p_impl(new ScreenBufferData(std::move(*sbuf.p_impl))), width(sbuf.width)
{
	sbuf.width = 0;
}
#	endif
ScreenBuffer::~ScreenBuffer()
{
#	if YCL_Win32
	::DeleteObject(hBitmap);
#	endif
}

#	if YCL_Win32
ScreenBuffer&
ScreenBuffer::operator=(ScreenBuffer&& sbuf)
{
	sbuf.swap(*this);
	return *this;
}

void
ScreenBuffer::Resize(const Size& s)
{
	if(s != size)
		*this = ScreenBuffer(s);
}

void
ScreenBuffer::Premultiply(ConstBitmapPtr p_buf) ynothrow
{
	// NOTE: Since the stride is guaranteed equal to the width, the storage for
	//	pixels can be supposed to be contiguous.
	std::transform(p_buf, p_buf + size.Width * size.Height, pBuffer,
		[](const Pixel& pixel){
			const auto a(pixel.GetA());

			return Pixel{MonoType(pixel.GetB() * a / 0xFF),
				MonoType(pixel.GetG() * a / 0xFF),
				MonoType(pixel.GetR() * a / 0xFF), a};
	});
}
#	elif YCL_Android
BitmapPtr
ScreenBuffer::GetBufferPtr() const ynothrow
{
	return Deref(p_impl).GetBufferPtr();
}
const YSLib::Drawing::Graphics&
ScreenBuffer::GetContext() const ynothrow
{
	return Deref(p_impl).GetContext();
}
Size
ScreenBuffer::GetSize() const ynothrow
{
	return {width, Deref(p_impl).GetHeight()};
}
YSLib::SDst
ScreenBuffer::GetStride() const ynothrow
{
	return Deref(p_impl).GetWidth();
}

void
ScreenBuffer::Resize(const Size& s)
{
	// TODO: Expand stride for given width using a proper strategy.
	Deref(p_impl).SetSize(s);
	width = s.Width;
}
#	endif

void
ScreenBuffer::UpdateFrom(ConstBitmapPtr p_buf) ynothrow
{
#	if YCL_Win32
	// NOTE: Since the pitch is guaranteed equal to the width, the storage for
	//	pixels can be supposed to be contiguous.
	std::copy_n(Nonnull(p_buf), size.Width * size.Height, GetBufferPtr());
#	elif YCL_Android
	// TODO: Expand stride for given width using a proper strategy.
	std::copy_n(Nonnull(p_buf), GetAreaOf(GetSize()),
		Deref(p_impl).GetBufferPtr());
#	endif
}

void
ScreenBuffer::swap(ScreenBuffer& sbuf) ynothrow
{
#	if YCL_Win32
	std::swap(size, sbuf.size),
	std::swap(pBuffer, sbuf.pBuffer),
	std::swap(hBitmap, sbuf.hBitmap);
#	elif YCL_Android
	Deref(p_impl).swap(Deref(sbuf.p_impl)),
	std::swap(width, sbuf.width);
#	endif
}


void
ScreenRegionBuffer::UpdateFrom(ConstBitmapPtr p_buf) ynothrow
{
	lock_guard<mutex> lck(mtx);

	ScreenBuffer::UpdateFrom(p_buf);
}

#	if YCL_Win32
void
ScreenRegionBuffer::UpdatePremultipliedTo(NativeWindowHandle h_wnd, AlphaType a,
	const Point& pt)
{
	lock_guard<mutex> lck(mtx);
	GSurface<> sf(h_wnd);

	sf.UpdatePremultiplied(*this, h_wnd, a, pt);
}
#	elif YCL_Android
ScreenRegionBuffer::ScreenRegionBuffer(const Size& s)
	: ScreenRegionBuffer(s, s.Width)
{}
ScreenRegionBuffer::ScreenRegionBuffer(const Size& s, SDst buf_stride)
	: ScreenBuffer(s, buf_stride),
	mtx()
{}
#	endif

void
ScreenRegionBuffer::UpdateTo(NativeWindowHandle h_wnd, const Point& pt) ynothrow
{
	lock_guard<mutex> lck(mtx);
#	if YCL_Win32
	GSurface<> sf(h_wnd);

	sf.Update(*this, pt);
#	elif YCL_Android
	UpdateContentTo(h_wnd, {pt, GetSize()}, GetContext());
#	endif
}


#	if YCL_Win32
void
WindowMemorySurface::Update(ScreenBuffer& sbuf, const Point& pt) ynothrow
{
	const auto h_old(::SelectObject(h_mem_dc, sbuf.GetNativeHandle()));
	const auto& s(sbuf.GetSize());

	// NOTE: Unlocked intentionally for performance.
	::BitBlt(h_owner_dc, pt.X, pt.Y, s.Width, s.Height, h_mem_dc, 0, 0,
		SRCCOPY);
	::SelectObject(h_mem_dc, h_old);
}
void
WindowMemorySurface::UpdatePremultiplied(ScreenBuffer& sbuf,
	NativeWindowHandle h_wnd, YSLib::Drawing::AlphaType a, const Point& pt)
{
	const auto h_old(::SelectObject(h_mem_dc, sbuf.GetNativeHandle()));
	auto rect(FetchWindowRect(h_wnd));
	::SIZE size{rect.right - rect.left, rect.bottom - rect.top};
	::POINT ptx{pt.X, pt.Y};
	::BLENDFUNCTION bfunc{AC_SRC_OVER, 0, a, AC_SRC_ALPHA};

	// NOTE: Unlocked intentionally for performance.
	if(YB_UNLIKELY(!::UpdateLayeredWindow(h_wnd, h_owner_dc,
		reinterpret_cast<::POINT*>(&rect), &size, h_mem_dc, &ptx, 0, &bfunc,
		ULW_ALPHA)))
	{
		// TODO: Use RAII.
		::SelectObject(h_mem_dc, h_old);
		YF_Raise_Win32Exception("UpdateLayeredWindow");
	}
	::SelectObject(h_mem_dc, h_old);
}


WindowClass::WindowClass(const wchar_t* class_name, ::WNDPROC wnd_proc,
	unsigned style, ::HBRUSH h_bg)
	: h_instance(::GetModuleHandleW({}))
{
	// NOTE: Intentionally no %CS_OWNDC or %CS_CLASSDC, so %::ReleaseDC
	//	is always needed.
	const ::WNDCLASSW wnd_class{style, wnd_proc, 0, 0, h_instance,
		::LoadIconW({}, IDI_APPLICATION), ::LoadCursorW({}, IDC_ARROW),
		h_bg, nullptr, class_name};

	if(YB_UNLIKELY(::RegisterClassW(&wnd_class) == 0))
		YF_Raise_Win32Exception("RegisterClassW");
	// TODO: Trace class name.
	YTraceDe(Notice, "Window class registered.");
}
WindowClass::~WindowClass()
{
	::UnregisterClassW(WindowClassName, h_instance);
	// TODO: Trace class name.
	YTraceDe(Notice, "Window class unregistered.");
}
#	endif


HostWindow::HostWindow(NativeWindowHandle h)
	: WindowReference(h)
#	if YCL_Win32
	, MessageMap()
#	endif
{
#	if YCL_Win32
	YAssert(::IsWindow(h), "Invalid window handle found.");
	YAssert(::GetWindowThreadProcessId(h, {}) == ::GetCurrentThreadId(),
		"Window not created on current thread found.");
	YAssert(::GetWindowLongPtrW(h, GWLP_USERDATA) == 0,
		"Invalid user data of window found.");

	wchar_t buf[ystdex::arrlen(WindowClassName)];

	if(YB_UNLIKELY(!::GetClassNameW(GetNativeHandle(), buf,
		ystdex::arrlen(WindowClassName))))
		YF_Raise_Win32Exception("GetClassNameW");
	if(std::wcscmp(buf, WindowClassName) != 0)
		throw GeneralEvent("Wrong windows class name found.");
	::SetLastError(0);
	if(YB_UNLIKELY(::SetWindowLongPtrW(GetNativeHandle(), GWLP_USERDATA,
		::LONG_PTR(this)) == 0 && GetLastError() != 0))
		YF_Raise_Win32Exception("SetWindowLongPtrW");
	if(YB_UNLIKELY(!::SetWindowPos(GetNativeHandle(), {}, 0, 0, 0, 0,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREDRAW
		| SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOZORDER)))
		YF_Raise_Win32Exception("SetWindowPos @ HostWindow::HostWindow");

	::RAWINPUTDEVICE rid{0x01, 0x02, 0, nullptr};

	if(YB_UNLIKELY(!::RegisterRawInputDevices(&rid, 1, sizeof(rid))))
		YF_Raise_Win32Exception("RegisterRawInputDevices");
	MessageMap[WM_DESTROY] += []{
		::PostQuitMessage(0);
	};
#	elif YCL_Android
	::ANativeWindow_acquire(GetNativeHandle());
#	endif
}

HostWindow::~HostWindow()
{
#	if YCL_Win32
	const auto h_wnd(GetNativeHandle());

	::SetWindowLongPtrW(h_wnd, GWLP_USERDATA, ::LONG_PTR());
	// NOTE: The window could be already destroyed in window procedure.
	if(::IsWindow(h_wnd))
		::DestroyWindow(h_wnd);
#	elif YCL_Android
	::ANativeWindow_release(GetNativeHandle());
#	endif
}

#	if YCL_Win32
Point
HostWindow::MapPoint(const Point& pt) const
{
	return pt;
}
#	endif

} // namespace platform_ex;

#endif
