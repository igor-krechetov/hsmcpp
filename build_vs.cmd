rmdir .\build /S /Q
mkdir .\build
cd .\build

@rem Fix path according to your system
call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

@rem Change Qt5_DIR to match your system if you want to enable Qt dispatcher
@rem set Qt5_DIR=C:\Qt\5.9\5.9.2\msvc2015\lib\cmake\Qt5
@rem cmake -DHSMBUILD_DISPATCHER_GLIB=OFF -DHSMBUILD_DISPATCHER_GLIBMM=OFF ..
cmake -DHSMBUILD_DISPATCHER_GLIB=OFF -DHSMBUILD_DISPATCHER_GLIBMM=OFF -DHSMBUILD_DISPATCHER_QT=OFF ..
MSBuild.exe hsmcpp.sln -target:ALL_BUILD
pause
