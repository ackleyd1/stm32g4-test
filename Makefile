export PATH := /usr/local/gcc-arm-none-eabi/bin:$(PATH)
export LD_LIBRARY_PATH := /usr/local/gcc-arm-none-eabi/lib:$(LD_LIBRARY_PATH)

PROJ=test
TARGET_ARCH=-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
CC=arm-none-eabi-gcc
OC=arm-none-eabi-objcopy
NM=arm-none-eabi-nm
CFLAGS= -Wall


OBJS=startup_stm32g431xx.o main.o system_stm32g4xx.o
AS=arm-none-eabi-as
LD=arm-none-eabi-ld
ASFLAGS=
LDFLAGS=
LDLIBS=

vpath %.c src
vpath %.c ext/st/src
vpath %.s ext/st/src

.PHONY: all
all: build/$(PROJ).bin

build/%.o: %.s
ifeq ("$(wildcard build)", "")
	@echo "creating build directory"
	@mkdir build
endif
	$(CC) -c -O0 -g3 -Wall $(TARGET_ARCH) --specs=nosys.specs -o $@ $<

build/%.o: %.c
	$(CC) -c -O0 -g3 -Wall $(TARGET_ARCH) --specs=nosys.specs -nostdlib -lgcc -I./inc -I./ext/st/inc -I./ext/cmsis/inc -o $@ $<

build/$(PROJ).elf: $(addprefix build/,$(OBJS))
	$(CC) $^ $(TARGET_ARCH) --specs=nosys.specs -nostdlib -lgcc -T ld/linker.ld -Wl,-Map build/test.map -o $@

build/$(PROJ).bin: build/$(PROJ).elf
	$(NM) $< > build/$(PROJ).sym
	$(OC) -S -O binary $< $@

.PHONY: flash
flash: build/$(PROJ).bin
	st-flash write $< 0x8000000

.PHONY: clean
clean:
ifeq ("$(wildcard build)", "")
	@echo "no build to clean"
endif
	rm -f $(addprefix build/,$(OBJS))
	rm -f build/$(PROJ).elf
	rm -f build/$(PROJ).bin