#ifndef INPUT_IO_H
#define INPUT_IO_H
#include "esp_err.h"
#include "hal/gpio_types.h"

#define BUTTON_0 GPIO_NUM_0

typedef enum{
    HI_TO_LOW = 2,
    LOW_TO_HI = 1,
    ANY_EDLE = 3
}   interrupt_type_edle_t; 

typedef void (*input_callback_t) (int, uint64_t);
typedef void (*timeoutButton_t ) (int);


void input_io_create(gpio_num_t gpio_num, interrupt_type_edle_t level);
int input_io_get_level(gpio_num_t gpio_num);
void input_set_callback(void *cb);
void input_set_timeoutButton_callback(void *cb);

#endif