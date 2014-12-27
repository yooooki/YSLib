#!/usr/bin/bash
# (C) 2014 FrankHB.
# Script for build YSLib using SHBuild.

set -e
SHBuild_ToolDir=$(cd `dirname "$0"`; pwd)
: ${SHBuild_BaseDir:="$SHBuild_ToolDir/../SHBuild"}
. $SHBuild_ToolDir/SHBuild-common.sh
: ${AR:="gcc-ar"}
export AR
. $SHBuild_ToolDir/SHBuild-common-toolchain.sh

: ${SHBuild:="$SHBuild_BaseDir/SHBuild"}
SHBuild_CheckUName
if [[ "$SHBuild_Env_OS" == "Win32" ]]; then
	: ${SHBuild_YSLib_Platform:="MinGW32"}
	SHBuild_YF_SystemLibs="-lgdi32 -limm32"
else
	: ${SHBuild_YSLib_Platform:=$SHBuild_Env_OS}
	SHBuild_YF_SystemLibs="-lxcb -lpthread"
fi

INCLUDES_YBase="-I../../YBase/include"
INCLUDES_YFramework=" \
	-I../../YFramework/include -I../../YFramework/Android/include \
	-I../../YFramework/DS/include -I../../YFramework/MinGW32/include \
	-I../../3rdparty/include -I../../YBase/include"
SHBOPT_IGN="-xid,alternative -xid,data -xid,include -xid,Android"

LIBS_YFramework=" \
	-L../../YFramework/$SHBuild_YSLib_Platform/lib-$SHBuild_Env_Arch \
	-lFreeImage -lfreetype $SHBuild_YF_SystemLibs"

SHBuild_EchoVar "SHBuild" "$SHBuild"
SHBuild_EchoVar "SHBuild_YSLib_Platform" "$SHBuild_YSLib_Platform"
SHBuild_EchoVar "INCLUDES_YBase" "$INCLUDES_YBase"
SHBuild_EchoVar "INCLUDES_YFramework" "$INCLUDES_YFramework"
SHBuild_EchoVar "LIBS_YFramework" "$LIBS_YFramework"

export LANG=C
unset CXXFLAGS_COMMON
unset CXXFLAGS
