# Examples

## Blink LED at pin 2

```
io_allout
pin_mode #2, output

def loop:
    io_write #2, 0x00
    delay 1000
    io_write #2, 0x01
    delay 1000
    jump to @loop
end
```

## Blink pin 2 twice when pin 0 is triggered

```
io_allout
pin_mode #2, output
pin_mode #0, input

def blink:
    io_write #2, off
    delay 1000
    io_write #2, on
    delay 1000
    io_interruptToggle on
end

def onpush:
    io_interrupt #0, 0x05, @blink
    io_interruptToggle
end
```