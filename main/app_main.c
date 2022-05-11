
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

#include "esp_ota_ops.h"

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
#include "i2cdev.h"
#include "ds3231.h"
#include "sd_card.h"




static const char *TAG = "MAIN";

//EC11 encoder for settings
#define ROT_ENC_A_GPIO 25
#define ROT_ENC_B_GPIO 33

// Set to true to enable tracking of rotary encoder at half step resolution
#define ENABLE_HALF_STEPS 0 

//LCD 20x4 pins define
#define LCD_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4

//Data sent struct
typedef struct Data_Send
{
    float PH;
    int TDS;
    float temperature;

    float PH_set;
    int TDS_set;
    float PH_dead;
    int TDS_dead;

    int Nutri_A;
    int Nutri_B;
    float Nutri_Ratio;
    int Acid_So;
    int Base_So;

    int Water;
    int8_t ChangeNutri;
    
}Data;

typedef struct time
{
    int year;
    int month;
    int day;

    int hour;
    int minute;
    int second;
    
}time_Data;


//buffer for data sent
char JSON_buff[300];


//buffer for device ID 
char mac_buff[12];
char data_buff[100];
char status_buff[100];
char config_buff[100];

//file log name
char filename_arr[20];


/*LED state for wifi connection:
    1 => CONNECTED to wifi
    2 => DISCONNECTED
    3 => SMART CONFIG
    4 => MQTT CONNECTED 
*/
uint8_t led_state = 2;
#define LED GPIO_NUM_12

//SD card status
bool sd_card_status = 0;

//Buzzer
#define BUZZ GPIO_NUM_23

//4 peristaltic pump
#define Pum_NutriA GPIO_NUM_22
#define Pum_NutriB GPIO_NUM_21
#define Pum_Acid GPIO_NUM_19
#define Pum_Base GPIO_NUM_18

#define Pum_Water GPIO_NUM_17

//Limited water sensor
#define Lim_Sen_High GPIO_NUM_36
#define Lim_Sen_low GPIO_NUM_39


//volume of liquid in 1 pump (ml)
#define Pum_liquid 10

////volume of added water (ml)
#define Volume_liquid_water 1000

//Time for 1 pump (ticks)
#define Time_1_pump 200

/*OTA variable for OTA update
1 => Looking for a new firmware
2 => New firmware available
3 => Downloading and installing new firmware
4 => Current firmware is lower or equal availble one
5 => Update successfull
6 => Update fail
*/
uint8_t OTA_status=0;


//Event group button press
#define BIT_SHORT_PRESS 	( 1 << 0 )
#define BIT_NORMAL_PRESS	( 1 << 1 )
#define BIT_LONG_PRESS	    ( 1 << 2 )
static EventGroupHandle_t Button_event_group;
int short_press=0, long_press=0;


//Soft timer for button press
TimerHandle_t xTimers;
extern timeoutButton_t timeoutButton_callback;

//Mutex for handle variable data sensors
SemaphoreHandle_t Sensor_Semaphore = NULL;

SemaphoreHandle_t Screen_Semaphore = NULL;

//Sensors value
static float temp_value=0 , tds_value=0, ph_value=0;

//Sensor calibration value
//PH sensor calibration values
int Voltage_686 = 1000, Voltage_401 = 2000;
int ph_vol;

//TDS sensor calibration values
float k_value = 0.34;



//Sensors set value
float tds_set_value=500, ph_set_value=6.5;

//Sensors deadband value
float tds_deadband_value=100, ph_deadband_value=0.2;

//Nutrian value
int Nutri_A = 0, Nutri_B = 0, Acid_So = 0, Base_So = 0, Water = 0;


//Nutrian ratio: Nutrian A/ NutrianB
float Nu_ratio = 1;

//Bool variable for notifi server when nutrient were changed
int8_t change_nutri = 0;

//Firmware version
extern double FIRMWARE_VERSION;



/*---------------------------------Function-------------------------------------------------*/

//Create bip sound
void bip(int time)
{
    if(time == 10)
    {
        output_io_set_level(BUZZ, 1);
        vTaskDelay(500/portTICK_PERIOD_MS);
        output_io_set_level(BUZZ, 0);    
    }
    else
    for(int i=0;i<time;i++)
    {
        output_io_set_level(BUZZ, 1);
        vTaskDelay(100/portTICK_PERIOD_MS);
        output_io_set_level(BUZZ, 0);
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

//Genarate JSON data from sensor output
char* GenData(Data myData, time_Data mytimeData, bool type)
{
    char str_PH[100];
    char str_TDS[100];
    char str_temp[100];
    
    char str_PH_set[100];
    char str_TDS_set[100]; 
    char str_PH_dead[100];
    char str_TDS_dead[100];

    char str_NutriA[100];
    char str_NutriB[100];
    char str_Nutri_Ratio[100];
    char str_Acid_So[100];
    char str_Base_So[100];

    char str_Water[100];

    char str_ChangeNutri[100];

    char str_time[10];
    

    for(int i=0;i<100;i++) 
    {

        JSON_buff[i]=0;
        JSON_buff[i+100]=0;
        str_PH[i]=0;
        str_TDS[i]=0;
        str_temp[i]=0;
        
        str_PH_set[i]=0;
        str_TDS_set[i]=0; 
        str_PH_dead[i]=0;
        str_TDS_dead[i]=0;

        str_NutriA[i]=0;
        str_NutriB[i]=0;
        str_Nutri_Ratio[i] = 0;
        str_Acid_So[i]=0;
        str_Base_So[i]=0;

        str_Water[i]=0;
        str_ChangeNutri[i] = 0;
    }


    if(type)
    {
        sprintf(str_time, mytimeData.year<10 ? "0%d" : "%d", mytimeData.year);
        strcat(JSON_buff, str_time);
        sprintf(str_time, mytimeData.month<10 ? "/0%d" : "/%d", mytimeData.month);
        strcat(JSON_buff, str_time);
        sprintf(str_time, mytimeData.day<10 ? "/0%d   " : "/%d   ", mytimeData.day);
        strcat(JSON_buff, str_time);

        sprintf(str_time, mytimeData.hour<10 ? "0%d" : "%d", mytimeData.hour);
        strcat(JSON_buff, str_time);
        sprintf(str_time, mytimeData.minute<10 ? ":0%d" : ":%d", mytimeData.minute);
        strcat(JSON_buff, str_time);
        sprintf(str_time, mytimeData.second<10 ? ":0%d   " : ":%d   ", mytimeData.second);
        strcat(JSON_buff, str_time);
    }

    sprintf(str_PH, "%0.1f", myData.PH);
    sprintf(str_TDS, "%d", myData.TDS);
    sprintf(str_temp, "%0.1f", myData.temperature);

    sprintf(str_PH_dead, "%0.1f", myData.PH_dead);
    sprintf(str_TDS_dead, "%d", myData.TDS_dead);

    sprintf(str_PH_set, "%0.1f", myData.PH_set);
    sprintf(str_TDS_set, "%d", myData.TDS_set);

    sprintf(str_NutriA, "%d", myData.Nutri_A);
    sprintf(str_Nutri_Ratio, "%0.3f", myData.Nutri_Ratio);
    sprintf(str_NutriB, "%d", myData.Nutri_B);
    
    sprintf(str_Acid_So, "%d", myData.Acid_So);
    sprintf(str_Base_So, "%d", myData.Base_So);

    sprintf(str_Water, "%d", myData.Water);
    sprintf(str_ChangeNutri, "%d", myData.ChangeNutri);

   
    //{"PH":"123","EC":"256", "TDS": "1", "temp":"23"}

    strcat(JSON_buff, "{\"PH\":\"");

    strcat(JSON_buff, str_PH);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"TDS\":\"");
    strcat(JSON_buff, str_TDS);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"temp\":\"");
    strcat(JSON_buff, str_temp);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"PH_set\":\"");
    strcat(JSON_buff, str_PH_set);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"TDS_set\":\"");
    strcat(JSON_buff, str_TDS_set);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"PH_dead\":\"");
    strcat(JSON_buff, str_PH_dead);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"TDS_dead\":\"");
    strcat(JSON_buff, str_TDS_dead);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"Nutri_A\":\"");
    strcat(JSON_buff, str_NutriA);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"Nutri_B\":\"");
    strcat(JSON_buff, str_NutriB);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"Nutri_Ratio\":\"");
    strcat(JSON_buff, str_Nutri_Ratio);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"Acid_So\":\"");
    strcat(JSON_buff, str_Acid_So);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"Base_So\":\"");
    strcat(JSON_buff, str_Base_So);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"Water\":\"");
    strcat(JSON_buff, str_Water);
    strcat(JSON_buff, "\",");

    strcat(JSON_buff, "\"ChangeNutri\":\"");
    strcat(JSON_buff, str_ChangeNutri);


    strcat(JSON_buff, "\"}");

    return JSON_buff;
}

void Display_Settings_1(int select)
{
    LCD_setCursor(0, select);
    LCD_writeChar(2);

    for(int j=0;j<4;j++) 
        if(j != select) 
        {
            LCD_setCursor(0, j);
            LCD_writeChar(32);
        } 
}

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
        // else if(press_ms < 2000)
        // {
        //     //normal press
        //     xEventGroupSetBitsFromISR(Button_event_group, BIT_NORMAL_PRESS,  &xHigherPriorityTaskWoken);
        // }
        // else if(press_ms < 4000)
        // {
        //     //long press
        //     xEventGroupSetBitsFromISR(Button_event_group, BIT_LONG_PRESS,  &xHigherPriorityTaskWoken);
        // }
    }

}

void button_timeout_callback(int pin)
{
    if(pin == GPIO_NUM_32)
    {
        // printf("Button 0 timeOut! \n");      
        long_press =1;
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

uint8_t Json_parse(char *data)
{
    cJSON *str_json;
    
    str_json = cJSON_Parse(data);
    if(!str_json)
    {
        // printf("Not a json \n");
        return 0;
    }     
    else
    {
        // printf("Json ok \n");
        if(cJSON_GetObjectItem(str_json, "PH_val"))
        {
            ph_set_value = atof(cJSON_GetObjectItem(str_json, "PH_val")->valuestring);
            // printf("Gia tri PH_val moi: %0.2f\n", ph_set_value);
        }
        if(cJSON_GetObjectItem(str_json, "PH_dead"))
        {
            ph_deadband_value = atof(cJSON_GetObjectItem(str_json, "PH_dead")->valuestring);
            // printf("Gia tri PH_dead set moi: %0.2f\n", ph_deadband_value);
        }
        if(cJSON_GetObjectItem(str_json, "TDS_val"))
        {
            tds_set_value = atof(cJSON_GetObjectItem(str_json, "TDS_val")->valuestring);
            // printf("Gia tri TDS_val set moi: %0.2f\n", tds_set_value);
        }
        if(cJSON_GetObjectItem(str_json, "TDS_dead"))
        {
            tds_deadband_value = atof(cJSON_GetObjectItem(str_json, "TDS_dead")->valuestring);
            // printf("Gia tri TDS_dead set moi: %0.2f\n", tds_deadband_value);
        }
        if(cJSON_GetObjectItem(str_json, "Nutri_ratio"))
        {
            Nu_ratio = atof(cJSON_GetObjectItem(str_json, "Nutri_ratio")->valuestring);
            // printf("Gia tri TDS_dead set moi: %0.2f\n", tds_deadband_value);
        }
    }
        
    return 1;
}

//MQTT data subscribed callback
void mqtt_event_data_callback(char *topic, char *mess)
{
    
    if(strstr(topic, "config"))
    {
        if(Json_parse(mess)) 
        {
            write_config_value_to_flash(tds_set_value, tds_deadband_value, ph_set_value, ph_deadband_value, Voltage_686, Voltage_401, k_value, Nu_ratio);
        }
        
        xSemaphoreTake(Screen_Semaphore, portMAX_DELAY);
 
        LCD_clearScreen();
        LCD_setCursor(1, 0);
        LCD_writeStr("Write successfully!");
        LCD_setCursor(1, 2);
        LCD_writeStr("Restarting...");
        bip(3);
        vTaskDelay(3000/portTICK_PERIOD_MS);
        LCD_clearScreen();
        esp_restart();

    } 
}

void Display_Sensor_Data(float ver, int TDS_Value, int TDS_Set, float PH_Value, float PH_Set, float Temp)
{
    LCD_setCursor(1, 0);
    LCD_writeStr("Dosing System V"); 
    LCD_writeChar((int)ver + 0x30);
    LCD_writeStr("."); 
    LCD_writeChar(10*(ver-(int)ver)+0x30);

    LCD_setCursor(1, 1);
    LCD_writeStr("TDS: "); 
    Lcd_write_int(TDS_Value);
    LCD_writeStr("/"); 
    Lcd_write_int(TDS_Set);
    LCD_writeStr(" ppm  "); 

    LCD_setCursor(1, 2);
    LCD_writeStr("PH : "); 
    Lcd_write_int((int)PH_Value);
    LCD_writeStr("."); 
    Lcd_write_int(10*(PH_Value-(int)PH_Value));
    LCD_writeStr("/");
    Lcd_write_int((int)PH_Set);
    LCD_writeStr("."); 
    Lcd_write_int(10*(PH_Set-(int)PH_Set));
    
    if(Temp < 50)
    {
        LCD_setCursor(1, 3);
        LCD_writeStr("Temp: "); 
        Lcd_write_int((int)Temp);
        LCD_writeStr("."); 
        Lcd_write_int(10*(Temp-(int)Temp));
        LCD_writeChar(1);
        LCD_writeStr("C  "); 
    }

}

void Display_sensors_settings()
{
    LCD_setCursor(1, 0);
    LCD_writeStr("TDS level:"); 
    Lcd_write_int(tds_set_value);
    LCD_writeStr(" ppm ");

    LCD_setCursor(1, 1);
    LCD_writeStr("TDS deadband:");
    Lcd_write_int(tds_deadband_value);
    LCD_writeStr("   ");
    
    LCD_setCursor(1, 2);
    LCD_writeStr("PH level: "); 
    Lcd_write_int((int)ph_set_value);
    LCD_writeStr("."); 
    Lcd_write_int(10*(ph_set_value-(int)ph_set_value));
    LCD_writeStr("  "); 

    LCD_setCursor(1, 3);
    LCD_writeStr("PH deadband: ");
    Lcd_write_int((int)ph_deadband_value);
    LCD_writeStr("."); 
    Lcd_write_int(10*(ph_deadband_value-(int)ph_deadband_value));
    LCD_writeStr("  ");
}

void Display_nutrient_ratio_settings(int NuA, int NuB)
{
    LCD_setCursor(1, 0);
    LCD_writeStr("Nutrient A: "); 
    Lcd_write_int(NuA);
    LCD_writeStr("   "); 

    LCD_setCursor(1, 1);
    LCD_writeStr("Nutrient B: ");
    Lcd_write_int(NuB);
    LCD_writeStr("   "); 

}


/*---------------------------------Tasks-------------------------------------------------*/
void Button_Task( void * pvParameters )
{
    while(1)
    {
        EventBits_t uxBits = xEventGroupWaitBits(Button_event_group, BIT_LONG_PRESS|BIT_SHORT_PRESS|BIT_NORMAL_PRESS, pdTRUE, pdFALSE, portMAX_DELAY);

        if(uxBits & BIT_SHORT_PRESS)
        {
            // printf("Short Press! \n");
            short_press=1;
        }


        // else if(uxBits & BIT_NORMAL_PRESS)
        // {
        //     printf("Normal Press! \n");

        // }
        // else if(uxBits & BIT_LONG_PRESS)
        // {
        //     printf("Long Press! \n");
        // }

    }
}

void Read_sensor_task( void * pvParameters)
{
    init_adc1();

    //DS18B20 on GPIO 4
    ds18b20_init(4);
    float last_temp = 29, last_tds = 300, last_ph = 5;

    while(1)
    {
        xSemaphoreTake( Sensor_Semaphore, portMAX_DELAY );
        
        //Read temperature value
        temp_value = ds18b20_get_temp();
        if(temp_value > 50) temp_value = last_temp; else last_temp = temp_value;
        
        tds_value = read_tds_sensor(adc_read_tds_sensor_voltage(), temp_value, k_value);
        ph_value = read_ph_sensor(adc_read_ph_sensor_voltage(), Voltage_686, Voltage_401);

        
        if(ph_value > 14) ph_value = last_ph; else last_ph = ph_value;
        if(tds_value < 50 || tds_value > 3000) tds_value = last_tds; else last_tds = tds_value;
        
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
                vTaskDelay(50/portTICK_PERIOD_MS);
                output_io_set_level(LED, 0);
                vTaskDelay(5000/portTICK_PERIOD_MS);

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
                vTaskDelay(50/portTICK_PERIOD_MS);
                output_io_set_level(LED, 0);
                vTaskDelay(500/portTICK_PERIOD_MS);
                output_io_set_level(LED, 1);
                vTaskDelay(50/portTICK_PERIOD_MS);
                output_io_set_level(LED, 0);

                vTaskDelay(1000/portTICK_PERIOD_MS);

                if(!sd_card_status)
                {
                    output_io_set_level(LED, 1);
                    vTaskDelay(1000/portTICK_PERIOD_MS);
                    output_io_set_level(LED, 0);   
                }
                vTaskDelay(4000/portTICK_PERIOD_MS);
                break;
            default:
                break;
        }
        

    }
}

void Mqtt_communication_task( void * pvParameters)
{
    int year, month, day, hour, minute, second;
    float temp;

    TickType_t xLastWakeTime;
    const TickType_t xTime = 60*1000/portTICK_RATE_MS;
    xLastWakeTime = xTaskGetTickCount();

    mqtt_app_start();
    
    while(1)
    {
        xSemaphoreTake( Sensor_Semaphore, portMAX_DELAY );
        Data myData = {ph_value, tds_value, temp_value, ph_set_value, tds_set_value, ph_deadband_value, tds_deadband_value, Nutri_A, Nutri_B, Nu_ratio, Acid_So, Base_So, Water, change_nutri};

        //Get time from RTC
        if(getClock(&year, &month, &day, &hour, &minute, &second, &temp) == ESP_OK) 
        {
            ESP_LOGI(TAG, "Time: %04d-%02d-%02d %02d:%02d:%02d, %.2f deg Cel", year, month, day, hour, minute, second, temp);                             
        }

        time_Data mytimeData = {year, month, day, hour, minute, second};
        
        //Publish data to MQTT broker
        mqtt_publish_data(data_buff, GenData(myData, mytimeData, 0));

        //Write log to SD card
        sd_card_write_file(creat_file_name(day), (hour == 0 && minute <2 ) ? 1 : 0, GenData(myData, mytimeData, 1));
        
        Nutri_A = 0; Nutri_B = 0; Acid_So = 0; Base_So = 0; Water = 0; change_nutri = 0;
        
        xSemaphoreGive(Sensor_Semaphore);
        vTaskDelayUntil(&xLastWakeTime, xTime );    
        
    }
}

void Settings_display_task( void * pvParameters)
{
    //Custom character arrow
    uint8_t arrow_char[8] = {0x00, 0x04, 0x06, 0x1F, 0x06, 0x04, 0x00};
    //Custom character degree symbol
    uint8_t degree_char[8] = {0x07, 0x05, 0x07, 0x00, 0x00, 0x00, 0x00};
    
    //Init LCD I2C
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);

    LCD_createCustomCharacter(degree_char, 1);
    LCD_createCustomCharacter(arrow_char, 2);

    // Init the rotary encoder device with the GPIOs for A and B signals
    rotary_encoder_info_t info = { 0 };
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ENABLE_HALF_STEPS));

    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));


    // Create a queue for events from the rotary encoder driver.
    // Tasks can read from this queue to receive up to date position information.
    QueueHandle_t event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));
    rotary_encoder_event_t event = { 0 };
    static float t_tds = 0, t_ph = 0, t_temp = 0;
    
    //Timeout for settings menu
    TickType_t xStart, xStart_Vol, timeOut=6000;
    
    //Read config values from flash
    read_config_value_from_flash();
    
    bip(2);
    
    while(1)
    {
        if(tds_value != t_tds || ph_value != t_ph || temp_value != t_temp)
        {
            xSemaphoreTake(Screen_Semaphore, portMAX_DELAY);
            Display_Sensor_Data(FIRMWARE_VERSION, tds_value, tds_set_value, ph_value, ph_set_value, temp_value); 
            t_tds = tds_value; 
            t_ph = ph_value; 
            t_temp = temp_value;
            xSemaphoreGive(Screen_Semaphore);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
            
        if(short_press)
        {
            bip(1);
            short_press=0;
            LCD_clearScreen();
            int select = 0;
            xSemaphoreTake( Sensor_Semaphore, portMAX_DELAY );
            
            
            first_screen:
            
            LCD_setCursor(1, 0);
            LCD_writeStr("Sensors Calibration"); 
            LCD_setCursor(1, 1);
            LCD_writeStr("Value Settings  ");
            LCD_setCursor(1, 2);
            LCD_writeStr("Config Wifi");
            LCD_setCursor(1, 3);
            LCD_writeStr("Device ID");

            goto next;

            second_screen:
            LCD_setCursor(1, 0);
            LCD_writeStr("Update Firmware"); 
            LCD_setCursor(1, 1);
            LCD_writeStr("Factory Reset");
            LCD_setCursor(1, 2);
            LCD_writeStr("About");
            LCD_setCursor(1, 3);
            LCD_writeStr("Restart");
            
            next:
            xStart = xTaskGetTickCount();
            while(1)
            {
                if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
               
                Display_Settings_1(select %4);
                if (xQueueReceive(event_queue, &event, 1/portTICK_RATE_MS) == pdTRUE)
                {
                    event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                    if(select<0) select=0;

                    if((select % 6) ==4)
                    {
                        LCD_clearScreen();
                        goto second_screen;

                    }
                    else if( (select%4==0) && (select>0) ) 
                    {
                        select =0;
                        LCD_clearScreen();
                        goto first_screen;
                    }
                    if(select==3) goto first_screen;

                }   
                if(short_press) 
                {
                    if(select%8 !=7) bip(1);
                    short_press = 0;
                    select = select%8;
                    LCD_clearScreen();
                    break;
                }
                if(long_press)
                {
                    vTaskDelay(100/portTICK_PERIOD_MS);
                    long_press = 0;
                    short_press=0;
                    LCD_clearScreen();
                    goto exit;
                }

            }
            xStart = xTaskGetTickCount();
            switch (select)
            {
                //Sensors Calibration
                case 0:
                    LCD_setCursor(1, 0);
                    LCD_writeStr("TDS Sensor");
                    LCD_setCursor(1, 1);
                    LCD_writeStr("PH Sensor");
                    LCD_setCursor(1, 2);
                    LCD_writeStr("Pumps test");
                    select =0;
                    while(1)
                    {
                        if((xTaskGetTickCount() - xStart) > timeOut) goto exit;

                        Display_Settings_1(select);
                        if (xQueueReceive(event_queue, &event, 1/portTICK_RATE_MS) == pdTRUE)
                        {
                            event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                            if(select<0 || select>2) select=0;
                        
                        }   
                        if(short_press) 
                        {
                            bip(1);
                            short_press = 0;
                            LCD_clearScreen();
                            break;
                        }
                        
                    }

                    //Test pumps
                    if(select==2)
                    {
                        test_pump:
                        LCD_clearScreen();
                        LCD_setCursor(1, 0);
                        LCD_writeStr("A nutrient pump");
                        LCD_setCursor(1, 1);
                        LCD_writeStr("B nutrient pump");
                        LCD_setCursor(1, 2);
                        LCD_writeStr("Acid pump"); 
                        LCD_setCursor(1, 3);
                        LCD_writeStr("Base pump"); 
                        select = 0;
                        while(1)
                        {
                            if((xTaskGetTickCount() - xStart) > timeOut) goto exit;

                            Display_Settings_1(select);
                            if (xQueueReceive(event_queue, &event, 1/portTICK_RATE_MS) == pdTRUE)
                            {
                                event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                                if(select<0 || select>3) select=0;
                            
                            }   
                            if(short_press) 
                            {
                                bip(1);
                                short_press = 0;
                                LCD_clearScreen();
                                break;
                            }
                            if(long_press)
                            {
                                long_press = 0;
                                goto exit;
                            }
                            
                        }
                        LCD_writeStr("Testing...");
                        switch(select)
                        {
                            case 0:
                                output_io_set_level(Pum_NutriA, 1);  
                                vTaskDelay(5000/portTICK_PERIOD_MS);
                                output_io_set_level(Pum_NutriA, 0);  
                                goto test_pump;
                            case 1:
                                output_io_set_level(Pum_NutriB, 1);  
                                vTaskDelay(5000/portTICK_PERIOD_MS);
                                output_io_set_level(Pum_NutriB, 0);  
                                goto test_pump;
                            case 2:
                                output_io_set_level(Pum_Acid, 1);  
                                vTaskDelay(5000/portTICK_PERIOD_MS);
                                output_io_set_level(Pum_Acid, 0);  
                                goto test_pump;
                            case 3:
                                output_io_set_level(Pum_Base, 1);  
                                vTaskDelay(5000/portTICK_PERIOD_MS);
                                output_io_set_level(Pum_Base, 0);  
                                goto test_pump;
                            default:
                                goto test_pump;
                        }


                    }
                    //PH sensor
                    else if(select==1)
                    {
                        int8_t count;
                        int8_t first_solution = 0;

                        int8_t solution_type=0;
                        
                        first_ph:
                        count=1;
                        
                        second_ph:
                        xStart = xTaskGetTickCount();
                        xStart_Vol = xTaskGetTickCount();
                        if(count == 1) 
                        {
                            LCD_setCursor(3, 0);
                            LCD_writeStr("First Solution");
                        }
                        else if(count == 2) 
                        {
                            LCD_setCursor(2, 0);
                            LCD_writeStr("Second Solution");
                        }
                        LCD_setCursor(1, 1);
                        LCD_writeStr("Voltage:");
                        LCD_setCursor(1, 2);
                        LCD_writeStr("Auto Detect:");
                        
                        while(1)
                        {
                            
                            if((xTaskGetTickCount() - xStart) > timeOut*10) goto exit;

                            if((xTaskGetTickCount() - xStart_Vol) > 50)
                            {
                                xStart_Vol = xTaskGetTickCount();
                                
                                ph_vol = (int)adc_read_ph_sensor_voltage();
                                LCD_setCursor(9, 1);
                                Lcd_write_int(ph_vol);
                                LCD_writeStr(" mV    ");
                                
                                LCD_setCursor(13, 2);
                                if(ph_vol>1500  && ph_vol<1800) 
                                {
                                    LCD_writeStr("6.86   ");
                                    solution_type = 1;
                                }
                                else if(ph_vol>2000  && ph_vol<2300) 
                                {
                                    LCD_writeStr("4.01   ");
                                    solution_type = 2;
                                }
                                else 
                                {
                                    LCD_writeStr("Unknown");
                                    solution_type = 0;
                                }
  
                            }
                            if(long_press) 
                            {         
                               long_press=0;
                                goto exit; 
                            }  
                            
                            if(short_press) 
                            {            
                                short_press = 0;
                                vTaskDelay(100/portTICK_PERIOD_MS);
                                short_press = 0;


                                LCD_clearScreen();
                                if(count == 1) 
                                {
                                    
                                    if(solution_type == 0) 
                                    {
                                        bip(3);
                                        goto first_ph;
                                    }
                                    else if(solution_type == 1) Voltage_686 = ph_vol;
                                    else if(solution_type == 2) Voltage_401 = ph_vol;
                                    bip(1);
                                    first_solution = solution_type;

                                    count=2;
                                    goto second_ph;
                                }
                                else if(count == 2)
                                {
                                    if(solution_type == 0 || first_solution == solution_type) 
                                    {
                                        bip(3);
                                        goto second_ph;
                                    }
                                    else if(solution_type == 1) Voltage_686 = ph_vol;
                                    else if(solution_type == 2) Voltage_401 = ph_vol;
                                   
                                    write_config_value_to_flash(tds_set_value, tds_deadband_value, ph_set_value, ph_deadband_value, Voltage_686, Voltage_401, k_value, Nu_ratio);

                                    break;
                                }
                            }   
                        }
                    }
                    
                    //TDS sensor 
                    else if(select==0)
                    {
                        LCD_setCursor(1, 0);
                        LCD_writeStr("342 ppm Solution");
                        LCD_setCursor(1, 1);
                        LCD_writeStr("1000 ppm Solution");
                        select =0;
                        xStart = xTaskGetTickCount();
                        static float temp_k_value;
                        int temp_tds_value;
                        int solution;
                        while(1)
                        {
                            if((xTaskGetTickCount() - xStart) > timeOut) goto exit;

                            Display_Settings_1(select);
                            if (xQueueReceive(event_queue, &event, 1/portTICK_RATE_MS) == pdTRUE)
                            {
                                event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                                if(select<0 || select>1) select=0;
                            
                            }   
                            if(short_press) 
                            {
                                bip(1);
                                short_press =0;
                                LCD_clearScreen();
                                break;
                            }
                        }

                        //select =0: 342 ppm Solution, select =1: 1000 ppm Solution
                        select == 1 ?  solution = 1000 : (solution = 342);
              
                        LCD_setCursor(1, 0);
                        Lcd_write_int(solution);
                        LCD_writeStr(" ppm Solution");
                        LCD_setCursor(1, 1);
                        LCD_writeStr("TDS read: "); 
                        LCD_setCursor(1, 2);
                        LCD_writeStr("K value: "); 
                        xStart = xTaskGetTickCount();
                        xStart_Vol = xTaskGetTickCount();
                        
                        while(1)
                        {
                            if((xTaskGetTickCount() - xStart) > timeOut*10) goto exit;
                        
                            if((xTaskGetTickCount() - xStart_Vol) > 50)
                            {
                                xStart_Vol = xTaskGetTickCount();
                                
                                LCD_setCursor(11, 1);
                                temp_tds_value =  read_tds_sensor(adc_read_tds_sensor_voltage(), temp_value, k_value);
                                Lcd_write_int(temp_tds_value);
                                LCD_writeStr(" ppm   ");
                                LCD_setCursor(10, 2);
                                temp_k_value = solution/(temp_tds_value/k_value);
                                
                                Lcd_write_int((int)temp_k_value);
                                LCD_writeStr("."); 
                                Lcd_write_int(1000*(temp_k_value-(int)temp_k_value));
                                
                            }
                            
                            if(long_press) 
                            {         
                                long_press=0;
                                goto exit; 
                            } 
                            
                            if(short_press)
                            {
                                vTaskDelay(100/portTICK_PERIOD_MS);
                                short_press =0;
                                k_value = temp_k_value;
                                write_config_value_to_flash(tds_set_value, tds_deadband_value, ph_set_value, ph_deadband_value, Voltage_686, Voltage_401, k_value, Nu_ratio);
                                break;
                            } 
                        }
                            
                    }
                    break;
                
                //Value Settings
                case 1:  
                    LCD_setCursor(1, 0);
                    LCD_writeStr("TDS and PH values");
                    LCD_setCursor(1, 1);
                    LCD_writeStr("Nutrient Ratio");
                    LCD_setCursor(1, 2);
                    LCD_writeStr("Change nutrient");
                    select =0;
                    while(1)
                    {
                        if((xTaskGetTickCount() - xStart) > timeOut) goto exit;

                        Display_Settings_1(select);
                        if (xQueueReceive(event_queue, &event, 1/portTICK_RATE_MS) == pdTRUE)
                        {
                            event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                            if(select<0 || select>2) select=0;
                        
                        }   
                        if(short_press) 
                        {
                            bip(1);
                            vTaskDelay(100/portTICK_PERIOD_MS);
                            short_press = 0;
                            LCD_clearScreen();
                            break;
                        }
                        
                    }
                    //Save change nutrient notification for next data sending
                    if(select ==2 )
                    {
                        LCD_writeStr("Notification saved!");
                        change_nutri = 1;
                        vTaskDelay(2000/portTICK_PERIOD_MS);
                        goto exit;
                    }
                    else if(select == 0)
                    {
                        a:   
                        Display_sensors_settings();
                        select=0;
                        while(1)
                        {
                            if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
                            Display_Settings_1(select %4);
                            if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                            {
                                event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                                if(select<0) select=4;
                            }

                            if(short_press) 
                            {
                                bip(1);
                                short_press = 0;
                                select = select%4;
                                xStart = xTaskGetTickCount();
                                switch (select)
                                {
                                    //TDS set value
                                    case 0:
                                        Display_Settings_1(0);          
                                        while(1)
                                        {
                                            if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
                                            Display_sensors_settings();
                                            if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                            {
                                                event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? tds_set_value+=10 : (tds_set_value-=10);
                                                if(tds_set_value<100) tds_set_value=100;
                                            }
                                            if(short_press)
                                            {
                                                bip(1);
                                                short_press=0;
                                                break;
                                            }

                                        }
                                        break;
                                    
                                    //TDS deadband value
                                    case 1:
                                        Display_Settings_1(1);
                                        while(1)
                                        {
                                            if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
                                            Display_sensors_settings();
                                            if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                            {
                                                event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? tds_deadband_value+=5 : (tds_deadband_value-=5);
                                                if(tds_deadband_value<50) tds_deadband_value=50;
                                                if(tds_deadband_value>300) tds_deadband_value=300;
                                            }
                                            if(short_press)
                                            {
                                                bip(1);
                                                short_press=0;
                                                break;
                                            }

                                        }
                                        break;
                                    //PH set value
                                    case 2:
                                        Display_Settings_1(2);
                                        while(1)
                                        {
                                            if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
                                            Display_sensors_settings();
                                            if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                            {
                                                event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? ph_set_value+=0.1 : (ph_set_value-=0.1);
                                                if(ph_set_value<0) ph_set_value=0;
                                                if(ph_set_value>14) ph_set_value=14;
                                            }
                                            if(short_press)
                                            {
                                                bip(1);
                                                short_press=0;
                                                break;
                                            }

                                        }
                                        break;
                                    //PH deadband value
                                    case 3:
                                        Display_Settings_1(3);
                                        while(1)
                                        {
                                            if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
                                            Display_sensors_settings();
                                            if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                            {
                                                event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? ph_deadband_value+=0.1 : (ph_deadband_value-=0.1);
                                                if(ph_deadband_value<0) ph_deadband_value=0.1;
                                                if(ph_deadband_value>1) ph_deadband_value=1;
                                            }
                                            if(short_press)
                                            {
                                                bip(1);
                                                short_press=0;
                                                break;
                                            }

                                        }
                                        break;
                                }
                                LCD_clearScreen();
                                goto a;

                            }
                            if(long_press)
                            {
                                vTaskDelay(100/portTICK_PERIOD_MS);
                                long_press=0;
                                short_press=0;
                                write_config_value_to_flash(tds_set_value, tds_deadband_value, ph_set_value, ph_deadband_value, Voltage_686, Voltage_401, k_value, Nu_ratio);
                                vTaskDelay(1000/portTICK_PERIOD_MS);
                                short_press=0;
                                LCD_clearScreen();
                                goto exit;
                            }

                        }

                    }
                    else if(select==1)
                    //Nutrient ratio
                    {
                        int NuA = 1, NuB = 1;
                        loop1:   
                        Display_nutrient_ratio_settings(NuA, NuB);
                        select =0;
                        while(1)
                        {
                            if((xTaskGetTickCount() - xStart) > timeOut) goto exit;

                            Display_Settings_1(select);
                            if (xQueueReceive(event_queue, &event, 1/portTICK_RATE_MS) == pdTRUE)
                            {
                                event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                                if(select<0 || select>1) select=0;
                            
                            }   
                            if(short_press) 
                            {
                                bip(1);
                                vTaskDelay(100/portTICK_PERIOD_MS);
                                short_press = 0;
                                LCD_clearScreen();
                                break;
                            }
                            if(long_press)
                            {
                                long_press = 0;
                                Nu_ratio = (float)NuA/NuB;
                                write_config_value_to_flash(tds_set_value, tds_deadband_value, ph_set_value, ph_deadband_value, Voltage_686, Voltage_401, k_value, Nu_ratio);
                                bip(10);
                                LCD_clearScreen();
                                
                                //Resatart to load new value
                                esp_restart();
                            }

                            
                        }
                        if(!select)
                        {
                            Display_Settings_1(0);          
                            while(1)
                            {

                                if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
                                Display_nutrient_ratio_settings(NuA, NuB);
                                if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                {
                                    event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? NuA+=1 : (NuA-=1);
                                    if(NuA<1) NuA=1;
                                    if(NuA>100) NuA=100;
                                }
                                if(short_press)
                                {
                                    bip(1);
                                    short_press=0;
                                    LCD_clearScreen();
                                    goto loop1;
                                    break;
                                }

                            }
                        }
                        else
                        {
                            Display_Settings_1(1);          
                            while(1)
                            {

                                if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
                                Display_nutrient_ratio_settings(NuA, NuB);
                                if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                {
                                    event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? NuB+=1 : (NuB-=1);
                                    if(NuB<1) NuB=1;
                                    if(NuB>100) NuB=100;
                                }
                                if(short_press)
                                {
                                    bip(1);
                                    short_press=0;
                                    LCD_clearScreen();
                                    goto loop1;
                                    break;
                                }

                            }
                        }


                        

                    }

                    break;
                
                //Config Wifi
                case 2:
                    LCD_setCursor(7, 0);
                    LCD_writeStr("Wifi");
                    LCD_setCursor(2, 1);
                    LCD_writeStr("is configuring...");
                    esp_err_t t = mqtt_app_stop();
                    if( t == ESP_OK)
                        start_smart_config();
                   
                    while(led_state==3) 
                    {
                        vTaskDelay(10/portTICK_PERIOD_MS);
                        if((xTaskGetTickCount() - xStart) > timeOut) goto fail;
                        if(long_press)
                        {
                            vTaskDelay(100/portTICK_PERIOD_MS);
                            long_press=0;
                            goto fail;
                        }
                    }
                    LCD_setCursor(0, 0);
                    if(led_state==1) 
                    {
                        LCD_clearScreen();
                        LCD_writeStr("      Success!       ");   
                        vTaskDelay(3000/portTICK_PERIOD_MS);
                    }
                    else if(led_state==2)
                    {
                        fail:
                        LCD_clearScreen();
                        LCD_setCursor(8, 0);
                        LCD_writeStr("Fail!");  
                        LCD_setCursor(5, 2);
                        LCD_writeStr("Restarting...");
                        vTaskDelay(3000/portTICK_PERIOD_MS);
                        esp_restart();
                    } 
                    LCD_clearScreen();
                    break;  
                
                //Device ID
                case 3:
                    LCD_setCursor(6, 0);
                    LCD_writeStr("Device ID");
                    LCD_setCursor(4, 1);
                    for(int i=0;i<12;i++) LCD_writeChar(mac_buff[i]);
                    while(1)
                    {
                        if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
                        if(short_press)
                        {
                            short_press=0;
                            LCD_clearScreen();
                            break;
                        }
                        vTaskDelay(10/portTICK_PERIOD_MS);
                    }
                    break;
                
                //Update Firmware
                case 4:
                    LCD_clearScreen();
                    mqtt_app_stop();
                    OTA_start();
                    while(1)
                    {
                        switch (OTA_status)
                        {
                            case 1:
                                LCD_clearScreen();
                                LCD_setCursor(1, 0);
                                LCD_writeStr("Looking for a new");
                                LCD_setCursor(5, 1);
                                LCD_writeStr("firmware...");
                                while(OTA_status==1) vTaskDelay(10/portTICK_PERIOD_MS);
                                break;
                            
                            case 2: 
                                LCD_clearScreen();
                                LCD_setCursor(4, 0);
                                LCD_writeStr("New firmware    ");
                                LCD_setCursor(6, 1);
                                LCD_writeStr("available!");
                                while(OTA_status==2) vTaskDelay(10/portTICK_PERIOD_MS);
                                break;

                            case 3: 
                                LCD_clearScreen();
                                LCD_setCursor(3, 0);
                                LCD_writeStr("Downloading ");
                                LCD_setCursor(3, 1);
                                LCD_writeStr("and installing ");
                                LCD_setCursor(3, 2);
                                LCD_writeStr("new firmware...");   

                                while(OTA_status==3) vTaskDelay(10/portTICK_PERIOD_MS);
                                break;

                            case 4:
                                LCD_clearScreen();
                                LCD_setCursor(3, 0);
                                LCD_writeStr("New firmware");
                                LCD_setCursor(3, 1);
                                LCD_writeStr("not available!");
                                LCD_setCursor(4, 3);
                                LCD_writeStr("Aborting...");
                                vTaskDelay(3000/portTICK_PERIOD_MS);
                                goto out;

                            case 5:
                                LCD_clearScreen();
                                LCD_setCursor(4, 0);
                                LCD_writeStr("Successfully");
                                LCD_setCursor(6, 1);
                                LCD_writeStr("updated!");
                                LCD_setCursor(3, 3);
                                LCD_writeStr("Restarting...");
                                vTaskDelay(3000/portTICK_PERIOD_MS);
                                goto out;
                            case 6:
                                LCD_clearScreen();
                                LCD_setCursor(0, 0);
                                LCD_writeStr("Update fail!");
                                LCD_setCursor(0, 1);
                                LCD_writeStr("Aborting...");
                                vTaskDelay(3000/portTICK_PERIOD_MS);
                                goto out;

                            default:
                                break;
                        }
                        vTaskDelay(10/portTICK_PERIOD_MS);
                    }
                    out:
                    OTA_status =0;
                    LCD_clearScreen();
                    break;
                
                //About
                case 6:
                    LCD_clearScreen();
                    LCD_setCursor(4, 0);
                    LCD_writeStr("Dosing System"); 
                    LCD_setCursor(1, 1);
                    LCD_writeStr("Version: "); 
                    
                    LCD_writeChar((int)FIRMWARE_VERSION + 0x30);
                    LCD_writeStr("."); 
                    LCD_writeChar(10*(FIRMWARE_VERSION-(int)FIRMWARE_VERSION)+0x30);
                    
                    LCD_setCursor(1, 2);
                    LCD_writeStr("HCMUT Final Project"); 
                    LCD_setCursor(1, 3);
                    LCD_writeStr("Tel: 0388161923"); 

                    while(1)
                    {
                        if((xTaskGetTickCount() - xStart) > timeOut) goto exit;
                        if(short_press)
                        {
                            short_press=0;
                            LCD_clearScreen();
                            break;
                        }
                        vTaskDelay(10/portTICK_PERIOD_MS);
                    }
                    break;

                //Restart
                case 7:
                    LCD_clearScreen();
                    LCD_setCursor(3, 0);
                    LCD_writeStr("Restart device?");
                    LCD_setCursor(1, 1);
                    LCD_writeStr("YES");
                    LCD_setCursor(1, 2);
                    LCD_writeStr("NO");

                    select =2;
                    while(1)
                    {
                        if((xTaskGetTickCount() - xStart) > timeOut) goto exit;

                        Display_Settings_1(select);
                        if (xQueueReceive(event_queue, &event, 1/portTICK_RATE_MS) == pdTRUE)
                        {
                            event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                            if(select<1 || select>2) select=1;
                        
                        }   
                        if(short_press) 
                        {
                            short_press = 0;
                            LCD_clearScreen();
                            if(select==2) goto exit; else
                            {
                                bip(1);
                                break;
                            }
                        }

                    }
                    LCD_clearScreen();
                    LCD_setCursor(2, 1);
                    LCD_writeStr("Restarting...");
                    vTaskDelay(2000/portTICK_PERIOD_MS);
                    esp_restart();

                    break;
                
                //Factory reset
                case 5:
                    LCD_clearScreen();
                    LCD_setCursor(3, 0);
                    LCD_writeStr("Are you sure?");
                    LCD_setCursor(1, 1);
                    LCD_writeStr("YES");
                    LCD_setCursor(1, 2);
                    LCD_writeStr("NO");

                    select =2;
                    while(1)
                    {
                        if((xTaskGetTickCount() - xStart) > timeOut) goto exit;

                        Display_Settings_1(select);
                        if (xQueueReceive(event_queue, &event, 1/portTICK_RATE_MS) == pdTRUE)
                        {
                            event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                            if(select<1 || select>2) select=1;
                        
                        }   
                        if(short_press) 
                        {
                            
                            short_press = 0;
                            LCD_clearScreen();
                            if(select==2) goto exit; else
                            {
                                bip(1);
                                break;
                            }
                        }
                    }
                    
                    //Search the factory partition in flash memmory
                    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
                    
                    //Partition found, set it to boot partition and reset the esp
                    if(pi != NULL)
                    {
                        const esp_partition_t* factory = esp_partition_get(pi);
                        esp_partition_iterator_release(pi);
                        if(esp_ota_set_boot_partition(factory) == ESP_OK)
                        {
                            LCD_setCursor(1, 0);
                            LCD_writeStr("Reset successfuly!"); 
                            LCD_setCursor(1, 2);
                            LCD_writeStr("Restarting..."); 
                            vTaskDelay(3000/portTICK_PERIOD_MS);
                            esp_restart();
                        }

                    }
                    break;
  
            }
            exit:
            select=0;  
            LCD_clearScreen();
            bip(10);
            xSemaphoreGive(Sensor_Semaphore);
            short_press=0;
        }
       
    }
}

void Pum_ctr_task(void * pvParameters)
{
    TickType_t xLastWakeTime, pump_time, time_pump_A, time_pump_B, time_pump_Acid_Base = Time_1_pump;
    if(Nu_ratio <=1 )
    {
        time_pump_A = Time_1_pump;
        time_pump_B = (int)(time_pump_A/Nu_ratio);
    }
    else
    {
        time_pump_B = Time_1_pump;
        time_pump_A = (int)(time_pump_B*Nu_ratio);
    }
    const TickType_t xTime = 20*1000/portTICK_PERIOD_MS;
    xLastWakeTime = xTaskGetTickCount();
    
    while(1)
    {
        xSemaphoreTake( Sensor_Semaphore, portMAX_DELAY );
        vTaskDelay(1);
        if((tds_set_value - tds_value) > tds_deadband_value)
        {
            
            output_io_set_level(Pum_NutriA, 1);
            output_io_set_level(Pum_NutriB, 1);
            pump_time = xTaskGetTickCount();
            bool FlagA = 0, FlagB = 0;
            while(1)
            {
                if(xTaskGetTickCount() - pump_time > time_pump_A  && !FlagA)
                {
                    output_io_set_level(Pum_NutriA, 0);
                    FlagA = 1;
                    if(time_pump_A > time_pump_B) 
                    {     
                        Nutri_A += (int)(Pum_liquid*Nu_ratio);
                        printf("NutriA : %d\n",Nutri_A);
                        break;
                    }
                    else 
                    {
                        Nutri_A += Pum_liquid;
                        printf("NutriA : %d\n", Nutri_A);
                    }

                }
                
                if(xTaskGetTickCount() - pump_time > time_pump_B && !FlagB)
                {
                    output_io_set_level(Pum_NutriB, 0);
                    FlagB = 1;
                    if(time_pump_B > time_pump_A) 
                    {     
                        Nutri_B += (int)(Pum_liquid/Nu_ratio);
                        printf("NutriB : %d\n", Nutri_B);
                        break;
                    }
                    else 
                    {
                        Nutri_B += Pum_liquid; 
                        printf("NutriB : %d\n", Nutri_B);
                    }
                    
                }

                if(FlagA && FlagB) break;
            }
                  
        }
        else if(ph_value - ph_set_value > ph_deadband_value) //PH > PH_set, needed Acid solution to decrease PH
        {
            output_io_set_level(Pum_Acid, 1);
            pump_time = xTaskGetTickCount();
            while(1)
            {
                if(xTaskGetTickCount() - pump_time > time_pump_Acid_Base)
                {
                    output_io_set_level(Pum_Acid, 0);
                    Acid_So += Pum_liquid;
                    printf("Acid So : %d\n", Acid_So);
                    break;
                }
            }

        }
        else if(ph_set_value - ph_value > ph_deadband_value) //PH < PH_set, needed Base solution to increase PH
        {
            output_io_set_level(Pum_Base, 1);
            pump_time = xTaskGetTickCount();
            while(1)
            {
                if(xTaskGetTickCount() - pump_time > time_pump_Acid_Base)
                {
                    output_io_set_level(Pum_Base, 0);
                    Base_So += Pum_liquid;
                    printf("Base So : %d\n", Base_So);
                    break;
                }
            }
        }

        xSemaphoreGive(Sensor_Semaphore);
        vTaskDelayUntil(&xLastWakeTime, xTime );    
    }
    
}

void Water_Pum_ctr_task(void *pvParameters)
{
    bool Water_Flag = 0;
    while(1)
    {
        //Water pump control
        if(input_io_get_level(Lim_Sen_High) && input_io_get_level(Lim_Sen_low)) 
        {
            output_io_set_level(Pum_Water, 0);
            
            if(Water_Flag) 
            {
                Water = Volume_liquid_water; 
                printf("High/Low: %d - %d\n", input_io_get_level(Lim_Sen_High), input_io_get_level(Lim_Sen_low));
                printf("Water pump off\n");
            }
            Water_Flag = 0;
        }
        else if(!input_io_get_level(Lim_Sen_High) && !input_io_get_level(Lim_Sen_low)) 
        {
            output_io_set_level(Pum_Water, 1);
            printf("High/Low: %d - %d\n", input_io_get_level(Lim_Sen_High), input_io_get_level(Lim_Sen_low));
            printf("Water pump on\n");
            Water_Flag = 1;
        }
        else if(!input_io_get_level(Lim_Sen_High) && input_io_get_level(Lim_Sen_low)) 
        {
            if(input_io_get_level(Pum_Water)) output_io_set_level(Pum_Water, 1);
            else output_io_set_level(Pum_Water, 0);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}



void app_main(void)
{
    //Init NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
 
    //Init wifi
    initialise_wifi();

    //init sttp for update time from internet
    initialize_sntp();

    //init SD card
    if(init_sd_card() !=ESP_OK) 
        ESP_LOGE(TAG, "Init sd card fail");
    else sd_card_status = 1;

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
    output_io_create(LED);

    //init I2C master
    i2c_master_init(I2C_NUM_0, SDA_PIN, SCL_PIN);

    //Init buzzer
    output_io_create(BUZZ);

    //Init 4 peristaltic pump
    output_io_create(Pum_NutriA);
    output_io_create(Pum_NutriB);
    output_io_create(Pum_Acid);
    output_io_create(Pum_Base);

    //Init water pump and 2 level sensor
    output_io_create(Pum_Water);
    output_io_set_level(Pum_Water, 0);

    gpio_pad_select_gpio(Lim_Sen_High);
    gpio_set_direction(Lim_Sen_High, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(Lim_Sen_low);
    gpio_set_direction(Lim_Sen_low, GPIO_MODE_INPUT);


    //Tasks for smart config
    xTaskCreate(Led_task, "LED_task", 2048, NULL, 3, NULL);
    xTaskCreate(Button_Task, "ButtonTask", 2048, NULL, 3, NULL);

    //Timer for button, time out = 2s
    xTimers = xTimerCreate("Timmer_for_button_timeout", 2000/portTICK_PERIOD_MS, pdFALSE, (void *) 0, vTimerCallback);
    
    //Create mutex for sensor data value and screen displayment
    Sensor_Semaphore = xSemaphoreCreateMutex();
    Screen_Semaphore = xSemaphoreCreateMutex();
    
    
    //init sensors 
    xTaskCreate(Read_sensor_task, "Read_sensor_task", 2048, NULL, 3, NULL);

    //Settings and display sensor data to LCD
    xTaskCreate(Settings_display_task, "Settings_display_task", 4096, NULL, 3, NULL);

    //init and start MQTT communication
    xTaskCreate(Mqtt_communication_task, "Mqtt_communication_task", 4096, NULL, 3, NULL);
    
    //Get time from internet and save to DS3231 RTC every starting
    setClock();

    // Nutrient Pumbs control task
    xTaskCreate(Pum_ctr_task, "Pum_ctr_task", 4096, NULL, 3, NULL);

    //Water Pumbs control task
    xTaskCreate(Water_Pum_ctr_task, "Water_Pum_ctr_task", 4096, NULL, 3, NULL);


}
