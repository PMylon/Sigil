/*
 * Copyright 2019 The TensorFlow Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "accelerometer_handler.hpp"
#include "constants.hpp"
#include "cmath"

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/zephyr.h>

#define BUFLEN 300
#define st_lsm9ds1_acc st_lsm6ds0 // Driver for lsm9ds1 acc is compatible with lsm6ds0
#define sensor_acc nxp_fxos8700

int begin_index = 0;
const struct device *sensor = DEVICE_DT_GET_ONE(sensor_acc);
int current_index = 0;

float bufx[BUFLEN] = { 0.0f };
float bufy[BUFLEN] = { 0.0f };
float bufz[BUFLEN] = { 0.0f };

bool initial = true;

// How often we should save a measurement during downsampling
int sample_every_n;

// The number of measurements since we last saved one
int sample_skip_counter = 0;

static int GetUpdateRate(tflite::ErrorReporter *error_reporter, int sampling_freq)
{
	int update_rate = sampling_freq;

	if (kTargetHz < sampling_freq)
	{
		update_rate = static_cast<int>(roundf(sampling_freq / kTargetHz));
	}
	else if (kTargetHz > sampling_freq)
	{
		TF_LITE_REPORT_ERROR(error_reporter, "Target sampling frequency is higher than the configured one!\n");
	}

	return update_rate;
}

TfLiteStatus SetupAccelerometer(tflite::ErrorReporter *error_reporter)
{
	if (!device_is_ready(sensor)) {
		printk("%s: device not ready.\n", sensor->name);
		return kTfLiteApplicationError;
	}

	if (sensor == NULL) {
		TF_LITE_REPORT_ERROR(error_reporter,
				     "Failed to get accelerometer, name: %s\n",
				     sensor->name);
	} else {
		TF_LITE_REPORT_ERROR(error_reporter, "Got accelerometer, name: %s\n",
				     sensor->name);
	}

	// struct sensor_value sampling_freq;

	struct sensor_value sampling_freq = {
		.val1 = 50,
		.val2 = 0,
	};

	if (sensor_attr_set(dev, SENSOR_CHAN_ALL,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &sampling_freq)) {
		printk("Could not set sampling frequency\n");
		return;
	}

	// if (sensor_attr_get(sensor, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &sampling_freq) < 0)
	// {
	// 	TF_LITE_REPORT_ERROR(error_reporter, "Failed to get device's sampling frequency!\n");
	// }

	// TODO Debug
	printk("Sampling frequency is: %d\n", sampling_freq.val1);

	sample_every_n = GetUpdateRate(error_reporter, sampling_freq.val1);

	return kTfLiteOk;
}

bool ReadAccelerometer(tflite::ErrorReporter *error_reporter, float *input,
		       int length)
{
	int rc;
	struct sensor_value accel[3];
	int samples_count;

	rc = sensor_sample_fetch(sensor);
	if (rc < 0) {
		TF_LITE_REPORT_ERROR(error_reporter, "Fetch failed\n");
		return false;
	}
	/* Skip if there is no data */
	if (!rc) {
		return false;
	}

	samples_count = rc;

	for (int i = 0; i < samples_count; i++) {
		rc = sensor_channel_get(sensor, SENSOR_CHAN_ACCEL_XYZ, accel);

		if (rc < 0) {
			TF_LITE_REPORT_ERROR(error_reporter, "ERROR: Update failed: %d\n", rc);
			return false;
		}

		sample_skip_counter++;

		if (sample_skip_counter != sample_every_n)
		{
			continue;
		}

		const int model_x = 2; // z
		const int model_y = 1; // y
		const int model_z = 0; // x

		bufx[begin_index] = (float)sensor_value_to_double(&accel[model_x]);
		bufy[begin_index] = (float)sensor_value_to_double(&accel[model_y]);
		bufz[begin_index] = (float)sensor_value_to_double(&accel[model_z]);

		begin_index++;

		sample_skip_counter = 0; // Reset counter as we took the sample

		if (begin_index >= BUFLEN) {
			begin_index = 0;
		}
	}

	if (initial && begin_index >= 100) {
		initial = false;
	}

	if (initial) {
		return false;
	}

	int sample = 0;

	for (int i = 0; i <= (length - 3); i += 3) {
		int ring_index = begin_index + sample - length / 3;

		if (ring_index < 0) {
			ring_index += BUFLEN;
		}

		input[i] = bufx[ring_index];
		input[i + 1] = bufy[ring_index];
		input[i + 2] = bufz[ring_index];
		sample++;
	}

	return true;
}
