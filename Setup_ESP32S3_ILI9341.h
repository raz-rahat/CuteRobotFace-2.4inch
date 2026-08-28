// =====================================================================
//  TFT_eSPI setup  ->  ESP32-S3-N16R8 + 2.8" ILI9341 (240x320) + XPT2046
//  Copy this file into:  Arduino/libraries/TFT_eSPI/User_Setups/
//  Then in TFT_eSPI/User_Setup_Select.h :
//      - comment out  #include <User_Setup.h>
//      - add          #include <User_Setups/Setup_ESP32S3_ILI9341.h>
// =====================================================================

#define USER_SETUP_ID 900

#define ILI9341_DRIVER
// If colours look inverted / washed out, use ILI9341_2_DRIVER instead:
// #define ILI9341_2_DRIVER

#define TFT_MISO  12      // TFT MISO + Touch T_DO
#define TFT_MOSI  11      // TFT MOSI + Touch T_DIN
#define TFT_SCLK  13      // TFT SCK  + Touch T_CLK
#define TFT_CS    10      // TFT CS
#define TFT_DC     9      // TFT DC
#define TFT_RST    8      // TFT RESET
#define TFT_BL     5      // TFT LED backlight
#define TFT_BACKLIGHT_ON HIGH

#define TOUCH_CS   7      // XPT2046 T_CS  (T_IRQ on GPIO6 is not needed)

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY        40000000   // try 27000000 if the display glitches
#define SPI_READ_FREQUENCY   20000000
#define SPI_TOUCH_FREQUENCY   2500000
