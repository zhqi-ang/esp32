/* Created 19 Nov 2016 by Chris Osborn <fozztexx@fozztexx.com>
 * http://insentricity.com
 *
 * Demo of driving WS2812 RGB LEDs using the RMT peripheral.
 *
 * This code is placed in the public domain (or CC0 licensed, at your option).
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <soc/rmt_struct.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <stdio.h>
#include "ws2812_control.h"


#define delay_ms(ms) vTaskDelay((ms) / portTICK_RATE_MS)



void rainbow(void *pvParameters)
{
	const uint8_t anim_step = 10;
	const uint8_t anim_max = 240;

	rgb_color color = set_LedRGB(anim_max, 0, 0);
	uint8_t step = 0;
	rgb_color color2 = set_LedRGB(anim_max, 0, 0);
	uint8_t step2 = 0;
	rgb_color *pixels;

	pixels = malloc(sizeof(rgb_color) * LEDS_NUM);

	while (1) {
	color = color2;
	step = step2;

		for (uint8_t i = 0; i < LEDS_NUM; i++) {
		  pixels[i] = color;

		  if (i == 1) {
		    color2 = color;
		    step2 = step;
		  }

		  switch (step) {
		  case 0:
		    color.g += anim_step;
		    if (color.g >= anim_max)
		      step++;
		    break;
		  case 1:
		    color.r -= anim_step;
		    if (color.r == 0)
		      step++;
		    break;
		  case 2:
		    color.b += anim_step;
		    if (color.b >= anim_max)
		      step++;
		    break;
		  case 3:
		    color.g -= anim_step;
		    if (color.g == 0)
		      step++;
		    break;
		  case 4:
		    color.r += anim_step;
		    if (color.r >= anim_max)
		      step++;
		    break;
		  case 5:
		    color.b -= anim_step;
		    if (color.b == 0)
		      step = 0;
		    break;
		  }
		}


		ws2812_write_leds(pixels);

		delay_ms(100);
	}
}

void app_main()
{
  nvs_flash_init();

  ws2812_control_init(GPIO_NUM_18);
  xTaskCreate(rainbow, "ws2812 rainbow demo", 4096, NULL, 10, NULL);

  return;
}
