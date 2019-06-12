# smartconfig Example

This example shows how ESP32 connects to AP with ESPTOUCH. Example does the following steps:

* Download ESPTOUCH APP from app store. [Android source code](https://github.com/EspressifApp/EsptouchForAndroid) and [iOS source code](https://github.com/EspressifApp/EsptouchForIOS) is available.

* Compile this example and upload it to an ESP32.

* Make sure your phone connect to target AP (2.4GHz).

* Open ESPTOUCH app and input password. There will be success message after few sec.

### 主要功能
1. smartconfig模式
2. nvs保存wifi信息
3. make erase_flash 清空flash信息【可选】（如果已经保存过wifi信息了，需要先进行这一步）
4. make flash monitor
5. 密码错误/连接断开，在flash没有密码的情况下会重新进入smartconfig模式；有密码则会重连
6. http读取天气信息，json解析
