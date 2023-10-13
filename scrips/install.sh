#!/bin/sh

premake5 gmake 
make config=release

BIN_SRC="./bin/Release/ensr"
BIN_DST="/usr/local/bin"

MAN_SRC="./doc/ensr"
MAN_DST="/usr/local/man"

/bin/cp "$BIN_SRC" "$BIN_DST"
/bin/cp "$MAN_SRC" "$MAN_DST"
