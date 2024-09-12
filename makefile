# project config
BUILD_DIR := build
EXECUTABLE := main
COMPILER=avr-gcc
OBJ_COPY=avr-objcopy
MMCU=atmega1284p
SERIAL_PORT=/dev/$(shell ls /dev | grep tty.usb)

DEBUG=0
# compiler config
FLAGS=-Wall -Os -DF_CPU=16000000UL -DDEBUG=$(DEBUG) -mmcu=$(MMCU) -c

# source files
DRIVERS := drivers/led.c drivers/spidev.c drivers/sdcard.c
FILESYSTEM := filesystem/diskio.c filesystem/pff.c
FILES := main.c commands.c $(DRIVERS) $(FILESYSTEM)
OBJECTS := $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(FILES)))

.PHONY: $(EXECUTABLE)
$(EXECUTABLE): $(BUILD_DIR)/$(EXECUTABLE)

.PRECIOUS: $(BUILD_DIR)/. $(BUILD_DIR)%/.

$(BUILD_DIR)/.:
	mkdir -p $@

$(BUILD_DIR)%/.:
	mkdir -p $@

.SECONDEXPANSION:

$(BUILD_DIR)/%.o: src/%.c | $$(@D)/.
	$(COMPILER) $(FLAGS) $< -o $@

$(BUILD_DIR)/$(EXECUTABLE): $(OBJECTS)
	@echo "building project for $(MMCU)..."
	$(COMPILER) -mmcu=$(MMCU) $^ -o ./$(BUILD_DIR)/$(EXECUTABLE)
	@echo "building elf file..."
	$(OBJ_COPY) -O elf32-avr -R .eeprom ./$(BUILD_DIR)/$(EXECUTABLE) ./$(BUILD_DIR)/$(EXECUTABLE).elf
	@echo "done. info:	"
	@avr-size ./build/main.elf

flash:
	@echo "flashing $(MMCU) using port $(SERIAL_PORT)..."
	avrdude -p m1284p -c arduino -P $(SERIAL_PORT) -b 57600 -U flash:w:./$(BUILD_DIR)/$(EXECUTABLE).elf
	@echo "done."

new:
	@make clean
	@make

all: 
	@make clean 
	@make

debug:
	@make clean 
	@make DEBUG=1

clean:
	@echo "cleaning..."
	@rm -rf build
	@mkdir build
	@echo "done."