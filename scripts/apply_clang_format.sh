#!/bin/sh

find ./src/ -iname *.hpp -o -iname *.cpp | xargs clang-format -i
find ./examples/ -iname *.hpp -o -iname *.cpp | xargs clang-format -i
find ./tests/ -iname *.hpp -o -iname *.cpp | xargs clang-format -i
