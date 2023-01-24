obj-m:=chenillard.o
KERNEL_SOURCE=/lib/modules/$(shell uname -r)/build

all :
	make -C $(KERNEL_SOURCE) M=$(PWD) modules
clean :
	make -C $(KERNEL_SOURCE) M=$(PWD) clean
install :
	make −C $(KERNEL_SOURCE) M=$(PWD) modules install

