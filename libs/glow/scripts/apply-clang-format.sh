if [ "$#" -ne 1 ]; then
    echo "Usage: ~ \"path/to/src/dir\""
    echo " e.g. ./apply-clang-format src/"
    exit 0
fi

DIR="$1"

for fn in `find $DIR -iname "*.[ch][ch]"`; do
    echo clang-format -i --style=file $fn
    clang-format -i --style=file $fn
done
