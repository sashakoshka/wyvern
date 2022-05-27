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

lint)
        clang-tidy \
        -checks=portability-*,bugprone-*,-bugprone-macro-parentheses \
        --warnings-as-errors=* "$SRC_PATH"/*/*.c -- $FLAGS_CFLAGS
        ;;

val)
        buildAll
        valgrind --tool=memcheck --leak-check=yes $DEBUG_PATH
        ;;

*)
        buildModule $1 $2
        ;;
esac
