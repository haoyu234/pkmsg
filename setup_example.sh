#!/bin/sh

work_dir=$(cd "$(dirname $0)";pwd)

sh ${work_dir}/subprojects/columns/columns.sh -C ${work_dir}/example --std=c11 -p plugin.py ./messages.h
