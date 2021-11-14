
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "adc.h"

#define DEFAULT_VREF    1100        
#define NO_OF_SAMPLES   64          

static esp_adc_cal_characteristics_t *adc_chars;

static const adc_channel_t TDS_sensor = ADC_CHANNEL_6; 
static const adc_channel_t PH_sensor = ADC_CHANNEL_7;    //GPIO34 

static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

static const adc_atten_t atten = ADC_ATTEN_DB_11;


void init_adc1(void)
{
    //Configure ADC
    adc1_config_width(width);
    adc1_config_channel_atten(TDS_sensor, atten);

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);
}

uint32_t adc_read_tds_sensor()
{
    uint32_t TDS_sensor_value = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) 
    {
        TDS_sensor_value += adc1_get_raw(TDS_sensor);
    }
    TDS_sensor_value /= NO_OF_SAMPLES;

    //Convert TDS_sensor_value to voltage in mV
    return esp_adc_cal_raw_to_voltage(TDS_sensor_value, adc_chars);
}

uint32_t adc_read_ph_sensor()
{
    uint32_t PH_sensor_value = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) 
    {
        PH_sensor_value += adc1_get_raw(PH_sensor);
    }
    PH_sensor_value /= NO_OF_SAMPLES;

    //Convert PH_sensor_value to voltage in mV
    return esp_adc_cal_raw_to_voltage(PH_sensor_value, adc_chars);
}