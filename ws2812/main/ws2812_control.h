#ifndef WS2812_CONTROL_H
#define WS2812_CONTROL_H
#include <stdint.h>
#include "driver/rmt.h"
#include "driver/gpio.h"

#ifndef LEDS_NUM
#define LEDS_NUM 24
#endif

#define LED_RMT_TX_CHANNEL RMT_CHANNEL_0

// This structure is used for indicating what the colors of each LED should be set to.
// There is a 32bit value for each LED. Only the lower 3 bytes are used and they hold the
// Red (byte 2), Green (byte 1), and Blue (byte 0) values to be set.

typedef union {

  struct __attribute__ ((packed)) {
    uint8_t r, g, b;
  };
  uint32_t rgb;
} rgb_color;


rgb_color set_LedRGB(uint8_t r, uint8_t g, uint8_t b);

// Setup the hardware peripheral. Only call this once.
void ws2812_control_init(gpio_num_t gpio_num);

// Update the LEDs to the new state. Call as needed.
// This function will block the current task until the RMT peripheral is finished sending 
// the entire sequence.
void ws2812_write_leds(rgb_color *led_color);

#endif