#include<stdio.h>
#include<esp_log.h>
#include<driver/gpio.h>
#include "input_iot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"


input_callback_t  input_calback = NULL;
timeoutButton_t timeoutButton_callback = NULL;
extern TimerHandle_t xTimers;

static uint64_t _start, _stop, _pressTick;


static void IRAM_ATTR gpio_input_handler(void* arg)
{
    int gpio_num = (uint32_t) arg;
    uint32_t rtc = xTaskGetTickCountFromISR();

    if(gpio_get_level(gpio_num) == 0)
    {
        _start = rtc;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStartFromISR( xTimers, &xHigherPriorityTaskWoken );
    }
    else
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStopFromISR( xTimers, &xHigherPriorityTaskWoken );

        _stop = rtc;
        _pressTick = _stop - _start;
        input_calback(gpio_num, _pressTick);
    }

    // input_calback(gpio_num, 123);
}

void input_io_create(gpio_num_t gpio_num, interrupt_type_edle_t type)
{
    gpio_pad_select_gpio(gpio_num);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(gpio_num, type);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_num, gpio_input_handler, (void*) gpio_num);
}

int input_io_get_level(gpio_num_t gpio_num)
{
    return gpio_get_level(gpio_num); 
}

void input_set_callback(void *cb)
{
    input_calback = cb;
}

void input_set_timeoutButton_callback(void *cb)
{
    timeoutButton_callback = cb;
}