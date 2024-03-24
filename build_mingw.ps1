rm -r -force build
mkdir build
#cp deps\curl_windows_mingw\libcurl-x64.dll build
#Copy-Item shaders -Destination build -Recurse
#Copy-Item textures -Destination build -Recurse
.\buildtools\windows\cmake\bin\cmake.exe -B build -S . -G "MinGW Makefiles" --toolchain=mingw64-toolchain.cmake
cd build
.\..\buildtools\windows\mingw64\bin\mingw32-make.exe