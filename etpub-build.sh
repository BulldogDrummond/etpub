#!/bin/sh

cd trunk/src
mkdir bin

scons BUILD=release COPYBINS=1

# EOF
