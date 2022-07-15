
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "ledc_app.h"



void ledc_init()
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_HIGH_SPEED_MODE,           // timer mode
        .timer_num = LEDC_TIMER_1,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

}

void ledc_add_pin(int pin, int channel)
{
    ledc_channel_config_t ledc_channel = 
    {
        .channel    = channel,
        .duty       = 0,
        .gpio_num   = pin,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_1
    };
    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel);
}

void ledc_app_set_duty(int channel, int duty)
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty *8192/100);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
}

