CC="clang"
OBJ_PATH="o"
OUT_PATH="bin"
SRC_PATH="src"
INC_PATH="include"
EXE_NAME="wyvern"

FLAGS_RELEASE="-03 -Drelease"
FLAGS_WARN="-Wall -Wextra"
FLAGS_DEBUG="-g -Ddebug"

INSTALL_LOCATION="/usr/local"

library "cairo"
library "x11"
library "freetype2"
library "xkbcommon"
