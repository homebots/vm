// system instructions [0x01..0x1f]
#define op_noop 0x01
#define op_halt 0x02
#define op_restart 0x03
#define op_systeminfo 0x04
#define op_debug 0x05
#define op_dump 0x06
#define op_yield 0x07
#define op_delay 0x08
#define op_print 0x09
#define op_jumpto 0x0a
#define op_jumpif 0x0b
#define op_sleep 0x0c
#define op_return 0x0d

// operators [0x20..0x3f]
// binary operations
#define op_gt 0x20
#define op_gte 0x21
#define op_lt 0x22
#define op_lte 0x23
#define op_equal 0x24
#define op_notequal 0x25
#define op_xor 0x26
#define op_and 0x27
#define op_or 0x28
#define op_add 0x29
#define op_sub 0x2a
#define op_mul 0x2b
#define op_div 0x2c
#define op_mod 0x2d

// unary operations
#define op_not 0x2e
#define op_inc 0x2f
#define op_dec 0x30

#define op_assign 0x31
#define op_declare 0x32
#define op_define 0x33

// memory/io instructions [0x40..0x5f]
#define op_memget 0x40
#define op_memset 0x41

#define op_iowrite 0x43
#define op_ioread 0x44
#define op_iomode 0x45
#define op_iotype 0x46
#define op_ioallout 0x47
#define op_iointerrupt 0x48
#define op_iointerruptToggle 0x49
#define op_ioallinput 0x50

// wifi [0x60..0x6f]
#define op_wifistatus 0x60
#define op_wifiap 0x61
#define op_wificonnect 0x62
#define op_wifidisconnect 0x63
#define op_wifilist 0x64

// protocols [0x70..0x8f]
#define op_i2csetup 0x70
#define op_i2cstart 0x71
#define op_i2cstop 0x72
#define op_i2cwrite 0x73
#define op_i2cread 0x74
#define op_i2csetack 0x75
#define op_i2cgetack 0x76
#define op_i2cfind 0x77
#define op_i2cwriteack 0x78
#define op_i2cwriteack_b 0x79
