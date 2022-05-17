#!/bin/sh
# please keep this posix compliant!

CC="clang"
OBJ_PATH="o"
OUT_PATH="bin"
SRC_PATH="src"
INC_PATH="include"

RELEASE_PATH="$OUT_PATH/wyvern"
DEBUG_PATH="$OUT_PATH/wyvern-debug"

FLAGS_RELEASE="-03 -Drelease"
FLAGS_WARN="-Wall -Wextra"
FLAGS_DEBUG="-g -Ddebug"

FLAGS_CFLAGS="$FLAGS_CFLAGS $(pkg-config --cflags cairo)"
FLAGS_CFLAGS="$FLAGS_CFLAGS $(pkg-config --cflags x11)"
FLAGS_CFLAGS="$FLAGS_CFLAGS $(pkg-config --cflags freetype2)"
FLAGS_CFLAGS="$FLAGS_CFLAGS $(pkg-config --cflags xkbcommon)"
FLAGS_CFLAGS="$FLAGS_CFLAGS -I$INC_PATH"

FLAGS_LIBS="$FLAGS_LIBS $(pkg-config --libs cairo)"
FLAGS_LIBS="$FLAGS_LIBS $(pkg-config --libs x11)"
FLAGS_LIBS="$FLAGS_LIBS $(pkg-config --libs freetype2)"
FLAGS_LIBS="$FLAGS_LIBS $(pkg-config --libs xkbcommon)"

INSTALL_LOCATION="/usr/local"

# build a single module from src

buildModule () {
  mkdir -p "$OBJ_PATH"
  mkdir -p "$OBJ_PATH/release"
  mkdir -p "$OBJ_PATH/debug"

  modIn="$SRC_PATH/$1.c"
  modHead="$INC_PATH/$1.h"

  flags="-c $FLAGS_WARN $FLAGS_CFLAGS"
  if [ "$2" = "release" ]
  then flags="$flags $FLAGS_RELEASE"
       modOut="$OBJ_PATH/release/$1.o"
  else flags="$flags $FLAGS_DEBUG"
       modOut="$OBJ_PATH/debug/$1.o"
  fi
  
  if [ ! -f "$modIn" ]; then
    echo "!!! module $1 does not exist, skipping" >&2; return
  fi
  
  if [ "$modOut" -nt "$modIn" ] && [ "$modOut" -nt "$modHead" ]; then
    echo "(i) skipping module $1, already built"; return
  fi
  
  echo "... building module $1: $1.c ---> $1.o"
  
  $CC "$modIn" -o "$modOut" $flags && echo ".// built module $1" \
  || echo "ERR could not build module $1" >&2
}

# build all modules in src, then link them together into final executable

buildAll () {
  mkdir -p "$OUT_PATH"
  
  echo "... building all modules"

  for module in $SRC_PATH/*.c; do
    buildModule $(basename "${module%.*}") "$1"
  done

  echo "... building entire executable"
  
  flags="$FLAGS_WARN $FLAGS_LIBS"
  if [ "$1" = "release" ]
  then flags="$flags $FLAGS_RELEASE -s"
       allIn="$OBJ_PATH/release/*.o"
       allOut="$RELEASE_PATH"
  else flags="$flags $FLAGS_DEBUG"
       allIn="$OBJ_PATH/debug/*.o"
       allOut="$DEBUG_PATH"
  fi

  if $CC $allIn -o "$allOut" $flags
  then echo ".// built entire executable"
  else echo "ERR could not build executable" >&2
       return
  fi
}

# clean everything

clean () {
  rm -f $OBJ_PATH/debug/* $OBJ_PATH/release/* $OUT_PATH/* && echo "(i) cleaned"
}

# control script

case $1 in
  all)     buildAll $2      ;;
  release) buildAll release ;;
  "")      buildAll         ;;

  redo)
    clean
    buildAll $2
    ;;
  
  clean)
    clean
    ;;
    
  run)
    buildAll && "./$DEBUG_PATH"
    ;;
    
  *) buildModule $1 $2 ;;
esac
