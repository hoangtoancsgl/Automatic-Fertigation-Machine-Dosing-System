
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "adc.h"

#define DEFAULT_VREF    1000        
#define NO_OF_SAMPLES   70     

static esp_adc_cal_characteristics_t *adc_chars;

static const adc_channel_t TDS_sensor = ADC_CHANNEL_6; //GPIO34 
static const adc_channel_t PH_sensor = ADC_CHANNEL_7;   //GPIO35 

static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

static const adc_atten_t atten = ADC_ATTEN_DB_11; //2600mV

static int adc_buff[NO_OF_SAMPLES];
static int adc_buff1[NO_OF_SAMPLES];

static int getMedianNum(int bArray[], int iFilterLen) 
{
    int bTab[iFilterLen];
    for (int i = 0; i<iFilterLen; i++) bTab[i] = bArray[i];

    int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++) 
    {
        for (i = 0; i < iFilterLen - j - 1; i++) 
        {
            if (bTab[i] > bTab[i + 1]) 
            {
                bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }
    if ((iFilterLen & 1) > 0) bTemp = bTab[(iFilterLen - 1) / 2];
    else bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
    return bTemp;
}

void init_adc1(void)
{
    //Configure ADC
    adc1_config_width(width);
    adc1_config_channel_atten(TDS_sensor, atten);
    adc1_config_channel_atten(PH_sensor, atten);

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);
}

float adc_read_tds_sensor_voltage()
{
    uint32_t TDS_sensor_value = 0;
    //Multisampling
    for (int j = 0; j < NO_OF_SAMPLES; j++) 
    {
        for (int i = 0; i < NO_OF_SAMPLES; i++) 
        {
            adc_buff[i]= adc1_get_raw(TDS_sensor);
        }
        vTaskDelay(1);
        adc_buff1[j] = getMedianNum(adc_buff, NO_OF_SAMPLES);
    }
        
    TDS_sensor_value = getMedianNum(adc_buff1, NO_OF_SAMPLES);
    //Convert TDS_sensor_value to voltage in V
    if(esp_adc_cal_raw_to_voltage(TDS_sensor_value, adc_chars)<=142) 
        return 0;
    else 
        return esp_adc_cal_raw_to_voltage(TDS_sensor_value, adc_chars)-15;
}

float adc_read_ph_sensor_voltage()
{
    uint32_t PH_sensor_value = 0;
    //Multisampling
    for (int j = 0; j < NO_OF_SAMPLES; j++) 
    {
        for (int i = 0; i < NO_OF_SAMPLES; i++) 
        {
            adc_buff[i]= adc1_get_raw(PH_sensor);
        }
        vTaskDelay(1);
        adc_buff1[j] = getMedianNum(adc_buff, NO_OF_SAMPLES);
    }
        
    PH_sensor_value = getMedianNum(adc_buff1, NO_OF_SAMPLES);
    //Convert PH_sensor_value to voltage in V
    if(esp_adc_cal_raw_to_voltage(PH_sensor_value, adc_chars)<=142) 
        return 0;
    else 
        return esp_adc_cal_raw_to_voltage(PH_sensor_value, adc_chars)-15;
}

int read_tds_sensor(float tds_voltage, float temp_value, float k_value)
{
    //Convert mV to Vol
    tds_voltage /= 1000;
    float compensationCoefficient=1.0+0.02*(temp_value-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge=tds_voltage/compensationCoefficient;  //temperature compensation

    return (int)(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*k_value; //convert voltage value to tds value

}

float read_ph_sensor(float ph_voltage, float Voltage_686, float Voltage_401)
{
    float slope = (6.86-4.01)/((Voltage_686-1500.0)/3.0 - (Voltage_401-1500.0)/3.0);  // two point: (_neutralVoltage,6.86),(_acidVoltage,4.01)
    float intercept =  6.86 - slope*(Voltage_686-1500.0)/3.0;

    return slope*(ph_voltage-1500.0)/3.0+intercept;  //y = k*x + b
}

