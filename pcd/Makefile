obj-m := pcd.o

# Cross Compilation Information
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
## Define the CROSS_KERNEL_DIR throught the export
## Example: export CROSS_KERNEL_DIR=/home/linux
KERNEL_DIR=${CROSS_KERNEL_DIR}

# Host Compilation Information
HOST_KERNEL_DIR=/lib/modules/$(shell uname -r)/build/

target:
	which arm-linux-gnueabihf-gcc
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) modules

host:
	make -C $(HOST_KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(HOST_KERNEL_DIR) M=$(PWD) clean
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(CROSS_KERNEL_DIR) M=$(PWD) clean
