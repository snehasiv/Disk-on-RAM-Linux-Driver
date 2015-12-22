ifeq ($(KERNELRELEASE),)

	KERNEL_SOURCE := /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

module:
	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) clean

else

	obj-m := dorc.o dorb.o
	dorc-y := ram_char.o ram_device.o partition.o	
	dorb-y := ram_block.o ram_device.o partition.o
endif
