### AVR library

This is my AVR library I've written over the years working on different projects.

The library is designed to be configured and compiled for each project individually.
To achieve this, each project using it must define two files: hwdefs.h and swdefs.h.
Library modules include either none, one or both of these files.

*hwdefs.h* is used to define the hardware layout of your project (pin mapping)
for modules that need one. At minimal hwdefs.h must define DDR and PIN macros like this:

```c
#define DDR(x) (*(&x - 1))
#define PIN(x) (*(&x - 2))
```

*swdefs.h* is used to define software options, like which functions to include.

Module|Description|hwdefs.h|swdefs.h
------|-----------|--------|--------
adc          | ADC peripheral | | ADC_AVG_SAMP
circbuf8     | Circular byte buffer | |
cmt          | Cooperative multitasking | | CMT_NEED_MINSP, CMT_MUTEX_FUNC
i2c          | I2C peripheral | | I2C_USE_CMT
lcd          | HD44780 high level routines. Requires exactly one low level implementation | | LCD_WIDTH, LCD_HEIGHT, LCD_USE_FB, LCD_NEED_func
lcd_io       | HD44780 low level (IO pins) | LCD IO pin map |
lcd_pcf8574  | HD44780 low level (PCF8574) | PCF LCD pin map | LCD_I2C_SPEED
rtc.h        | RTC routines common header. Requires exactly one rtc implementation. | |
rtc_mcp79410 | RTC impl. with MCP79410 | |
rtc_ds3231   | RTC impl. with DS3231 | |
rtc_timer2   | RTC impl. with Timer2 | |
serque       | UART peripheral | |
spi          | SPI peripheral | | SPI_USE_CMT
time         | Time routines | |
