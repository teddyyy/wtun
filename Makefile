obj-m		:= wtun.o
wtun-objs	:= module.o dev.o net.o filter.o

KDIR		:= /lib/modules/$(shell uname -r)/build/
PWD			:= $(shell pwd)

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean

