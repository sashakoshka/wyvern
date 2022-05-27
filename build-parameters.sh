CC="clang"
OBJ_PATH="o"
OUT_PATH="bin"
SRC_PATH="src"
INC_PATH="include"
EXE_NAME="wyvern"

FLAGS_RELEASE="-O3 -DRELEASE"
FLAGS_WARN="-Wall -Wextra"
FLAGS_DEBUG="-g -DDEBUG"

INSTALL_LOCATION="/usr/local"

library "cairo"
library "x11"
library "freetype2"
library "xkbcommon"
