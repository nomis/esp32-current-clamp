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

#include "clamp/power.h"

extern "C" void app_main() {
	ESP_ERROR_CHECK(esp_task_wdt_add(nullptr));

	auto &power = *new clamp::Power{ADC_CHANNEL_1};
	power.start();

	ESP_ERROR_CHECK(esp_task_wdt_reset());

	TaskStatus_t status;

	vTaskGetInfo(nullptr, &status, pdTRUE, eRunning);
	ESP_LOGD("main", "Free stack: %lu", status.usStackHighWaterMark);

	ESP_ERROR_CHECK(esp_task_wdt_delete(nullptr));
}
