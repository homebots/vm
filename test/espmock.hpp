#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define NUMBER_OF_PINS 4
#define MOVE_TO_FLASH
#define MAX_DELAY 6871000

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long long uint64;

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

#define os_mem_read intnoop
#define os_mem_write noop
#define os_memcpy memcpy

#define os_free free
#define os_realloc realloc

void *os_zalloc(int size)
{
  void *p = malloc(size);
  memset(p, 0, size);
  return p;
}

#define os_printf ::printf
#define os_sprintf sprintf
#define os_strlen strlen
#define os_time intnoop
#define os_restart noop
#define os_freeHeapSize intnoop
#define os_memset memset

typedef void timerCallback(void *arg);

typedef struct
{
  int delay;
  timerCallback *fn;
  void *arg;
} Timer;

void os_timer_setfn(Timer *timer, timerCallback *fn, void *arg)
{
  timer->fn = fn;
  timer->arg = arg;
}

void os_timer_arm(Timer *timer, unsigned int delay, int zero)
{
  usleep((unsigned long)(delay * 1000));
  timer->fn(timer->arg);
}

void os_timer_disarm(Timer *timer)
{
}

void os_sleep(uint64 time)
{
  printf("sleep %d\n", (int)time);
  usleep(time);
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
