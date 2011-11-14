#!/bin/sh

PREFIX=/home/harald/Documents/mingw32
TARGET=i386-mingw32
PATH="$PREFIX/bin:$PREFIX/$TARGET/bin:$PATH"
export PATH
exec scons $*
