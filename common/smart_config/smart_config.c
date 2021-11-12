#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "esp_smartconfig.h"
#include "smart_config.h"
#include "../mqtt_client_app/mqtt_client_app.h"




/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;


//wifi infor default 
static char ssid[100]  = "Hoang Vuong";
static char pass[100] = "1234567890";

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

extern uint8_t led_state;
static bool smart_config_process = false;

static const char *TAG = "SMART_CONFIG";

wifi_config_t wifi_config;

void write_wifi_infor_to_flash(char* ssid, char* pass)
{
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle to write wifi infor... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGI(TAG, "Error opening NVS handle!");
    } 
    else 
    {
        err = nvs_set_str(my_handle, "ssid", ssid);
        // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        err = nvs_set_str(my_handle, "pass", pass);
        // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        nvs_close(my_handle);
    }
    
}

void read_wifi_infor_from_flash()
{
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle to read wifi infor... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    } 
    else 
    {
        ESP_LOGI(TAG, "Done");

        size_t size_ssid;
        size_t size_pass;

        nvs_get_str(my_handle, "ssid", NULL, &size_ssid);
        nvs_get_str(my_handle, "pass", NULL, &size_pass);
        char* SSID = malloc(size_ssid);
        char* PASS = malloc(size_pass);

        err = nvs_get_str(my_handle, "ssid", SSID, &size_ssid);
        if(err != ESP_OK)
        {
            ESP_LOGI(TAG, "SSID read Failed!");
        }
        else 
        {
            ESP_LOGI(TAG, "SSID read Done");
            memcpy(ssid, SSID, sizeof(ssid));
            
        }

        err = nvs_get_str(my_handle, "pass", PASS, &size_pass);
        if(err != ESP_OK)
        {
            ESP_LOGI(TAG, "PASS read Failed!");
        }
        else 
        {
            ESP_LOGI(TAG, "PASS read Done");
            memcpy(pass, PASS, sizeof(pass));

        }
        free(SSID);
        free(PASS);
    }

}

void smartconfig_example_task(void * parm)
{
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    while (1) 
    {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "WiFi Connected to AP");
            memcpy(ssid, wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid));
            memcpy(pass, wifi_config.sta.password, sizeof(wifi_config.sta.password));

            // //Write wifi infor to falsh
            write_wifi_infor_to_flash(ssid, pass);
        }

        if(uxBits & ESPTOUCH_DONE_BIT) 
        {
            ESP_LOGI(TAG, "Smartconfig over");
            led_state = 1;
            smart_config_process = false;
            esp_smartconfig_stop();
            //Reconnect MQTT
            mqtt_app_start();
            vTaskDelete(NULL);
        }
    }
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();
    }  

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    } 

    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) 
    {
        ESP_LOGI(TAG, "Scan done");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) 
    {
        ESP_LOGI(TAG, "Found channel");
        smart_config_process = false;
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        
        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        esp_wifi_connect();

    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) 
    {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }

    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED ) 
    {
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);

        if(!smart_config_process)
        {
            ESP_LOGI(TAG, "Disconnected from AP, retry...");
            led_state = 2;
            esp_smartconfig_stop();
            esp_wifi_connect();
        }   
    }

    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) 
    {
        led_state = 1;
        ESP_LOGI(TAG, "Connected to AP");
    }

}

void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    bzero(&wifi_config, sizeof(wifi_config_t));

    
    //Read wifi information form flash
    read_wifi_infor_from_flash();
    printf("Data read SSID: %s\n", ssid);
    printf("Data read PASS: %s\n", pass);

    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );

}
void start_smart_config()
{
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    smart_config_process = true;
    ESP_LOGI(TAG, "Disconnected from AP for smart config...");
    //Smart config mode
    led_state = 3;
    ESP_LOGI(TAG, "Smart config mode...");
    xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 0, NULL);
}



