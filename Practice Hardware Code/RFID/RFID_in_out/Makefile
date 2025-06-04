# Makefile for Arduino CLI usage.
# This Makefile provides simple targets to compile, upload, and monitor your
# Arduino sketch, as well as helper targets for core installation and board listing.
#
# Default settings:
#   FQBN              - Fully Qualified Board Name. (Example: esp32:esp32:esp32s3)
#   BOARD_OPTIONS     - https://github.com/arduino/arduino-cli/issues/1869#issuecomment-1240181438
#   PORT              - Serial port for uploading and monitoring.
#   BAUD              - Baud rate for the serial monitor.
#   ARDUINO_CACHE_DIR - Arduino cache directory. Used during make clean.
#   SKETCH_DIR        - Directory containing your sketch (with the .ino file).
#
# Usage example (for more, check `make help`):
#   make cupmon: Compiles, uploads, and then opens the serial monitor.
#
# You can override any variable from the command line. For example:
#   make FQBN=arduino:avr:uno BOARD_OPTIONS=CDCOnBoot=cdc PORT=/dev/ttyACM0 cup
#
# arduino-cli setup guide:
#   - https://arduino.github.io/arduino-cli/latest/installation/
#   - Update the Core index and install a board platform
#     arduino-cli core update-index
#     arduino-cli core install esp32:esp32
#   - Install libraries
#     arduino-cli lib install IRremote
#   - Create a configuration file
#     arduino-cli config init
#     arduino-cli config set board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
#
# https://wiki.archlinux.org/title/arduino
# - Append the following to /etc/udev/rules.d/01-ttyusb.rules
#   ```
#   SUBSYSTEMS=="usb-serial", TAG+="uaccess"
#   ```
# - Append the following to ~/.arduino15/arduino-cli.yaml
#   ```
#   serial:
#     debug_rate: 115200
#   ```
#
# To install my LCD-Remote UI library from git
# ```
# library:
#   enable_unsafe_install: true
# ```

# Default variables (override as needed):
FQBN ?= esp32:esp32:esp32s3
BOARD_OPTIONS ?= CDCOnBoot=cdc  # Required for USB port on esp32s3, remove if you use UART

# PORT ?= /dev/ttyACM0  # USB port
# PORT ?= /dev/ttyUSB0  # UART port
PORT ?= /dev/cu.usbmodem1401 
BAUD ?= 115200
# ARDUINO_CACHE_DIR ?= ~/.cache/arduino/  # MacOS: ~/Library/Caches/arduino
SKETCH_DIR ?= .

ifeq ($(strip $(BOARD_OPTIONS)),)
	FULL_FQBN := $(FQBN)
else
	FULL_FQBN := $(FQBN):$(BOARD_OPTIONS)
endif

.PHONY: all compile upload monitor cup upmon cupmon board-list lib-list version help

# Default target compiles the sketch.
all: compile

# Clean Arduino build cache.
clean:
	arduino-cli cache clean
# rm -rf $(ARDUINO_CACHE_DIR)

# Compile the sketch using the Arduino CLI.
compile:
	arduino-cli compile --fqbn $(FULL_FQBN) $(SKETCH_DIR)

# Upload the compiled sketch to the board.
upload:
	arduino-cli upload --fqbn $(FULL_FQBN) -p $(PORT) $(SKETCH_DIR)

# Open the serial monitor with the correct baud rate.
monitor:
	arduino-cli monitor -p $(PORT) --fqbn $(FULL_FQBN) --config baudrate=$(BAUD)

# Combined target: compile, then upload.
cup: clean compile upload

# Combined target: upload and then open the serial monitor.
upmon: upload monitor

# Combined target: compile, upload, and then open the serial monitor.
cupmon: clean compile upload monitor

# List all connected boards.
board-list:
	arduino-cli board list

# Prints all board options.
board-details:
	arduino-cli board details --fqbn $(FULL_FQBN)

# List all installed libraries.
lib-list:
	arduino-cli lib list

# Display the Arduino CLI version.
version:
	arduino-cli version

# Display help message.
help:
	@echo "Arduino CLI Makefile helpers:"
	@echo "  make clean         - Clean Arduino build cache."
	@echo "  make compile       - Compile the sketch."
	@echo "  make upload        - Upload the sketch."
	@echo "  make monitor       - Open the serial monitor (you probably have to set serial.debug_rate in arduino-config.yaml)."
	@echo "  make cup           - Compile, then upload."
	@echo "  make upmon         - Upload and then open the serial monitor."
	@echo "  make cupmon        - Compile, upload, and open the serial monitor."
	@echo "  make board-list    - List connected Arduino boards."
	@echo "  make board-details - Prints all board options."
	@echo "  make lib-list      - List installed libraries."
	@echo "  make version       - Show Arduino CLI version."
	@echo "  make help          - Show this help message."
