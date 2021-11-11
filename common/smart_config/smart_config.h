#ifndef SMART_CONFIG_H
#define SMART_CONFIG_H

#include "nvs_flash.h"
#include "nvs.h"

void initialise_wifi(void);
void write_wifi_infor_to_flash(char* ssid, char* pass);
void read_wifi_infor_from_flash();
void start_smart_config();


#endif