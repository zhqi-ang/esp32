#include "ws2812_control.h"



#define BITS_PER_LED_CMD 24 
#define LED_BUFFER_ITEMS ((LEDS_NUM * BITS_PER_LED_CMD))

#define RMT_CLK_DIV      2    /*!< RMT counter clock divider */


//#define RMT_TICK_1_NS    (1/(80000000/RMT_CLK_DIV))   /*!< RMT counter value for 10 us.(Source clock is APB clock) */
// APB_CLK 80MHz, clock_cycle = 1/80/RMT_CLK_DIV = 25ns
//
//#define T0H 	350ns/RMT_TICK_1_NS = 14
//#define T1H 	700ns/RMT_TICK_1_NS = 28
//#define T0L  	800ns/RMT_TICK_1_NS = 32
//#define T1L  	600ns/RMT_TICK_1_NS = 24

// These values are determined by measuring pulse timing with logic analyzer and adjusting to match datasheet. 
#define T0H 	14  // 0 bit high 		
#define T1H 	28  // 1 bit high time
#define T0L  	32  // low time for either bit
#define T1L  	24  // low time for either bit


// This is the buffer which the hw peripheral will access while pulsing the output pin
rmt_item32_t led_data_buffer[LED_BUFFER_ITEMS];


rgb_color set_LedRGB(uint8_t r, uint8_t g, uint8_t b)
{
  rgb_color v;

  v.r = r;
  v.g = g;
  v.b = b;
  return v;
}


void ws2812_control_init(gpio_num_t gpio_num)
{
  rmt_config_t config;
  config.rmt_mode = RMT_MODE_TX;		//发送
  config.channel = LED_RMT_TX_CHANNEL;	//通道
  config.gpio_num = gpio_num;			//管脚
  config.mem_block_num = 3;				//3*8bit；rgb
  config.tx_config.loop_en = false;
  config.tx_config.carrier_en = false;
  config.tx_config.idle_output_en = true;
  config.tx_config.idle_level = 0;
  config.clk_div = RMT_CLK_DIV;			//时钟分频


  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}



void setup_rmt_data_buffer(rgb_color *led_color) 
{
  for (uint32_t led = 0; led < LEDS_NUM; led++) {
    uint32_t bits_to_send = led_color[led].rgb;
    uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);
    for (uint32_t bit = 0; bit < BITS_PER_LED_CMD; bit++) {
      uint32_t bit_is_set = bits_to_send & mask;
      led_data_buffer[led * BITS_PER_LED_CMD + bit] = bit_is_set ?
                                                      (rmt_item32_t){{{T1H, 1, T0L, 0}}} : 
                                                      (rmt_item32_t){{{T0H, 1, T1L, 0}}};
      mask >>= 1;
    }
  }
}

void ws2812_write_leds(rgb_color *led_color) {
  setup_rmt_data_buffer(led_color);
  ESP_ERROR_CHECK(rmt_write_items(LED_RMT_TX_CHANNEL, led_data_buffer, LED_BUFFER_ITEMS, false));
  ESP_ERROR_CHECK(rmt_wait_tx_done(LED_RMT_TX_CHANNEL, portMAX_DELAY));
}


