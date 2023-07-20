# project config
BUILD_DIR := build
EXECUTABLE := main
COMPILER=avr-gcc
OBJ_COPY=avr-objcopy
MMCU=atmega1284p
SERIAL_PORT=/dev/tty.usbserial-14120

# compiler config
COMPILER := avr-gcc 
FLAGS=-Wall -Os -DF_CPU=16000000UL -mmcu=$(MMCU) -c

# source files
FILES := main.c
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
	@echo "done."

flash:
	@echo "flashing using port $(SERIAL_PORT)..."
	avrdude -p m1284p -c arduino -P $(SERIAL_PORT) -b 57600 -U flash:w:./$(BUILD_DIR)/$(EXECUTABLE).elf
	@echo "done."

new:
	make clean
	make

clean:
	rm -rf build
	mkdir build
