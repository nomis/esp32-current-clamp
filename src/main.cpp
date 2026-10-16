/*
 * esp32-current-clamp - ESP32 Current Clamp
 * Copyright 2026  Simon Arlott
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <esp_err.h>
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <sdkconfig.h>

#include "clamp/power.h"

static void event_handler(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data) {
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		esp_wifi_connect();
	}
}

extern "C" void app_main() {
	ESP_ERROR_CHECK(esp_task_wdt_add(nullptr));

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	esp_event_handler_instance_t instance_any_id;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
		ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
	wifi_config_t wifi_config{};
	strncpy((char*)wifi_config.sta.ssid, CONFIG_CLAMP_WIFI_SSID, sizeof(wifi_config.sta.ssid));
	strncpy((char*)wifi_config.sta.password, CONFIG_CLAMP_WIFI_PASSWORD, sizeof(wifi_config.sta.password));
	wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start());

	auto &power = *new clamp::Power{ADC_CHANNEL_1};
	power.start();

	ESP_ERROR_CHECK(esp_task_wdt_reset());

	TaskStatus_t status;

	vTaskGetInfo(nullptr, &status, pdTRUE, eRunning);
	ESP_LOGD("main", "Free stack: %lu", status.usStackHighWaterMark);

	ESP_ERROR_CHECK(esp_task_wdt_delete(nullptr));
}
