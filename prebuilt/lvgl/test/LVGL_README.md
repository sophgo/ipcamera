# 下载编译LVGL开源库

## 下载源码

源码链接：https://github.com/lvgl/lvgl/archive/refs/tags/v9.1.0.tar.gz

## 配置

### 配置编译工具链

项目根目录CMakeLists.txt添加如下内容：

```cmake
set(CMAKE_C_COMPILER "riscv64-unknown-linux-musl-gcc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
```

### 创建编译脚本

项目根目录创建编译脚本build.sh

```shell
# build.sh
rm -rf build/
rm -rf install/
mkdir build
mkdir install
cd build/
cmake -DCMAKE_INSTALL_PREFIX=/home/liyuan.si/workspace/code/v4.1.0.lvgl/lvgl/lvgl-9.1.0/install ..
make
make install
```

CMAKE_INSTALL_PREFIX为安装路径，需根据用户实际路径配置。

### 配置编译选项

拷贝项目根目录下的lv_conf_template.h，并重命名为lv_conf.h。

配置lv_conf.h内的宏定义即可配置具体模块开关使能。（可参考ipcamera/prebuilt/lvgl/include/lvgl/lv_conf.h）

## 编译

执行编译脚本./build.sh

编译完成后，会在install目录下生成库、头文件、demo文件。