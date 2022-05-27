#!/bin/sh
# please keep this posix compliant!

# add a library

library () {
        FLAGS_CFLAGS="$FLAGS_CFLAGS $(pkg-config --cflags $1)"        
        FLAGS_LIBS="$FLAGS_LIBS $(pkg-config --libs $1)"
}

# include build parameters

. "./build-parameters.sh"

FLAGS_CFLAGS="$FLAGS_CFLAGS -I$INC_PATH"
RELEASE_PATH="$OUT_PATH/$EXE_NAME"
DEBUG_PATH="$OUT_PATH/$EXE_NAME-debug"

# build a single module from src

buildModule () {
        mkdir -p "$OBJ_PATH"
        mkdir -p "$OBJ_PATH/release"
        mkdir -p "$OBJ_PATH/debug"

        modIn="$SRC_PATH/$1"
        modHead="$INC_PATH/$1.h"

        flags="-c $FLAGS_WARN $FLAGS_CFLAGS"
        if [ "$2" = "release" ]; then
                flags="$flags $FLAGS_RELEASE"
                modOut="$OBJ_PATH/release/$1.o"
        else
                flags="$flags $FLAGS_DEBUG"
                modOut="$OBJ_PATH/debug/$1.o"
        fi

        # check if the module even exists
        if [ ! -d "$modIn" ]; then
                echo "!!! module $1 does not exist, skipping" >&2; return
        fi

        buildModule=""

        for sourceFile in $modIn/*; do
                if [ "$sourceFile" -nt "$modOut" ]; then
                        buildModule=true
                        break
                fi
        done

        [ ! -f "$modOut" ]           && buildModule=true
        [ "$modHead" -nt "$modOut" ] && buildModule=true

        if [ -z $buildModule ]; then
                # return if the module hasn't been built
                echo "(i) skipping module $1, already built"; return
        fi

        # build the module
        echo "... building module $1: $1/*.c ---> $1.o"
        $CC $modIn/*.c -o "$modOut" $flags && echo ".// built module $1" \
        || echo "ERR could not build module $1" >&2
}

# build all modules in src, then link them together into final executable

buildAll () {
        mkdir -p "$OUT_PATH"

        echo "... building all modules"

        for module in $SRC_PATH/*; do
                buildModule "$(basename $module)" "$1"
        done

        echo "... building entire executable"

        flags="$FLAGS_WARN $FLAGS_LIBS"
        if [ "$1" = "release" ]; then
                flags="$flags $FLAGS_RELEASE -s"
                allIn="$OBJ_PATH/release/*.o"
                allOut="$RELEASE_PATH"
        else
                flags="$flags $FLAGS_DEBUG"
                allIn="$OBJ_PATH/debug/*.o"
                allOut="$DEBUG_PATH"
        fi

        if $CC $allIn -o "$allOut" $flags; then
                echo ".// built entire executable"
        else
                echo "ERR could not build executable" >&2
                return
        fi
}

# clean everything

clean () {
        rm -f $OBJ_PATH/debug/* $OBJ_PATH/release/* $OUT_PATH/* \
        && echo "(i) cleaned"
}

# targets

. "./build-targets.sh"
