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

#include "clamp/power.h"

#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_continuous.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <string.h>

#include <chrono>
#include <cstring>
#include <thread>

namespace clamp {

Power::Power(adc_channel_t current_ch) : WakeupThread("power", false) {
	adc_continuous_handle_cfg_t handle_config{};
	handle_config.max_store_buf_size = BUFFER_SIZE_B;
	handle_config.conv_frame_size = FRAME_SIZE_B;
	ESP_ERROR_CHECK(adc_continuous_new_handle(&handle_config, &adc_));

	adc_continuous_config_t adc_config{};
	adc_config.sample_freq_hz = SAMPLE_RATE_HZ;
	adc_config.conv_mode = ADC_CONV_SINGLE_UNIT_1;

	adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX]{{}};
	adc_pattern[0].unit = ADC_UNIT_1;
	adc_pattern[0].channel = current_ch;
	adc_pattern[0].atten = ADC_ATTEN_DB_0;
	adc_pattern[0].bit_width = ADC_BITWIDTH_12;
	adc_config.pattern_num = 1;
	adc_config.adc_pattern = adc_pattern;
	ESP_ERROR_CHECK(adc_continuous_config(adc_, &adc_config));

	adc_cali_curve_fitting_config_t cali_config{};
	cali_config.unit_id = ADC_UNIT_1;
	cali_config.chan = current_ch;
	cali_config.atten = ADC_ATTEN_DB_0;
	cali_config.bitwidth = ADC_BITWIDTH_12;
	ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_));
}

void Power::start() {
	adc_continuous_evt_cbs_t adc_cbs{};
	std::thread t;

	adc_cbs.on_conv_done = adc_conv_done;

	ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(adc_, &adc_cbs, this));
	ESP_ERROR_CHECK(adc_continuous_start(adc_));

	make_thread(t, "power", 6144, 1, &Power::run_loop, this);
	t.detach();
}

bool Power::adc_conv_done(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data) {
	Power *power = reinterpret_cast<Power*>(user_data);
	power->wake_up_isr();
	return false;
}

unsigned long Power::run_tasks() {
	while (true) {
		static uint8_t frame[FRAME_SIZE_B];
		static adc_continuous_data_t readings[FRAME_SIZE_B / SOC_ADC_DIGI_RESULT_BYTES];
		uint32_t frame_len = 0;
		uint32_t readings_len = 0;

		if (adc_continuous_read(adc_, frame, sizeof(frame), &frame_len, 0) == ESP_OK) {
			if (adc_continuous_parse_data(adc_, frame, frame_len, readings, &readings_len) == ESP_OK) {
				unsigned int valid = 0;

				for (int i = 0; i < readings_len; i++) {
					if (!readings[i].valid) {
						continue;
					}

					int voltage_mV;

					if (adc_cali_raw_to_voltage(cali_, readings[i].raw_data, &voltage_mV) != ESP_OK) {
						readings[i].valid = false;
						continue;
					}

					readings[i].raw_data = voltage_mV;
					valid++;
				}
			}
		} else {
			break;
		}
	}

	return ULONG_MAX;
}

} // namespace clamp
