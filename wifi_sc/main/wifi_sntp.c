#include <string.h>
#include <stdlib.h>

#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include <time.h>

#include "wifi_smartconfig.h"
#include "wifi_sntp.h"


static const char *TAG = "sntp";


#define NTP_PORT  	123

//1970年的NTP时间戳
#define TIME1970   2208988800


void sntp_get_task(void *pvParameters)
{
	wifi_wait_connected();
	ESP_LOGI(TAG, "Connected to AP, begin sntp example\r\n");

	int socket_fb;
	
	struct sockaddr_in serv_addr;
    
	char *NTP_SERVER = "0.cn.pool.ntp.org";

	uint32_t SIZE_SOCKADDR_IN = sizeof(serv_addr);

	memset(&serv_addr, 0, SIZE_SOCKADDR_IN);

    struct hostent *hp = NULL;
    struct ip4_addr *ip4_addr = NULL;
	char ip[16];

    hp = gethostbyname(NTP_SERVER);

    ip4_addr = (struct ip4_addr *)hp->h_addr;
    strcpy(ip, inet_ntoa(*ip4_addr));

    ESP_LOGE(TAG,"DNS lookup succeeded.name: %s, ip: %s", NTP_SERVER, ip);

	serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(NTP_PORT);
	

	//新建socket,UDP
    socket_fb = socket(AF_INET, SOCK_DGRAM, 0);

	if(socket_fb < 0) {
		ESP_LOGE(TAG, "... Failed to allocate socket.\r\n");
		close(socket_fb);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		return;
	}

	uint8_t data[48]={0};
	int str_lent,ret;
	
	

	while(1)
	{
		memset(data, 0, 48);
		data[0] = 0x1b;
		
		ret = sendto(socket_fb, data, 48, 0, (struct sockaddr *)&serv_addr, SIZE_SOCKADDR_IN);

		do {
			vTaskDelay(10 / portTICK_PERIOD_MS);
			str_lent = recvfrom(socket_fb , data, 48, 0,  (struct sockaddr*)&serv_addr, &SIZE_SOCKADDR_IN);
        } while(str_lent != 48);

		if(str_lent == 48)
		{
			time_t t = 0;
			for(int i = 40; i <= 43; i++)
			{
				uint8_t c = (unsigned char)data[i];
				t = (t<<8) | c;
			}
			
			t -= TIME1970;	// 1970.1.1 0:0:0
			t += 8 * 60 * 60;	// UTC to Beijin
			struct tm now_time;
			localtime_r(&t, &now_time);
 			ESP_LOGE(TAG, "%d-%d-%d %d:%d:%d\n", now_time.tm_year + 1900, now_time.tm_mon + 1,
					now_time.tm_mday, now_time.tm_hour, now_time.tm_min, now_time.tm_sec);
			
		}

		//延时一会
		for(int countdown = 10; countdown >= 0; countdown--) {
			ESP_LOGI(TAG, "%d...", countdown);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}

	}

}


