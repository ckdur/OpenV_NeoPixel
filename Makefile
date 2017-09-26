ASSEMBLY_OBJS = start.o custom_ops.o
C_OBJS = OpenV_NeoPixel.o
CPP_OBJS = 
INO_OBJS = 
INCLUDE_OPT = 
TOOLCHAIN_PREFIX = riscv32-unknown-elf-

simple.bin: simple.elf
	$(TOOLCHAIN_PREFIX)objcopy -O binary $< $@
	$(TOOLCHAIN_PREFIX)objdump -d $<
	chmod -x $@

simple.elf: $(ASSEMBLY_OBJS) $(C_OBJS) $(CPP_OBJS) $(INO_OBJS) sections.lds
	$(TOOLCHAIN_PREFIX)gcc -Os -ffreestanding -nostdlib -o $@ \
		-Wl,-Bstatic,-T,sections.lds,-Map,input.map,--strip-debug \
		$(ASSEMBLY_OBJS) $(C_OBJS) $(CPP_OBJS) $(INO_OBJS) -lgcc -lm
	chmod -x $@

#%.o: %.ino
#	$(TOOLCHAIN_PREFIX)g++ -c $(CFLAGS) -o $@ $(INCLUDE_OPT) $(DEFS) $<

%.o: %.cpp
	$(TOOLCHAIN_PREFIX)g++ -c $(CFLAGS) -o $@ $(INCLUDE_OPT) $(DEFS) $<

%.o: %.c
	$(TOOLCHAIN_PREFIX)gcc $(INCLUDE_OPT) -c -march=rv32i --std=c99 -nostdlib $< -o $@
	
%.o: %.S
	$(TOOLCHAIN_PREFIX)gcc $(INCLUDE_OPT) -c $< -o $@

clean:
	rm -vrf *.elf *.bin *.o *.d *.map $(ASSEMBLY_OBJS) $(C_OBJS) $(CPP_OBJS) $(INO_OBJS)

.PHONY: simple.bin
