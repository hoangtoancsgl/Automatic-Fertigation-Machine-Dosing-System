#ifndef _ADC_H
#define _ADC_H

void init_adc1(void);
float adc_read_tds_sensor_voltage();
float adc_read_ph_sensor_voltage();

int read_tds_sensor(float tds_voltage, float temp_value, float k_value);
float read_ph_sensor(float ph_voltage, float Voltage_686, float Voltage_401);


#endif


