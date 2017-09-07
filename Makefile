ASSEMBLY_OBJS = start.o custom_ops.o
C_OBJS = OpenV_NeoPixel.o 
CPP_OBJS = 
INO_OBJS = 
GCC_WARNS  = -Werror -Wall -Wextra -Wshadow -Wundef -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings
GCC_WARNS += -Wredundant-decls -Wstrict-prototypes -Wmissing-prototypes -pedantic # -Wconversion
INCLUDE_OPT = -I./ -I./arduinoReady
DEFS = -DARDUINO=105 -DF_CPU=25000000 -D__RISCV_OPENV__
TOOLCHAIN_PREFIX = riscv32-unknown-elf-
CFLAGS = -march=rv32im 
CFLAGS +=-Os -ffunction-sections -fdata-sections -MD 
LDFLAGS = --data-sections

simple.bin: simple.elf
	$(TOOLCHAIN_PREFIX)objcopy -O binary $< $@
	$(TOOLCHAIN_PREFIX)objdump -d $<
	chmod -x $@

simple.elf: $(ASSEMBLY_OBJS) $(C_OBJS) $(CPP_OBJS) $(INO_OBJS) sections.lds
	$(TOOLCHAIN_PREFIX)gcc -Os -march=rv32im -ffreestanding -nostdlib -o $@ \
		-Wl,-Bstatic,-T,sections.lds,-Map,input.map,--strip-debug $(LDFLAGS) \
		$(ASSEMBLY_OBJS) $(C_OBJS) $(CPP_OBJS) $(INO_OBJS) -lgcc -lc -lstdc++
	chmod -x $@

#%.o: %.ino
#	$(TOOLCHAIN_PREFIX)g++ -c $(CFLAGS) -o $@ $(INCLUDE_OPT) $(DEFS) $<

%.o: %.cpp
	$(TOOLCHAIN_PREFIX)g++ -c $(CFLAGS) -o $@ $(INCLUDE_OPT) $(DEFS) $<

%.o: %.c
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) -o $@ $(INCLUDE_OPT) $(DEFS) $<
	
%.o: %.S
	$(TOOLCHAIN_PREFIX)gcc -c $(CFLAGS) -o $@ $(INCLUDE_OPT) $(DEFS) $<

clean:
	rm -vrf *.elf *.bin *.o *.map $(ASSEMBLY_OBJS) $(C_OBJS) $(CPP_OBJS) $(INO_OBJS)

.PHONY: simple.bin
