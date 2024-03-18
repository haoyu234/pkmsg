#!/bin/sh

work_dir=$(cd "$(dirname $0)";pwd)

valgrind --tool=memcheck --leak-check=full ${work_dir}/builddir/example
