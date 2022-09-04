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

#include "output_handler.hpp"
#include <zephyr/zephyr.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/uart.h>

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
	     "Console device is not ACM CDC UART device");

void SetupOutput()
{
	const struct device *dev_console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;

	if (usb_enable(NULL)) {
		printk("Serial output not initialized!\n");
		return;
	}

	/* Poll if the DTR flag was set */
	while (!dtr) {
		uart_line_ctrl_get(dev_console, UART_LINE_CTRL_DTR, &dtr);
		/* Give CPU resources to low priority threads. */
		k_sleep(K_MSEC(100));
	}

	printk("Serial output initialized!\n");
}

void HandleOutput(tflite::ErrorReporter *error_reporter, int kind)
{
	/* light (red: wing, blue: ring, green: slope) */
	if (kind == 0) {
		TF_LITE_REPORT_ERROR(
			error_reporter,
			"WING:\n\r*         *         *\n\r *       * *       "
			"*\n\r  *     *   *     *\n\r   *   *     *   *\n\r    * *       "
			"* *\n\r     *         *\n\r");
	} else if (kind == 1) {
		TF_LITE_REPORT_ERROR(
			error_reporter,
			"RING:\n\r          *\n\r       *     *\n\r     *         *\n\r "
			"   *           *\n\r     *         *\n\r       *     *\n\r      "
			"    *\n\r");
	} else if (kind == 2) {
		TF_LITE_REPORT_ERROR(
			error_reporter,
			"SLOPE:\n\r        *\n\r       *\n\r      *\n\r     *\n\r    "
			"*\n\r   *\n\r  *\n\r * * * * * * * *\n\r");
	}
}
