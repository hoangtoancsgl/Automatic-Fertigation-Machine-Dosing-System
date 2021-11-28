
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
#include "adc.h"
#include "ds18b20.h"
#include "rotary_encoder.h"
#include "HD44780.h"


static const char *TAG = "DOSING SYSTEM";

//EC11 encoder for settings
#define ROT_ENC_A_GPIO 13
#define ROT_ENC_B_GPIO 14

#define ENABLE_HALF_STEPS true  // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT          50      // Set to a positive non-zero number to reset the position if this value is exceeded
#define FLIP_DIRECTION    true  // Set to true to reverse the clockwise/counterclockwise sense

//LCD 16x2
#define LCD_ADDR 0x27
#define SDA_PIN  26
#define SCL_PIN  25
#define LCD_COLS 16
#define LCD_ROWS 2


//Sensor data struct
typedef struct Data_Send
{
    float PH;
    float EC;
    int TDS;
    float temperature;
    
}Data;

//buffer for sensors data
char JSON_buff[100];

/*LED state for wifi connection:
    1 => CONNECTED to wifi
    2 => DISCONNECTED
    3 => SMART CONFIG
    4=> CONNECTED MQTT
*/
uint8_t led_state = 2;
#define LED GPIO_NUM_2


//Event group button press
#define BIT_SHORT_PRESS 	( 1 << 0 )
#define BIT_NORMAL_PRESS	( 1 << 1 )
#define BIT_LONG_PRESS	    ( 1 << 2 )
static EventGroupHandle_t Button_event_group;


//Soft timer for button press
TimerHandle_t xTimers;
extern timeoutButton_t timeoutButton_callback;

//Mutex for handle variable data sensors
SemaphoreHandle_t Sensor_Semaphore = NULL;

//sensors value
static float temp_value=0 , tds_value=0, ph_value=0, ec_value =0.2;

//Genarate JSON data from sensor output
char* GenData(Data myData)
{
    char str_PH[100];
    char str_EC[100];
    char str_TDS[100];
    char str_temp[100];
    
    for(int i=0;i<100;i++)
    {
        str_PH[i]=0;
        str_EC[i]=0;
        str_TDS[i]=0;
        str_temp[i]=0;
        
        JSON_buff[i]=0;
    }
    sprintf(str_PH, "%0.2f", myData.PH);
    sprintf(str_EC, "%0.2f", myData.EC);
    sprintf(str_TDS, "%d", myData.TDS);
    sprintf(str_temp, "%0.1f", myData.temperature);
   
    //{"PH":"123","EC":"256", "TDS": "1", "temp":"23"}

    strcat(JSON_buff, "{\"PH\":\"");

    strcat(JSON_buff, str_PH);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"EC\":\"");
    strcat(JSON_buff, str_EC);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"TDS\":\"");
    strcat(JSON_buff, str_TDS);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"temp\":\"");
    strcat(JSON_buff, str_temp);

    strcat(JSON_buff, "\"}");

    return JSON_buff;
}

/*
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

*/

void input_even_callback(int pin, uint64_t tick)
{
    if(pin == GPIO_NUM_32)
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
    if(pin == GPIO_NUM_32)
    {
        printf("Button 0 timeOut! \n");
        esp_err_t t = mqtt_app_stop();
        if( t == ESP_OK)
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
        timeoutButton_callback(GPIO_NUM_32);
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

//Tasks 
void Button_Task( void * pvParameters )
{
    while(1)
    {
        EventBits_t uxBits = xEventGroupWaitBits(Button_event_group, BIT_LONG_PRESS|BIT_SHORT_PRESS|BIT_NORMAL_PRESS, pdTRUE, pdFALSE, portMAX_DELAY);

        if(uxBits & BIT_SHORT_PRESS)
        {
            printf("Short Press! \n");
            esp_err_t t = mqtt_app_stop();
            if( t == ESP_OK)
                OTA_start();
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

void Read_sensor_task( void * pvParameters)
{
    while(1)
    {
        xSemaphoreTake( Sensor_Semaphore, portMAX_DELAY );
        tds_value = adc_read_tds_sensor();
        ph_value = adc_read_ph_sensor();
        temp_value = ds18b20_get_temp();
        
        xSemaphoreGive(Sensor_Semaphore);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void Led_task( void * pvParameters )
{
    while(1)
    {
        switch(led_state)
        {
            /*LED state for wifi connection:
            1 => CONNECTED to wifi
            2 => DISCONNECTED
            3 => SMART CONFIG
            4=> CONNECTED MQTT
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
            case 4:
                output_io_set_level(LED, 1);
                vTaskDelay(100/portTICK_PERIOD_MS);
                output_io_set_level(LED, 0);
                vTaskDelay(200/portTICK_PERIOD_MS);
                output_io_set_level(LED, 1);
                vTaskDelay(100/portTICK_PERIOD_MS);
                output_io_set_level(LED, 0);

                vTaskDelay(3000/portTICK_PERIOD_MS);
                break;
            default:
                break;
        }
    }
}

void Mqtt_communication_task( void * pvParameters)
{
    while(1)
    {
        xSemaphoreTake( Sensor_Semaphore, portMAX_DELAY );
        Data myData = {ph_value, ec_value, tds_value, temp_value};

        mqtt_publish_data("hoangtoancsgl/sensors_data", GenData(myData));
        xSemaphoreGive(Sensor_Semaphore);
        vTaskDelay(5000/portTICK_RATE_MS);
    }
}

void EC11_encoder_task( void * pvParameters)
{
    // Initialise the rotary encoder device with the GPIOs for A and B signals
    rotary_encoder_info_t info = { 0 };
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));
#endif

    // Create a queue for events from the rotary encoder driver.
    // Tasks can read from this queue to receive up to date position information.
    QueueHandle_t event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));
    while(1)
    {
        // Wait for incoming events on the event queue.
        rotary_encoder_event_t event = { 0 };
        if (xQueueReceive(event_queue, &event, 1000 / portTICK_PERIOD_MS) == pdTRUE)
        {
            ESP_LOGI(TAG, "Event: position %d, direction %s", event.state.position,
                     event.state.direction ? (event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");
        }
        else
        {
            
            // Poll current position and direction
            rotary_encoder_state_t state = { 0 };
            ESP_ERROR_CHECK(rotary_encoder_get_state(&info, &state));
            /*
            ESP_LOGI(TAG, "Poll: position %d, direction %s", state.position,
                     state.direction ? (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");
            */
            // Reset the device
            if (RESET_AT && (state.position >= RESET_AT || state.position <= -RESET_AT))
            {
                ESP_LOGI(TAG, "Reset");
                ESP_ERROR_CHECK(rotary_encoder_reset(&info));
            }
        }
    }
}

void LCD_task (void * pvParameters)
{
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    LCD_setCursor(2, 0);
    LCD_writeStr("Hello World!");
    int i=0;
    while(1)
    {      
        LCD_setCursor(7, 1);   
        LCD_writeChar(i+0x30);
        i++;
        if(i>9) i=0;
        vTaskDelay(1000/portTICK_PERIOD_MS);
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

    //init mqtt data subscribe function
    mqtt_set_callback_data_subscribed(mqtt_event_data_callback);

    /*
    //init http server callback function
    http_set_callback_switch(switch_data_callback);
    http_set_callback_dht11(dht11_data_callback);
    http_set_callback_slider(slider_data_callback);
    http_set_callback_wifi_infor(wifi_infor_data_callback);
    http_set_callback_color(color_data_callback);
    
    //start webserver, used to config wifi when smart config fail
    start_webserver();
        */

    //init button and LED indicator for smart config
    Button_event_group = xEventGroupCreate();
    input_io_create(GPIO_NUM_32, ANY_EDLE);
   
    input_set_callback(input_even_callback);
    input_set_timeoutButton_callback(button_timeout_callback);

    output_io_create(GPIO_NUM_2);

    //Tasks for smart config
    xTaskCreate(Led_task, "LED_task", 2048, NULL, 3, NULL);
    xTaskCreate(Button_Task, "ButtonTask", 2048, NULL, 3, NULL);

    //Timer for button, time out = 5s
    xTimers = xTimerCreate("Timmer_for_button_timeout", 5000/portTICK_PERIOD_MS, pdFALSE, (void *) 0, vTimerCallback);
    
    //Create mutex for sensor data
    Sensor_Semaphore = xSemaphoreCreateMutex();

    //int sensors 
    init_adc1();
    ds18b20_init(4);
    xTaskCreate(Read_sensor_task, "Read_sensor_task", 2048, NULL, 3, NULL);

    //init and start MQTT communication
    mqtt_app_start();
    xTaskCreate(Mqtt_communication_task, "Mqtt_communication_task", 4096, NULL, 3, NULL);

    //EC11 encoder task 
    xTaskCreate(EC11_encoder_task, "EC11_encoder_task", 4096, NULL, 3, NULL);

    //LCD task 
    xTaskCreate(LCD_task, "LCD_task", 4096, NULL, 3, NULL);

}
