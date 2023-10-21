CFLAGS     = -m32 -nostdlib -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -fno-leading-underscore
AFLAGS     = --32
LDFLAGS    =
IDIR       =/tmp/hos
SRCDIRS    =. hos/drivers hos/kernel hos/algorithm

all: hos.iso hos.bin

## -------------------------------------------------------------------
HEADERS = $(shell find $(SRCDIRS) -maxdepth 1 -regex '.*\.h')
OBJECTS = $(shell find $(SRCDIRS) -maxdepth 1 -regex '.*\.[cs]' | sed 's|\.[cs]|\.o|')

%.o: %.c $(HEADERS) hos/kernel/config.h
	gcc $(CFLAGS) -o $@ -c $<
%.o: %.s
	as $(AFLAGS) -o $@ $<
hos.bin: hos/kernel/32.ld $(OBJECTS)
	ld $(LDFLAGS) -T hos/kernel/32.ld -o $@ $(OBJECTS)
clean:
	rm -f $(shell find . -iname '*.o') hos.bin hos.iso
hos/kernel/config.h: SETTINGS
	. ./SETTINGS; kernel_config > hos/kernel/config.h
## -------------------------------------------------------------------
hos.iso: hos.bin
	sh -e ./run/grub-h-mkiso -n "HOS - Harka's Operating System" -o $@ $<
test: hos.iso
	sh -e ./run/test
