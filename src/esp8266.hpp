#ifdef __CHIP_ESP8266__

#define MAX_DELAY 6871000
#define NUMBER_OF_PINS 4
#include "sdk.h"

#define Timer os_timer_t
static Wifi wifi;

#define os_restart system_restart
#define os_io_read pinRead
#define os_io_write pinWrite
#define os_io_type pinType
#define os_io_mode(pin, mode) pinMode(pin, (PinMode)(mode))
#define os_io_enableInterrupts armInterrupts
#define os_io_disableInterrupts disarmInterrupts

void os_io_interrupt(uint8_t pin, void *callback, void *arg, uint8_t mode)
{
  attachPinInterrupt((uint8_t)pin, (interruptCallbackHandler)callback, (void *)arg, (GPIO_INT_TYPE)mode);
}

#define os_i2c_setup i2c_setup
#define os_i2c_start i2c_start
#define os_i2c_stop i2c_stop
#define os_i2c_writeByteAndAck i2c_writeByteAndAck
#define os_i2c_findDevice i2c_findDevice
#define os_i2c_readByte i2c_readByte

#define os_enableSerial system_uart_de_swap
#define os_disableSerial system_uart_swap
#define os_time system_get_time
#define os_freeHeapSize system_get_free_heap_size

#define os_mem_read READ_PERI_REG
#define os_mem_write WRITE_PERI_REG

void os_sleep(uint64_t time)
{
  system_deep_sleep_set_option(2);
  system_deep_sleep((uint64_t)time);
}

void os_io_allOutput()
{
  pinType(0, 0);
  pinType(1, 3);
  pinType(2, 0);
  pinType(3, 3);
  pinMode(0, PinOutput);
  pinMode(1, PinOutput);
  pinMode(2, PinOutput);
  pinMode(3, PinOutput);
}

void os_io_allInput()
{
  pinType(0, 0);
  pinType(1, 3);
  pinType(2, 0);
  pinType(3, 3);
  pinMode(0, PinInput);
  pinMode(1, PinInput);
  pinMode(2, PinInput);
  pinMode(3, PinInput);
}

void os_wifi_ap()
{
  wifi.startAccessPoint();
}

void os_wifi_connect(const char *ssid, const char *password)
{
  wifi.connectTo(ssid, password);
}

void os_wifi_disconnect()
{
  wifi.disconnect();
}

#endif