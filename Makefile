FLASH_SPEED     = 230400
ESP_PORT        ?= $$(ls /dev/tty*usbserial*)
DOCKER_IMAGE    ?= ghcr.io/homebots/xtensa-gcc:latest

.PHONY: build flash asm sym test

build:
	mkdir -p build/ firmware/
	chmod a+w firmware build
	docker run --rm -e VERBOSE -e LIBS -e WIFI_SSID -e WIFI_PASSWORD -v$$(pwd):/home/project:rw $(DOCKER_IMAGE) make

flash:
	esptool.py --after hard_reset --baud $(FLASH_SPEED) --port $(ESP_PORT) write_flash --compress --flash_freq 80m -fm qio -fs 1MB 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin

test:
	mkdir -p bin
	clang++ -std=gnu++11 -Wno-int-to-void-pointer-cast -Wno-deprecated-declarations -I src/include test/test.cpp -o bin/vm
	bin/vm examples/hello.bin

asm:
	docker run --rm -v$$(pwd)/:/home/project $(DOCKER_IMAGE) make disassemble

sym:
	docker run --rm -v$$(pwd)/:/home/project $(DOCKER_IMAGE) make symboldump

inspect:
	docker run --rm -it --entrypoint=/bin/bash -v$$(pwd):/home/project $(DOCKER_IMAGE)
