# 编译和使用lvgl_test
## 硬件

板卡：cv1810c_wevb_0006a_spinor
屏幕：MIPI_panel_hx8394 （MIPI 2lane）
外设：usb 2.0 鼠标

## 配置内核开关

1. frame buffer

   ```
   CONFIG_FB=m
   CONFIG_FB_CVITEK=m
   ```

2. 支持USB鼠标设备（可选）

   ```
   CONFIG_USB=y
   CONFIG_USB_DWC2=y
   CONFIG_USB_GADGET=y
   CONFIG_USB_HID=y
   CONFIG_HID_GENERIC=y
   CONFIG_INPUT=y
   CONFIG_INPUT_EVDEV=y
   ```
## 编译

SDK根目录执行如下命令：（交叉编译工具链添加到环境变量，Makefile会使用）

```shell
source build/cvisetup.sh
defconfig cv1811c_wevb_0006a_spinor
```

编译lvgl_test，执行如下命令：

```
cd ipcamera/prebuilt/lvgl/test/
make clean;make
```

编译成功后，会生成可执行文件lvgl_test

## 执行

1. 加载frame buffer驱动（基于前面内核配置）

   ```shell
   insmod /mnt/system/ko/fb.ko
   insmod /mnt/system/ko/cfbcopyarea.ko
   insmod /mnt/system/ko/cfbfillrect.ko
   insmod /mnt/system/ko/cfbimgblt.ko
   insmod /mnt/system/ko/cvi_fb.ko
   ```

2. 加载usb驱动并配置管脚（可选）

   ```shell
   # 不通板卡管脚不一样，如下配置基于cv1810c
   devmem 0x030010B0 32 3
   echo host > /proc/cviusb/otg_role
   echo 352  >  /sys/class/gpio/export
   echo out > /sys/class/gpio/gpio352/direction
   echo 0 > /sys/class/gpio/gpio352/value
   ```
   
3. 加载PWM驱动支持调节屏幕亮度（可选）

   ```shell
   insmod /mnt/nfs/run/v4.1.0.lvgl/cv181x_pwm.ko
   devmem 0x03001068 32 0x2
   ```

4. 运行

   ./lvgl_test
   
   执行成功后，屏幕会显示滑动条，鼠标拖动滑动条可调节屏幕亮度。



## Q&A:

1. LVGL报错内存申请不到。

   增大lv_conf.h内的LV_MEM_SIZE的大小

2. 屏幕指针漂移

   改用linux evdev即可避免。

3. 画面刷新有条纹，即部分区域不能及时更新完毕

   每次frame buffer刷新都需调用 ioctl(dsc->fbfd, FBIOPUT_VSCREENINFO, &(dsc->vinfo)

4. 没有/dev/input/event0节点

   重新插拔usb即可

5. vo出流前提下，ui画面超过一半会出现画面异常。

   由于fb复用bootlogo的内存，需要增加bootlogo的大小。

6. LVGL  fb格式支持

   RGB565 RGB888 XRGB8888

7. cvi_fb格式支持

   argb1555 argb4444 argb8888