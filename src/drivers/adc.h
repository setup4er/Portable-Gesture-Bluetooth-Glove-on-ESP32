#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void adc_init(void);

float adc_get_voltage(void);
int adc_get_percent(void);

int adc_voltage_to_percent(float voltage);

#endif // ADC_H