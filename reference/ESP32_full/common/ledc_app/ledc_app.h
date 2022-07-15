#ifndef LEDC_APP_H
#define LEDC_APP_H


void ledc_init();
void ledc_add_pin(int pin, int channel);
void ledc_app_set_duty(int channel, int duty);


#endif