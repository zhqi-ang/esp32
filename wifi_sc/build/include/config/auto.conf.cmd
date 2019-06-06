deps_config := \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/app_trace/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/aws_iot/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/bt/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/driver/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/esp32/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/esp_adc_cal/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/esp_http_client/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/ethernet/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/fatfs/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/freertos/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/heap/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/libsodium/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/log/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/lwip/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/mbedtls/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/nvs_flash/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/openssl/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/pthread/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/spi_flash/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/spiffs/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/tcpip_adapter/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/vfs/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/wear_levelling/Kconfig \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/bootloader/Kconfig.projbuild \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/esptool_py/Kconfig.projbuild \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/components/partition_table/Kconfig.projbuild \
	/mnt/hgfs/ubuntu-share/esp/esp-mdf/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
