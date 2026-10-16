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

#pragma once

#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_continuous.h>

#include "thread.h"

namespace clamp {

class Power: public WakeupThread {
public:
	Power(adc_channel_t current_ch);
	~Power() = delete;

	void start();

private:
	static constexpr const unsigned int SAMPLE_RATE_HZ = 48000;
	static constexpr const unsigned int BUFFER_SIZE_B = SAMPLE_RATE_HZ * SOC_ADC_DIGI_DATA_BYTES_PER_CONV;
	static constexpr const unsigned int FRAMES_PER_S = 5; /* 200ms: 10 cycles at 50Hz, 12 cycles at 60Hz */
	static constexpr const unsigned int FRAME_SIZE_B = BUFFER_SIZE_B / FRAMES_PER_S;

	static IRAM_ATTR bool adc_conv_done(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data);

	unsigned long run_tasks() override;

	adc_continuous_handle_t adc_;
	adc_cali_handle_t cali_;
};

} // namespace clamp
