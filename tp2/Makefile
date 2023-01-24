obj-m:=chenillard.o
KERNEL_SOURCE=/home/ensea/linux-socfpga/
CFLAGS_MODULE=-fno-pic

all :
	make -C	$(CFLAGS)	$(KERNEL_SOURCE) M=$(PWD) modules
clean :
	make -C	$(CFLAGS)	$(KERNEL_SOURCE) M=$(PWD) clean
install :
	make −C	$(CFLAGS)	$(KERNEL_SOURCE) M=$(PWD) modules install

