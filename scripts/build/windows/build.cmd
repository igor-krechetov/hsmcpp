@echo off
set BUILD_DIR=%1
set BUILD_TYPE=%2

cmake --build "%BUILD_DIR%" --parallel 2 --config %BUILD_TYPE%
