FLASH_SPEED   ?= 460800
ESP_PORT      ?= $$(ls /dev/tty*usbserial*)

.PHONY: build flash asm sym

build:
	docker run --rm -e VERBOSE -e LIBS -v$$(pwd):/home/espbuilder/project darlanalves/homebots-sdk make all

flash:
	esptool.py --baud $(FLASH_SPEED) --port $(ESP_PORT) write_flash --compress -fm qio -fs 512KB 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin 0x7c000 firmware/0x7c000.bin

asm:
	docker run --rm -e VERBOSE -e LIBS -v$$(pwd)/:/home/espbuilder/project darlanalves/homebots-sdk:latest make disassemble

sym:
	docker run --rm -v$$(pwd)/:/home/espbuilder/project darlanalves/homebots-sdk:latest make symboldump

inspect:
	docker run -it --entrypoint=/bin/bash -v$$(pwd):/home/espbuilder/project darlanalves/homebots-sdk