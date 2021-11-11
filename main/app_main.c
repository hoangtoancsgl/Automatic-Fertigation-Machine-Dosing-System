
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
#include "input_iot.h"
#include "ledc_app.h"
#include "mqtt_client_app.h"
#include "ws2812b.h"
#include "smart_config.h"
#include "https_ota.h"


static const char *TAG = "DOSING SYSTEM";

//buffer for sensor data
char JSON_buff[100];

/*LED state for wifi connection:
    1 => CONNECTED
    2 => DISCONNECTED
    3 => SMART CONFIG
*/
uint8_t led_state = 2;

//Event group button press
#define BIT_SHORT_PRESS 	( 1 << 0 )
#define BIT_NORMAL_PRESS	( 1 << 1 )
#define BIT_LONG_PRESS	    ( 1 << 2 )

#define LED GPIO_NUM_2

static EventGroupHandle_t Button_event_group;

//Soft timer for button press
TimerHandle_t xTimers;
extern timeoutButton_t timeoutButton_callback;

//Genarate JSON data from sensor output
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

//Webserver event callback
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

void color_data_callback(char* data, int len)
{
    printf("RGB: %s\n", data);
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

void input_even_callback(int pin, uint64_t tick)
{
    if(pin == GPIO_NUM_0)
    {
        int press_ms = tick*portTICK_PERIOD_MS;
        BaseType_t  xHigherPriorityTaskWoken = pdFALSE;
        if(press_ms < 1000)
        {
            //short press
            xEventGroupSetBitsFromISR(Button_event_group, BIT_SHORT_PRESS,  &xHigherPriorityTaskWoken);
        }
        else if(press_ms < 2000)
        {
            //normal press
            xEventGroupSetBitsFromISR(Button_event_group, BIT_NORMAL_PRESS,  &xHigherPriorityTaskWoken);
        }
        else if(press_ms < 4000)
        {
            //long press
            xEventGroupSetBitsFromISR(Button_event_group, BIT_LONG_PRESS,  &xHigherPriorityTaskWoken);

        }
    }
}

void button_timeout_callback(int pin)
{
    if(pin == GPIO_NUM_0)
    {
        printf("Button 0 timeOut! \n");
        start_smart_config();
    }
}

void vTimerCallback (TimerHandle_t xTimer)
{
    uint32_t ID;
    configASSERT( xTimer );
    ID = ( uint32_t ) pvTimerGetTimerID( xTimer );

    if(ID == 0)
    {
        timeoutButton_callback(BUTTON_0);
    }

}

void Button_Task( void * pvParameters )
{
    while(1)
    {
        EventBits_t uxBits = xEventGroupWaitBits(Button_event_group, BIT_LONG_PRESS|BIT_SHORT_PRESS|BIT_NORMAL_PRESS, pdTRUE, pdFALSE, portMAX_DELAY);

        if(uxBits & BIT_SHORT_PRESS)
        {
            printf("Short Press! \n");
        }
        else if(uxBits & BIT_NORMAL_PRESS)
        {
            printf("Normal Press! \n");
        }
        else if(uxBits & BIT_LONG_PRESS)
        {
            printf("Long Press! \n");
        }

    }
}

void Led_task( void * pvParameters )
{
    while(1)
    {
        switch(led_state)
        {
            /*LED state for wifi connection:
            1 => CONNECTED
            2 => DISCONNECTED
            3 => SMART CONFIG
            */
            case 1:
                output_io_set_level(LED, 1);
                vTaskDelay(100/portTICK_PERIOD_MS);
                output_io_set_level(LED, 0);
                vTaskDelay(3000/portTICK_PERIOD_MS);
                break;
            case 2:
                output_io_set_level(LED, 1);
                vTaskDelay(500/portTICK_PERIOD_MS);
                output_io_set_level(LED, 0);
                vTaskDelay(500/portTICK_PERIOD_MS);
                break;
            case 3:
                output_io_set_level(LED, 1);
                vTaskDelay(100/portTICK_PERIOD_MS);
                output_io_set_level(LED, 0);
                vTaskDelay(100/portTICK_PERIOD_MS);
                break;
            default:
                break;
        }
    }
}

//MQTT data subscribed callback
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
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
 
    //Init wifi
    initialise_wifi();

    //init callback function
    http_set_callback_switch(switch_data_callback);
    http_set_callback_dht11(dht11_data_callback);
    http_set_callback_slider(slider_data_callback);
    http_set_callback_wifi_infor(wifi_infor_data_callback);
    http_set_callback_color(color_data_callback);

    mqtt_set_callback_data_subscribed(mqtt_event_data_callback);

    start_webserver();

    //Init button and LED for smart config
    Button_event_group = xEventGroupCreate();
    input_io_create(GPIO_NUM_0, ANY_EDLE);
   

    input_set_callback(input_even_callback);
    input_set_timeoutButton_callback(button_timeout_callback);

    output_io_create(GPIO_NUM_2);

    xTaskCreate(Led_task, "LED_task", 2048, NULL, 3, NULL);
    xTaskCreate(Button_Task, "ButtonTask", 2048, NULL, 3, NULL);
    xTaskCreate(&check_update_task, "check_update_task", 8192, NULL, 5, NULL);
    
    xTimers = xTimerCreate("Timmer_for_button_timeout", 5000/portTICK_PERIOD_MS, pdFALSE, (void *) 0, vTimerCallback);
    
    // ws2812b_init(GPIO_NUM_15, 8);
    // // ledc_init();
    // // ledc_add_pin(GPIO_NUM_2, 0);

    mqtt_app_start();
    while(1)
    {
        // mqtt_publish_data("hoangtoancsgl/test", "test");
        vTaskDelay(2000/portTICK_RATE_MS);
    }

}
