
CC	=arm-linux-gnueabihf-gcc
TARGET	=app_rob
OBJECT	=$(TARGET).o
SOURCE	=$(TARGET).c

SUB =doorlock

obj-m = Doorlock_md.o 
#KDIR  := /lib/modules/$(shell uname -r)/build
KDIR  := /mnt/RaspBerry/linux
PWD	:= $(shell pwd)
default:$(TARGET)
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KDIR) SUBDIRS=$(PWD) modules
$(TARGET):$(SOURCE) $(OBJECT)
	$(CC) -g -o $@ -static $< $(SUB).o -DDEBUG
$(OBJECT):$(SOURCE)
	$(CC) -c $(SUB).c
	#$(CC) -c $< $(SUB).o

clean:
	rm -rf *.o *.ko *.mod.c .tmp_versions
	rm -rf modules.order Module.symvers
	rm -rf .*.ko.cmd .*.mod.o.cmd .*.o.cmd
	rm -rf $(TARGET) $(SUB).o

