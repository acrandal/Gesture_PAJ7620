SOURCE_FILE=paj7620_cursor_mode

PORT=/dev/ttyUSB0

DEVICE=arduino:avr:uno

build:
	arduino-cli compile --fqbn $(DEVICE) $(SOURCE_FILE)

upload:
	arduino-cli upload --port $(PORT) --fqbn $(DEVICE) $(SOURCE_FILE).ino

mon:
	./serial_mon.py

list:
	arduino-cli board list
