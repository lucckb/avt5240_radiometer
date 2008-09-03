# setup

COMPILE_OPTS = -mcpu=cortex-m3 -mthumb -Wall -g -O0 -std=gnu99
INCLUDE_DIRS = -I . -I lib/inc
LIBRARY_DIRS = -L lib

CC = arm-elf-gcc
CFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS)

CXX = arm-elf-g++
CXXFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS)

AS = arm-elf-gcc
ASFLAGS = $(COMPILE_OPTS) -c

LD = arm-elf-gcc
LDFLAGS = -mcpu=cortex-m3 -mthumb -Wl,--gc-sections,-Map=$@.map,-cref,-u,Reset_Handler $(INCLUDE_DIRS) $(LIBRARY_DIRS) -T stm32.ld

OBJCP = arm-elf-objcopy
OBJCPFLAGS = -O binary

AR = arm-elf-ar
ARFLAGS = cr

MAIN_OUT = main
MAIN_OUT_ELF = $(MAIN_OUT).elf
MAIN_OUT_BIN = $(MAIN_OUT).bin

# all

all: $(MAIN_OUT_ELF) $(MAIN_OUT_BIN)

# main

$(MAIN_OUT_ELF): system.o main.o lcd.o stm32f10x_it.o lib/libstm32.a
	$(LD) $(LDFLAGS) system.o main.o lcd.o stm32f10x_it.o lib/libstm32.a --output $@

$(MAIN_OUT_BIN): $(MAIN_OUT_ELF)
	$(OBJCP) $(OBJCPFLAGS) $< $@


# flash

flash: $(MAIN_OUT)
	@cp $(MAIN_OUT_ELF) jtag/flash
	@cd jtag; openocd -f flash.cfg
	@rm jtag/flash


# libstm32.a

LIBSTM32_OUT = lib/libstm32.a

LIBSTM32_OBJS = \
 lib/src/stm32f10x_adc.o \
 lib/src/stm32f10x_bkp.o \
 lib/src/stm32f10x_can.o \
 lib/src/stm32f10x_dma.o \
 lib/src/stm32f10x_exti.o \
 lib/src/stm32f10x_flash.o \
 lib/src/stm32f10x_gpio.o \
 lib/src/stm32f10x_i2c.o \
 lib/src/stm32f10x_iwdg.o \
 lib/src/stm32f10x_lib.o \
 lib/src/stm32f10x_nvic.o \
 lib/src/stm32f10x_pwr.o \
 lib/src/stm32f10x_rcc.o \
 lib/src/stm32f10x_rtc.o \
 lib/src/stm32f10x_spi.o \
 lib/src/stm32f10x_systick.o \
 lib/src/stm32f10x_tim.o \
 lib/src/stm32f10x_tim1.o \
 lib/src/stm32f10x_usart.o \
 lib/src/stm32f10x_wwdg.o \
 lib/src/cortexm3_macro.o \
 lib/src/stm32f10x_vector.o

$(LIBSTM32_OUT): $(LIBSTM32_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBSTM32_OBJS)

$(LIBSTM32_OBJS): stm32f10x_conf.h


clean:
	-rm *.o lib/src/*.o $(LIBSTM32_OUT) $(MAIN_OUT_ELF) $(MAIN_OUT_BIN)

program:
	openocd -f flash.cfg -c init -c 'script flash.script' -c shutdown || true


