# 交叉编译工具链设置
# CROSS_COMPILE = /home/liyuan.si/workspace/toolchain/host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
CROSS_COMPILE = riscv64-unknown-linux-musl-

# 编译器设置
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++

MPI_INC         += $(TOP_DIR)/cvi_mpi/include
MPI_PANEL_INC   += $(TOP_DIR)/cvi_mpi/component/panel/cv181x
OSDRV_COMM_INC  += $(TOP_DIR)/osdrv/interdrv/include/common/uapi
OSDRV_CHIP_INC  += $(TOP_DIR)/osdrv/interdrv/include/chip/cv181x/uapi
LVGL_INC        += ../include

LVGL_LIB        += ../lib
MPI_LIB         += $(TOP_DIR)/cvi_mpi/lib

CFLAGS = -I$(LVGL_INC) -I$(MPI_INC) -I$(MPI_PANEL_INC) -I$(OSDRV_COMM_INC) -I$(OSDRV_CHIP_INC) # 外部头文件路径
LDFLAGS = -static -L$(LVGL_LIB) -L$(MPI_LIB) -llvgl -llvgl_examples -llvgl_demos -llvgl_thorvg -lmipi_tx # 外部库路径和库名

# 源文件和目标文件
SRC = lvgl_test.c
OBJ = lvgl_test.o
TARGET = lvgl_test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CPP) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)
