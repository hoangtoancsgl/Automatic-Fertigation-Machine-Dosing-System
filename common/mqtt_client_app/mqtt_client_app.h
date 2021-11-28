#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H


typedef void (*mqtt_data_callback_t ) (char* topic, char* mess);

void mqtt_app_start(void);
void mqtt_set_callback_data_subscribed(void *cb);
void mqtt_publish_data(char* topic, char* mess);
esp_err_t mqtt_app_stop(void);

#endif