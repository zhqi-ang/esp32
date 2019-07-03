
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "cJSON.h"

#include "wifi_smartconfig.h"
#include "http_weather.h"

static const char *TAG = "http";

/*
===========================
宏定义
=========================== 
*/
#define errno		(*__errno())

//http://t.weather.sojson.com/api/weather/city/101280701

//http组包宏，获取天气的http接口参数
#define WEB_SERVER          "t.weather.sojson.com"              
#define WEB_PORT            "80"
#define WEB_URL             "/api/weather/city/101280701"     

/*
===========================
全局变量定义
=========================== 
*/
//http请求包
static const char *REQUEST = "GET "WEB_URL" HTTP/1.1\r\n"
    "Host: "WEB_SERVER"\r\n"
    "\r\n";


//天气解析结构体
typedef struct 
{
    char cit[20];
    char weather_text[20];
    char weather_code[2];
    char temperatur[3];
}weather_info;

weather_info weather;


/*解析json数据 只处理 解析 城市 天气 天气代码  温度  其他的自行扩展
* @param[in]   text  		       :json字符串
* @retval      void                 :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/10, 初始化版本\n 
*/
void cjson_to_struct_info(char *text)
{
    cJSON *root,*psub;
    //截取有效json
    char *index=strchr(text,'{');
    strcpy(text,index);

    root = cJSON_Parse(text);
    
    if(root != NULL)
    {
        psub = cJSON_GetObjectItem(root, "time");
		ESP_LOGE(TAG,"localtime: %s",psub->valuestring);
		
		cJSON *cityInfo  = cJSON_GetObjectItem(root, "cityInfo");
		if(cityInfo != NULL)
		{
			psub = cJSON_GetObjectItem(cityInfo, "city");
			ESP_LOGE(TAG,"city: %s",psub->valuestring);
			psub = cJSON_GetObjectItem(cityInfo, "parent");
			ESP_LOGE(TAG,"parent: %s",psub->valuestring);
			psub = cJSON_GetObjectItem(cityInfo, "updateTime");
			ESP_LOGE(TAG,"updateTime: %s",psub->valuestring);
		}
		
		psub = cJSON_GetObjectItem(root, "date");
		ESP_LOGE(TAG,"date: %s",psub->valuestring);
		
		cJSON *data  = cJSON_GetObjectItem(root, "data");
		if(data != NULL)
		{
			psub = cJSON_GetObjectItem(data, "shidu");
			ESP_LOGE(TAG,"shidu: %s",psub->valuestring);
			psub = cJSON_GetObjectItem(data, "pm25");
			ESP_LOGE(TAG,"pm25: %4.2f",psub->valuedouble);
			psub = cJSON_GetObjectItem(data, "pm10");
			ESP_LOGE(TAG,"pm10: %4.2f",psub->valuedouble);
			psub = cJSON_GetObjectItem(data, "quality");
			ESP_LOGE(TAG,"quality: %s",psub->valuestring);
			psub = cJSON_GetObjectItem(data, "wendu");
			ESP_LOGE(TAG,"wendu: %s",psub->valuestring);
		}		
		
    }
    //ESP_LOGE(HTTP_TAG,"%s 222",__func__);
    cJSON_Delete(root);
}


void http_get_task(void *pvParameters)
{
	wifi_wait_connected();
	ESP_LOGI(TAG, "Connected to AP, begin http example");
	
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[1024];
	
    char mid_buf[1024*6];
	//清缓存
    memset(mid_buf,0,sizeof(mid_buf));
	
    while(1) {
        
        //DNS域名解析
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);
        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p\r\n", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        //打印获取的IP
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s\r\n", inet_ntoa(*addr));

        //新建socket
        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.\r\n");
            close(s);
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        //连接ip
        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d\r\n", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        freeaddrinfo(res);
        //发送http包
        ESP_LOGI(TAG,"REQUEST:%s",REQUEST);
        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed\r\n");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        //获取http应答包
        do {
			memset(recv_buf, 0, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
			strcat(mid_buf,recv_buf);
        } while(r > 0);
        //json解析
        cjson_to_struct_info(mid_buf);
        //关闭socket，http是短连接
        close(s);

		
		while(1)
        //延时一会
        for(int countdown = 10; countdown >= 0; countdown--) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}


