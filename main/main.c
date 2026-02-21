/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS products only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/* HomeKit Fan Example
*/

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_event.h>
#include <esp_log.h>

#include <hap.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

#include <hap_fw_upgrade.h>
#include <iot_button.h>

#include <app_wifi.h>
#include <app_hap_setup_payload.h>

//led_strip
#include "led_strip.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

//user code
#include "user_hap_serv.h"
#include "user_wifi_ap.h"
#include "user_nvs.h"
#include "config.h"
#include "user_web.h"

const char *TAG = "HAP Garage Door";
int i = 0;
WifiMode_t RFmode;


void configure_led(void){
    led_strip_config_t strip_config = {
        .strip_gpio_num = IO_INDECATOR,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
}

void led_pwm(void *mode){
    WifiMode_t *RFmode = (WifiMode_t *)mode;
    static int direction = 0;
    static int brightness = 0;
    while (1){       
        if (!direction){
        if (brightness <=100){
            brightness += 1;
        } else direction = 1;
        }if (direction){
            brightness -= 1;
            if (brightness <= 0){
                direction = 0;
            }
        }
    led_strip_set_pixel(led_strip, 0, brightness, brightness/2, 0);
    led_strip_refresh(led_strip);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main()
{   
    configure_gpio();
    if(i == 0){
    configure_led();
    reset_key_init(RESET_GPIO);
    nvs_init();
    char DEVICE_TYPE[16] = {0};
    RFmode = wifi_serv_init();
    if (RFmode == RF_MODE_AP)xTaskCreate(led_pwm, "led_pwm", 2048, &RFmode, 1, NULL);
    i++;
    
    if (RFmode == RF_MODE_STA) {
        if(read_devType(DEVICE_TYPE, sizeof(DEVICE_TYPE)) != ESP_OK) xTaskCreate(garage_door_serv, "garage_door_serv", 4096, NULL, 5, NULL);
        else {
            if (strcmp(DEVICE_TYPE, "grgedoor") == 0) {
                xTaskCreate(garage_door_serv, "garage_door_serv", 4096, NULL, 5, NULL);
            }else if (strcmp(DEVICE_TYPE, "watering") == 0) {
                xTaskCreate(watering_serv, "watering_serv", 4096, NULL, 5, NULL);
            } 
        }
        }
    }
}
