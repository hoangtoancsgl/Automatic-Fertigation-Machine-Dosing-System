/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "http_server_app.h"
#include "cJSON.h"
#include "output_iot.h"
#include "ledc_app.h"
#include "mqtt_client_app.h"


char* ssid  = "Hoang Vuong";
char* pass = "1234567890";

#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

char JSON_buff[100];

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(char* ssid, char* pass)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    wifi_config_t wifi_config = {
        .sta = {
            // .ssid = EXAMPLE_ESP_WIFI_SSID,
            // .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, pass);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ssid, pass);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ssid, pass);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    // vEventGroupDelete(s_wifi_event_group);
}

char* GenData(Data myData)
{
    char str_PH[100];
    char str_EC[100];
    char str_temp[100];
    char str_hum[100];
    char str_Pum[100];

    for(int i=0;i<100;i++)
    {
        str_PH[i]=0;
        str_EC[i]=0;
        str_temp[i]=0;
        str_hum[i]=0;
        str_Pum[i]=0;
        JSON_buff[i]=0;
    }
    sprintf(str_PH, "%d", myData.PH);
    sprintf(str_EC, "%d", myData.EC);
    sprintf(str_temp, "%d", myData.temperature);
    sprintf(str_hum, "%d", myData.humidity);
    sprintf(str_Pum, "%d", myData.Pump_state);

    //{"PH":"123","EC":"256"}

    strcat(JSON_buff, "{\"PH\":\"");
    strcat(JSON_buff, str_PH);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"EC\":\"");
    strcat(JSON_buff, str_EC);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"temp\":\"");
    strcat(JSON_buff, str_temp);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"hum\":\"");
    strcat(JSON_buff, str_hum);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"Pum\":\"");
    strcat(JSON_buff, str_Pum);
    strcat(JSON_buff, "\"}");

    return JSON_buff;
}

void switch_data_callback(char *data, int len)
{
    if(*data == '1')
    {
        output_io_set_level(GPIO_NUM_2, 1);
    }
    else if(*data == '0')
    {
        output_io_set_level(GPIO_NUM_2, 0);
    }

}

void write_wifi_infor_to_flash(char* ssid, char* pass)
{
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else 
    {
        printf("Done\n");
        // Write
        err = nvs_set_str(my_handle, "ssid", ssid);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        err = nvs_set_str(my_handle, "pass", pass);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        nvs_close(my_handle);
    }
    
}

void read_wifi_infor_from_flash()
{
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else 
    {
        printf("Done\n");

        size_t size_ssid;
        size_t size_pass;

        nvs_get_str(my_handle, "ssid", NULL, &size_ssid);
        nvs_get_str(my_handle, "pass", NULL, &size_pass);
        char* SSID = malloc(size_ssid);
        char* PASS = malloc(size_pass);

        err = nvs_get_str(my_handle, "ssid", SSID, &size_ssid);
        if(err != ESP_OK)
        {
            printf("SSID read Failed!\n");
        }
        else 
        {
            printf("SSID read Done\n");
            ssid = SSID;
        }

        err = nvs_get_str(my_handle, "pass", PASS, &size_pass);
        if(err != ESP_OK)
        {
            printf("PASS read Failed!\n");
        }
        else 
        {
            printf("PASS read Done\n");
            pass = PASS;
        }
    }

}

void wifi_infor_data_callback(char *data, int len)
{
    // "Hoang Vuong@1234567890@"
    char ssid[30];
    char pass[30];
    char *pt = strtok(data, "@");
    if(pt)
        strcpy(ssid, pt);
    pt = strtok(NULL, "@");
    if(pt)
        strcpy(pass, pt);
    printf("SSID: %s\n", ssid);
    printf("PASS: %s\n", pass);

    write_wifi_infor_to_flash(ssid, pass);

    // // stop_webserver();
    // esp_wifi_disconnect();
    // esp_wifi_stop();

    // // wifi_init_sta(ssid, pass);


    // wifi_config_t wifi_config = {
    //     .sta = {

	//      .threshold.authmode = WIFI_AUTH_WPA2_PSK,

    //         .pmf_cfg = {
    //             .capable = true,
    //             .required = false
    //         },
    //     },
    // };

    // strcpy((char*)wifi_config.sta.ssid, ssid);
    // strcpy((char*)wifi_config.sta.password, pass);

    // wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    // ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    // ESP_ERROR_CHECK(esp_wifi_start());

    // EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
    //     WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
    //     pdTRUE,
    //     pdFALSE,
    //     portMAX_DELAY);

    // if (bits & WIFI_CONNECTED_BIT) {
    //     ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
    //              ssid, pass);
    // } else if (bits & WIFI_FAIL_BIT) {
    //     ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
    //              ssid, pass);
    // } else {
    //     ESP_LOGE(TAG, "UNEXPECTED EVENT");
    // }
       

}

void slider_data_callback(char *data, int len)
{
    char num_str[10];
    memcpy(num_str, data, len+1);
    int duty = atoi(num_str);
    printf("%d\n", duty);
    ledc_app_set_duty(0, duty);
}

void dht11_data_callback(void)
{
    static int temp=20, hum=70;

    Data myData = {7, 3, 5, temp++, hum++};
   
    dht11_response(GenData(myData), strlen(GenData(myData)));

}

void mqtt_event_data_callback(char *topic, char *mess)
{
    if(strstr(topic, "123"))
    {
        printf("Button1 is pressed!\n"); 
        output_io_toggle(2);
    } 
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    //Read wifi information form flash
    read_wifi_infor_from_flash();
    printf("Data read SSID: %s\n", ssid);
    printf("Data read PASS: %s\n", pass);

    //Connect to wifi
    wifi_init_sta(ssid, pass);

    http_set_callback_switch(switch_data_callback);
    http_set_callback_dht11(dht11_data_callback);
    http_set_callback_slider(slider_data_callback);
    http_set_callback_wifi_infor(wifi_infor_data_callback);

    mqtt_set_callback_data_subscribed(mqtt_event_data_callback);


    output_io_create(GPIO_NUM_2);
    // ledc_init();
    // ledc_add_pin(GPIO_NUM_2, 0);


    start_webserver();  
    mqtt_app_start();
    while(1)
    {
        mqtt_publish_data("hoangtoancsgl/test", "test");
        vTaskDelay(1000/portTICK_RATE_MS);
    }

}
