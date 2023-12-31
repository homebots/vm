# Espresso Machine

Virtual Machine to execute programs on an esp8266 chip.

## Firmware

See [/vm](vm/) for firmware sources. Go to [releases](https://github.com/homebots/vm/releases) to download pre-built binaries.

## Compiler

See [/compiler](compiler/) for a script to convert ESP code into an executable binary.

Command example:

```sh
esptool.py write_flash --compress --flash_freq 80m -fm qio -fs 1MB 0x10000 0x10000.bin
```

After flashing, the ESP8266 will start in both Access Point and Wifi mode.
The AP mode creates an open WIFI called `Homebots_AP`, for local testing.
The VM opens an HTTP server at port 3000. Connect to this network and use `http://192.168.4.1:3000` to interact with the machine.

### From source

This is the preferred way to use the VM, as you probably want to change the WIFI SSID and password for your devices.

```bash
export WIFI_SSID="My_Network"
export WIFI_PASSWORD="SecretCode123"

make && make flash
```

See the [`Makefile`](Makefile) for more details on how each command works.

## Documentation

See [docs folder](docs/) for instructions, concepts and examples.
