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
        buildAll && "./$DEBUG_PATH" $ARGS_DEBUG
        ;;

lint)
        clang-tidy \
        -checks=portability-*,bugprone-*,-bugprone-macro-parentheses \
        --warnings-as-errors=* "$SRC_PATH"/*/*.c -- $FLAGS_CFLAGS
        ;;

memcheck)
        buildAll && \
        valgrind --tool=memcheck --leak-check=yes \
        $DEBUG_PATH $ARGS_DEBUG
        ;;

profile)
        rm *.out.*
        buildAll && \
        valgrind --tool=callgrind \
        $DEBUG_PATH $ARGS_DEBUG
        kcachegrind *.out.*
        ;;
*)
        buildModule $1 $2
        ;;
esac
