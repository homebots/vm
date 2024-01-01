#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define NUMBER_OF_PINS 4
#define MOVE_TO_FLASH
#define MAX_DELAY 128

#define Timer void *
typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long long uint64;

#define os_timer_disarm(...)

int os_io_read(uint8 pin)
{
  return 1;
}

void os_io_write(uint8 pin, bool value)
{
  printf("IO value %d = %d\n", pin, value);
}

void os_io_type(uint8 pin, uint8 value)
{
  printf("IO type %d = %d\n", pin, value);
}

void os_io_mode(uint8 pin, uint8 mode)
{
  printf("IO mode %d = %d\n", pin, mode);
}

void noop(...) {}

uint32 intnoop(...)
{
  return 1;
}

uint8 bytenoop(...)
{
  return 1;
}

#define os_io_interrupt noop
#define os_io_enableInterrupts noop
#define os_io_disableInterrupts noop

#define os_i2c_setup noop
#define os_i2c_start noop
#define os_i2c_stop noop
#define os_i2c_writeByteAndAck noop
#define os_i2c_findDevice bytenoop
#define os_i2c_readByte bytenoop

#define os_enableSerial noop
#define os_disableSerial noop

#define os_timer_arm noop

#define os_mem_read intnoop
#define os_mem_write noop
#define os_memcpy memcpy

#define os_free noop
#define os_realloc realloc
#define os_zalloc malloc

#define os_sprintf sprintf
#define os_strlen strlen
#define os_time intnoop
#define os_timer_setfn noop
#define os_restart noop
#define os_freeHeapSize intnoop

void os_sleep(uint64 time)
{
  printf("sleep %d\n", (int)time);
}

void os_io_allOutput()
{
  printf("All pins to output\n");
}

void os_wifi_ap()
{
  printf("wifi AP mode\n");
}

void os_wifi_connect(const char *ssid, const char *password)
{
  printf("wifi connect %s, %s\n", ssid, password);
}

void os_wifi_disconnect()
{
  printf("wifi disconnect\n");
}
