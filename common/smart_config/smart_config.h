#ifndef SMART_CONFIG_H
#define SMART_CONFIG_H

#include "nvs_flash.h"
#include "nvs.h"

void initialise_wifi(void);
void write_wifi_infor_to_flash(char* ssid, char* pass);
void write_config_value_to_flash(int tds_set_value, int tds_deadband_value, float ph_set_value, float ph_deadband_value, int Voltage_686, int Voltage_401);
void read_config_value_from_flash();
void read_wifi_infor_from_flash();
void start_smart_config();


#endif