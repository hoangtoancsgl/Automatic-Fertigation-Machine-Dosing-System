#ifdef   __HTTP_SERVER_APP_H
#define  __HTTP_SERVER_APP_H

#endif
#include <esp_http_server.h>

typedef void (*http_post_callback_t) (char* dta, int);
typedef void (*http_get_data_callback_t) (char* dta, int);

typedef void (*http_get_callback_t)(void);

typedef struct Data_Send
{
    int PH;
    int EC;
    int Pump_state;
    int temperature;
    int humidity;
}Data;


void start_webserver(void);
void stop_webserver(void);
void http_set_callback_switch(void *cb);
void http_set_callback_dht11(void *cb);
void http_set_callback_slider(void *cb);
void http_set_callback_color(void *cb);

void http_set_callback_wifi_infor(void *cb);
void dht11_response(char* data, int len);


