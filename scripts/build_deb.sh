#!/usr/bin/env bash

set -e

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUTPUT_DIR="$ROOT_DIR/build/deb"

if [[ ! -f "debian/control" || ! -f "CMakeLists.txt" ]]; then
    echo "错误：必须在项目根目录下运行此脚本！"
    echo "请确保当前目录包含 debian/control 和 CMakeLists.txt 文件"
    exit 1
fi


rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

cd "$ROOT_DIR"

debuild -us -uc -b

mv ../breutil_* "$OUTPUT_DIR"/
mv ../breutil-dev*.deb "$OUTPUT_DIR"/

rm -r obj-x86_64-linux-gnu/
rm -r debian/breutil*

ls -lh "$OUTPUT_DIR"
