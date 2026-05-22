OBJECTS = boot/loader.o \
          lib/io.o \
          kernel/kmain.o \
          kernel/gdt.o \
          kernel/interrupt.o \
          kernel/interrupt_handlers.o \
          drivers/pic.o \
          drivers/keyboard.o \
          drivers/serial.o						
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf32

all: generate program kernel.elf

generate:
	python3 generate_idt.py

program:
	nasm -f bin iso/modules/program.s -o iso/modules/program

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

os.iso: kernel.elf program
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R                              \
	            -b boot/grub/stage2_eltorito    \
	            -no-emul-boot                   \
	            -boot-load-size 4               \
	            -A os                           \
	            -input-charset utf8             \
	            -quiet                          \
	            -boot-info-table                \
	            -o os.iso                       \
	            iso

run: os.iso
	qemu-system-i386 -cdrom os.iso -serial file:com1.out

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) -i$(dir $<) $< -o $@

clean:
	rm -rf *.o kernel.elf os.iso iso/modules/program boot/*.o lib/*.o kernel/*.o drivers/*.o