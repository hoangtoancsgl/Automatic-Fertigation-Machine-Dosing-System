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

//buffer for generate ID 
extern char data_buff[100];
extern char status_buff[100];
extern char mac_buff[12];
extern char config_buff[100];


//wifi infor default 
static char ssid[100]  = "Mang Yeu";
static char pass[100] = "khongcho";

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

extern uint8_t led_state;
static bool smart_config_process = false;

static const char *TAG = "SMART_CONFIG";

extern float tds_set_value, tds_deadband_value, ph_deadband_value, ph_set_value;
extern int Voltage_686, Voltage_401;
extern float k_value;
extern float Nu_ratio;

wifi_config_t wifi_config;

void write_config_value_to_flash(int tds_set_value, int tds_deadband_value, float ph_set_value, float ph_deadband_value, int Voltage_686, int Voltage_401, float k_value, float Nu_ratio)
{
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle to write config value... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGI(TAG, "Error opening NVS handle!");
    } 
    else 
    {
        err = nvs_set_i16(my_handle, "tds_set_value", (int)tds_set_value);
        // printf((err != ESP_OK) ? "tds set value Failed!\n" : "Done\n");

        err = nvs_set_i16(my_handle, "Voltage_686", Voltage_686);
        // printf((err != ESP_OK) ? "tds set value Failed!\n" : "Done\n");

        err = nvs_set_i16(my_handle, "Voltage_401", Voltage_401);
        // printf((err != ESP_OK) ? "tds set value Failed!\n" : "Done\n");

        err = nvs_set_i16(my_handle, "tds_db_value", (int)tds_deadband_value);
        // printf((err != ESP_OK) ? "tds deadband value Failed!\n" : "Done\n");

        err = nvs_set_i16(my_handle, "ph_set_value", (int)(ph_set_value*10));
        // printf((err != ESP_OK) ? "ph set value Failed!\n" : "Done\n");

        err = nvs_set_i16(my_handle, "ph_db_value", (int)(ph_deadband_value*10));
        // printf((err != ESP_OK) ? "ph deadband value Failed!\n" : "Done\n");

        err = nvs_set_i16(my_handle, "k_value", (int)(k_value*1000));
        // printf((err != ESP_OK) ? "ph deadband value Failed!\n" : "Done\n");

        err = nvs_set_i16(my_handle, "Nu_ratio", (int)(Nu_ratio*1000));
        // printf((err != ESP_OK) ? "ph deadband value Failed!\n" : "Done\n");

        // printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        nvs_close(my_handle);
        if(err == ESP_OK) 
            ESP_LOGI(TAG, "DONE!");
    }
}

void read_config_value_from_flash()
{
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle to read config value... ");
    nvs_handle_t my_handle;
    int t=0;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    } 
    else 
    {
        err = nvs_get_i16(my_handle, "tds_set_value", &t);
        // printf((err != ESP_OK) ? "tds set value value Failed!\n" : "Done\n");
        tds_set_value = t;

        err = nvs_get_i16(my_handle, "Voltage_686", &t);
        // printf((err != ESP_OK) ? "tds set value value Failed!\n" : "Done\n");
        Voltage_686 = t;

        err = nvs_get_i16(my_handle, "Voltage_401", &t);
        // printf((err != ESP_OK) ? "tds set value value Failed!\n" : "Done\n");
        Voltage_401 = t;

        err = nvs_get_i16(my_handle, "tds_db_value", &t);
        // printf((err != ESP_OK) ? "tds deadband value value Failed!\n" : "Done\n");
        tds_deadband_value = t;
        
        err = nvs_get_i16(my_handle, "ph_set_value", &t);
        // printf((err != ESP_OK) ? "ph set value value Failed!\n" : "Done\n");
        ph_set_value = (float)t/10;
        
        err = nvs_get_i16(my_handle, "ph_db_value", &t);
        // printf((err != ESP_OK) ? "ph deadband value value Failed!\n" : "Done\n");
        ph_deadband_value = (float)t/10;

        err = nvs_get_i16(my_handle, "k_value", &t);
        // printf((err != ESP_OK) ? "ph deadband value value Failed!\n" : "Done\n");
        k_value = (float)t/1000;

        err = nvs_get_i16(my_handle, "Nu_ratio", &t);
        // printf((err != ESP_OK) ? "ph deadband value value Failed!\n" : "Done\n");
        Nu_ratio = (float)t/1000;

    }

}

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
        if(err == ESP_OK) 
            ESP_LOGI(TAG, "DONE!");
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

            // //Write wifi infor to flash
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

    

    read_wifi_infor_from_flash();


    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
   
    uint8_t mac_add[6];    
    esp_wifi_get_mac(WIFI_IF_STA, mac_add);
    
    strcat(status_buff, "hoangtoancsgl/");
    strcat(data_buff, "hoangtoancsgl/");
    strcat(config_buff, "hoangtoancsgl/");
    for(int i=0;i<6;i++)
    {
        char temp[3];
        if(mac_add[i]<10) 
        {
            sprintf(temp, "%d", 0);
            strcat(status_buff, temp);
            strcat(data_buff, temp);
            strcat(config_buff, temp);
            strcat(mac_buff, temp);
        }
        
        sprintf(temp, "%x", mac_add[i]);
        strcat(status_buff, temp);
        strcat(data_buff, temp);
        strcat(config_buff, temp);
        strcat(mac_buff, temp);
    }
    strcat(status_buff, "/status");
    strcat(data_buff, "/data");
    strcat(config_buff, "/config");
    //  printf("Config: %.*s\r\n", 50, config_buff);

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






