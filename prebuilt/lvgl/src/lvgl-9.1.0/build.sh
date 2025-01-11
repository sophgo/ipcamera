# build.sh
rm -rf build/
rm -rf install/
mkdir build
mkdir install
cd build/
cmake -DCMAKE_INSTALL_PREFIX=/home/liyuan.si/workspace/code/v4.1.0.lvgl/lvgl/lvgl-9.1.0/install ..
make
make install

