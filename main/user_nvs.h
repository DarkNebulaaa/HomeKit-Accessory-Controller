#pragma once
#ifndef USER_NVS_H
#define USER_NVS_H
#include "esp_err.h"
/* NVS 初始化 */
esp_err_t nvs_init(void);
/*寫入WiFi資料*/
esp_err_t write_wifi_data(const char *ssid, const char *password);
/*寫入裝置類型*/
esp_err_t write_devType(const char *devType);
/*讀取裝置類型*/
esp_err_t read_devType(char *devType, size_t len);
/*出廠重置NVS中WiFi資料*/
esp_err_t factory_reset_nvs(void);
#endif /* USER_NVS_H */