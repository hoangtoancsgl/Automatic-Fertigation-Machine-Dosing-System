#ifndef _WS2812B_H
#define _WS2812B_H


void ws2812b_init(int tx_pin, int num_led);
void ws2812b_set_color(int index, int r, int g, int b);
void ws2812b_update_color();

#endif


