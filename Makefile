FLASH_SPEED   ?= 460800
ESP_PORT      ?= $$(ls /dev/tty*usbserial*)

.PHONY: build flash asm sym

build:
	docker run --rm -e VERBOSE -e LIBS -v$$(pwd):/home/project:rw homebotz/xtensa-gcc make

flash:
	esptool.py --after no_reset --baud $(FLASH_SPEED) --port $(ESP_PORT) write_flash --compress --flash_freq 80m -fm qio -fs 1MB 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin

asm:
	docker run --rm -e VERBOSE -e LIBS -v$$(pwd)/:/home/project homebotz/xtensa-gcc make disassemble

sym:
	docker run --rm -v$$(pwd)/:/home/project homebotz/xtensa-gcc make symboldump

inspect:
	docker run -it --entrypoint=/bin/bash -v$$(pwd):/home/project homebotz/xtensa-gcc