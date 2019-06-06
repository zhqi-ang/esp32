
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

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "www.tianqiapi.com"
#define WEB_PORT "443"
#define WEB_URL "https://www.tianqiapi.com/api/?version=v1"

static const char *REQUEST = "GET " WEB_URL " HTTP/1.1\r\n"
    "Host: "WEB_SERVER"\r\n"
    "\r\n";




void https_get_task(void *pvParameters)
{


}


