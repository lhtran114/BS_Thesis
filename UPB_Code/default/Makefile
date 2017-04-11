###############################################################################
# Makefile for the project UPB_Atmega8
###############################################################################

## General Flags
PROJECT = UPB_Atmega8
MCU = atmega8
TARGET = UPB_Atmega8.elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=16000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=UPB_Atmega8.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = main.o Button_Input.o Dimmer_Light.o EEPROM.o Fire_Triacs.o Init_Comparator.o Pins_Init.o RX.o Sensor_Input.o Signal_Capturing.o Sync_Reference_Delays.o Timers.o TX.o UART.o Vref.o Zero_Cross.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) UPB_Atmega8.hex UPB_Atmega8.eep UPB_Atmega8.lss size

## Compile
main.o: ../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Button_Input.o: ../Button_Input.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Dimmer_Light.o: ../Dimmer_Light.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

EEPROM.o: ../EEPROM.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Fire_Triacs.o: ../Fire_Triacs.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Init_Comparator.o: ../Init_Comparator.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Pins_Init.o: ../Pins_Init.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

RX.o: ../RX.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Sensor_Input.o: ../Sensor_Input.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Signal_Capturing.o: ../Signal_Capturing.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Sync_Reference_Delays.o: ../Sync_Reference_Delays.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Timers.o: ../Timers.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

TX.o: ../TX.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

UART.o: ../UART.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Vref.o: ../Vref.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Zero_Cross.o: ../Zero_Cross.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) UPB_Atmega8.elf dep/* UPB_Atmega8.hex UPB_Atmega8.eep UPB_Atmega8.lss UPB_Atmega8.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

