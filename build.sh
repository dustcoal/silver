rm -rf build
mkdir build
cmake -B build -S .
cd build
make


#mkdir -p cmake_build_files
#cmake -S . -B cmake_build_files
#cd cmake_build_files
#make
#cd ..
#mkdir -p build
#cp cmake_build_files/craft build
#echo "copying server..."
#cp cmake_build_files/server build
#cp -r textures build
#cp -r shaders build
