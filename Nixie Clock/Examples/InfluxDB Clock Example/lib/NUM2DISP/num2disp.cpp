/**
 @file num2disp.cpp
 @brief Driver for I2C communication with PCA9698
 @author Edward62740
 */

#include "num2disp.h"
#include "NTC_PCA9698.h"

struct FullDisplayStruct FullDisplay;
uint8_t internal_num_current[6];
uint8_t internal_num_prev[6];

num2disp_err_t num2disp_createInstanceNumericalDisplay(NumericalDisplay_t *num,
		uint8_t pinout[]) {
	for (int i = 0; i <= 10; i++) {
		num->pinout[i] = pinout[i];
	}
}

num2disp_err_t num2disp_createInstanceFullDisplay(NumericalDisplay_t *num1,
		NumericalDisplay_t *num2, NumericalDisplay_t *num3,
		NumericalDisplay_t *num4, NumericalDisplay_t *num5,
		NumericalDisplay_t *num6, uint8_t num_active, uint8_t num_offset) {
	FullDisplay.display[0] = *num1;
	FullDisplay.display[1] = *num2;
	FullDisplay.display[2] = *num3;
	FullDisplay.display[3] = *num4;
	FullDisplay.display[4] = *num5;
	FullDisplay.display[5] = *num6;
	FullDisplay.num_active = num_active;
	FullDisplay.num_offset = num_offset;
}

num2disp_err_t num2disp_writeNumberToFullDisplay(uint32_t num,
		uint32_t prev_num, bool crossfade) {

	num2disp_err_t ret = 0;

	if (num > 999999 | prev_num > 999999) {

	}
	for (int i = 0; i < 6; i++) {

		internal_num_current[5 - i] = num % 10;
		num = (num - num % 10) / 10;
	}
	for (int i = 0; i < 6; i++) {

		internal_num_prev[5 - i] = prev_num % 10;
		prev_num = (prev_num - prev_num % 10) / 10;
	}
	for (uint8_t n = 0; n < FullDisplay.num_active; n++) {
		uint8_t current = internal_num_current[n + FullDisplay.num_offset];
		uint8_t prev = internal_num_prev[n + FullDisplay.num_offset];
		num2disp_writeNumberToNumericalDisplay(n + FullDisplay.num_offset,
				current, prev, crossfade);
	}

	return 1;

}

num2disp_err_t num2disp_clearNumberFromFullDisplay() {
	for (uint8_t i = 0; i < FullDisplay.num_active; i++) {
		for (uint8_t j = 0; j < 10; j++) {
			num2disp_gpio_write(
					(uint8_t)(
							FullDisplay.display[i + FullDisplay.num_offset].pinout[j]),
					0);
		}
	}
}

num2disp_err_t num2disp_writeNumberToNumericalDisplay(uint8_t index,
		uint8_t digit, uint8_t prev_digit, bool crossfade) {
	uint8_t gpio_digit_current =
			(uint8_t) FullDisplay.display[index].pinout[digit]; //get pin number from NumericalDisplayStruct
	uint8_t gpio_digit_prev =
			(uint8_t) FullDisplay.display[index].pinout[prev_digit];

	if (crossfade && (gpio_digit_current != gpio_digit_prev)) {
		num2disp_gpio_write(gpio_digit_current, 0);
		num2disp_gpio_write(gpio_digit_prev, 1);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.8);
		num2disp_gpio_write(gpio_digit_current, 1);
		num2disp_gpio_write(gpio_digit_prev, 0);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.2);
		num2disp_gpio_write(gpio_digit_current, 0);
		num2disp_gpio_write(gpio_digit_prev, 1);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.7);
		num2disp_gpio_write(gpio_digit_current, 1);
		num2disp_gpio_write(gpio_digit_prev, 0);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.3);

		num2disp_gpio_write(gpio_digit_current, 0);
		num2disp_gpio_write(gpio_digit_prev, 1);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.6);
		num2disp_gpio_write(gpio_digit_current, 1);
		num2disp_gpio_write(gpio_digit_prev, 0);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.4);
		num2disp_gpio_write(gpio_digit_current, 0);
		num2disp_gpio_write(gpio_digit_prev, 1);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.5);
		num2disp_gpio_write(gpio_digit_current, 1);
		num2disp_gpio_write(gpio_digit_prev, 0);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.5);

		num2disp_gpio_write(gpio_digit_current, 0);
		num2disp_gpio_write(gpio_digit_prev, 1);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.4);
		num2disp_gpio_write(gpio_digit_current, 1);
		num2disp_gpio_write(gpio_digit_prev, 0);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.6);
		num2disp_gpio_write(gpio_digit_current, 0);
		num2disp_gpio_write(gpio_digit_prev, 1);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.3);
		num2disp_gpio_write(gpio_digit_current, 1);
		num2disp_gpio_write(gpio_digit_prev, 0);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.7);

		num2disp_gpio_write(gpio_digit_current, 0);
		num2disp_gpio_write(gpio_digit_prev, 1);
		delay(CROSSFADE_PULSE_CYCLE_MS * 0.2);
		num2disp_gpio_write(gpio_digit_current, 1);
		num2disp_gpio_write(gpio_digit_prev, 0);
	} else if (!crossfade && (gpio_digit_current != gpio_digit_prev)) {
		num2disp_gpio_write(gpio_digit_current, 1);
		num2disp_gpio_write(gpio_digit_prev, 0);
	} else {
		num2disp_gpio_write(gpio_digit_current, 1);
	}
}

num2disp_err_t num2disp_runCathodePoisoningProtection(uint32_t iterations,
		bool style) {
	uint32_t num = 0;
	for (uint32_t i = 0; i < 10 * iterations; i++) {

		if (style) {

			if (num == 0) {
				num2disp_writeNumberToFullDisplay(0, 999999, false);
			}

			else {
				num2disp_writeNumberToFullDisplay(num, num - 111111, false);
			}

			if (num == 999999) {
				num = 0;
			} else {
				num += 111111;
			}
		} else {
			if (num == 0) {
				num = 123456;
			}

			if (num >= 123456 && num <= 345678) {
				num2disp_writeNumberToFullDisplay(num, num - 111111, false);
				num += 111111;
			} else if (num == 456789) {
				num2disp_writeNumberToFullDisplay(num, num - 111111, false);
				num += 111101;
			} else if (num == 567890) {
				num2disp_writeNumberToFullDisplay(num, 456789, false);
				num += 111011;
			} else if (num == 678901) {
				num2disp_writeNumberToFullDisplay(num, 567890, false);
				num += 110111;
			} else if (num == 789012) {
				num2disp_writeNumberToFullDisplay(num, 678901, false);
				num += 101111;
			} else if (num == 890123) {
				num2disp_writeNumberToFullDisplay(num, 789012, false);
				num += 11111;
			} else if (num == 901234) {
				num2disp_writeNumberToFullDisplay(num, 890123, false);
				num -= 888889;
			} else if (num == 12345) {
				num2disp_writeNumberToFullDisplay(num, 901234, false);
				num += 111111;
			}

		}
		delay(CATHODE_PROTECTION_INTER_MS);

	}
	num2disp_clearNumberFromFullDisplay();

}
