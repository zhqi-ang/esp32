[ESP32 各种时钟参数值设置](https://blog.csdn.net/qinrenzhi/article/details/88601697)
[ws2812驱动总结](https://blog.csdn.net/u013062709/article/details/85217281)

## 感谢两位大佬的奉献：
https://github.com/FozzTexx/ws2812-demo

https://github.com/JSchaenzle/ESP32-NeoPixel-WS2812-RMT
## 头文件：esp-idf/components/soc/esp32/inclued/soc/soc.h
```
//Periheral Clock {{
#define  APB_CLK_FREQ_ROM                            ( 26*1000000 )
#define  CPU_CLK_FREQ_ROM                            APB_CLK_FREQ_ROM
#define  CPU_CLK_FREQ                                APB_CLK_FREQ
#define  APB_CLK_FREQ                                ( 80*1000000 )       //unit: Hz
#define  REF_CLK_FREQ                                ( 1000000 )
#define  UART_CLK_FREQ                               APB_CLK_FREQ
#define  WDT_CLK_FREQ                                APB_CLK_FREQ
#define  TIMER_CLK_FREQ                              (80000000>>4) //80MHz divided by 16
#define  SPI_CLK_DIV                                 4
#define  TICKS_PER_US_ROM                            26              // CPU is 80MHz
#define  GPIO_MATRIX_DELAY_NS                        25
//}}
```
## 以APB_CLK_FREQ为例
### 1、时间周期（时序中最小的时间单位）

> 1 / 80MHz =1/80 us = 0.0125us =12.5 ns
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190619144900368.png)
```
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
```

## RMT配置

```
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
```