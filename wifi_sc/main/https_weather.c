
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "cJSON.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"


#include "wifi_smartconfig.h"
#include "https_weather.h"

static const char *TAG = "https";
//https://www.sojson.com/open/api/lunar/json.shtml
/* Constants that aren't configurable in menuconfig */
/*#define WEB_SERVER "www.sojson.com"
#define WEB_PORT "443"
#define WEB_URL "/open/api/lunar/json.shtml"
*/

//"https://api.seniverse.com/v3/weather/daily.json?key=4nik0ivxfmxfjzz1&location=beijing&language=zh-Hans&unit=c&start=0&days=5"
#define WEB_SERVER "api.seniverse.com"
#define WEB_PORT "443"
#define WEB_URL "/v3/weather/daily.json?key=4nik0ivxfmxfjzz1&location=beijing&language=zh-Hans&unit=c&start=0&days=5"



static const char *REQUEST = "GET " WEB_URL " HTTP/1.1\r\n"
    "Host: "WEB_SERVER"\r\n"
    "\r\n";




void https_get_task(void *pvParameters)
{

	int ret;

	char recv_buf[1024];
		
	
	mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    mbedtls_ssl_init(&ssl);
    mbedtls_ctr_drbg_init(&ctr_drbg);

	ESP_LOGI(TAG, "Seeding the random number generator");

    mbedtls_ssl_config_init(&conf);

    mbedtls_entropy_init(&entropy);
    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                    NULL, 0)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        abort();
    }
								
	ESP_LOGI(TAG, "Setting hostname for TLS session...");
	
	/* Hostname set here should match CN in server certificate */
   if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
   {
	   ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
	   abort();
   }

   ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");
   /*
	  * Setup SSL/TLS相关参数
	  */
   if((ret = mbedtls_ssl_config_defaults(&conf,
										 MBEDTLS_SSL_IS_CLIENT,
										 MBEDTLS_SSL_TRANSPORT_STREAM,
										 MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
   {
	   ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
	   goto exit;
   }

	/* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
	   a warning if CA verification fails but it will continue to connect.

	   You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
	*/
	/* 由于证书会过期,所以这些不进行证书认证 */
	mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
	/* 设置随机数生成的函数及方法 */
	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
	
#ifdef CONFIG_MBEDTLS_DEBUG
	/* 这里不需要设置调试函数 */
	mbedtls_esp_enable_debug_log(&conf, 4);
#endif

	/* 将ssl_conf的相关信息填充于ssl_ctx中去,用于进行SSL握手时使用 */
	if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
	{
		ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
		goto exit;
	}

	wifi_wait_connected();
	ESP_LOGI(TAG, "Connected to AP, begin https example");

	while(1)
	{
		//初始化fd
		mbedtls_net_init(&server_fd);
	
		ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);
		//进行TCP连接
		if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
									 WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
		{
		   ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
		   goto exit;
		}

		ESP_LOGI(TAG, "Connected.");
									 
		// 设置发送以及接收的时候,调用的内部函数
		mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

		ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");
		//握手
		while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
		{
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
			{
				ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
				goto exit;
			}
		}
		
		ESP_LOGI(TAG, "Writing HTTP request...");
		size_t written_bytes = 0;
		do {
			ret = mbedtls_ssl_write(&ssl,
									(const unsigned char *)REQUEST + written_bytes,
									strlen(REQUEST) - written_bytes);
			if (ret >= 0) {
				ESP_LOGI(TAG, "%d bytes written", ret);
				written_bytes += ret;
			} else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
				ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
				goto exit;
			}
		} while(written_bytes < strlen(REQUEST));

		ESP_LOGI(TAG, "Reading HTTP response...");

		do
		{
			memset(recv_buf, 0, sizeof(recv_buf));
			ret = mbedtls_ssl_read(&ssl, (unsigned char *)recv_buf, sizeof(recv_buf)-1);

			if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
				continue;

			if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
				ret = 0;
				break;
			}

			if(ret < 0)
			{
				ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
				break;
			}

			if(ret == 0)
			{
				ESP_LOGI(TAG, "connection closed");
				break;
			}

			ESP_LOGE(TAG,"date: %s",recv_buf);

		} while(1);

		mbedtls_ssl_close_notify(&ssl);


    exit:
        mbedtls_ssl_session_reset(&ssl);
        mbedtls_net_free(&server_fd);

        if(ret != 0)
        {
            mbedtls_strerror(ret, recv_buf, 100);
            ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, recv_buf);
        }

        putchar('\n'); // JSON output doesn't have a newline at end

        static int request_count;
        ESP_LOGI(TAG, "Completed %d requests", ++request_count);

        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");

	}


}



