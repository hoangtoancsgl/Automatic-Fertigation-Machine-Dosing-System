
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


static const char *TAG = "DOSING SYSTEM";

//EC11 encoder for settings
#define ROT_ENC_A_GPIO 13
#define ROT_ENC_B_GPIO 14

#define ENABLE_HALF_STEPS true  // Set to true to enable tracking of rotary encoder at half step resolution


//LCD 16x2
#define LCD_ADDR 0x27
#define SDA_PIN  26
#define SCL_PIN  25
#define LCD_COLS 20
#define LCD_ROWS 4


//Sensor data struct
typedef struct Data_Send
{
    float PH;
    int TDS;
    float temperature;
    
}Data;

//buffer for sensors data
char JSON_buff[100];

//buffer for device ID 
char mac_buff[12];
char data_buff[100];
char status_buff[100];


/*LED state for wifi connection:
    1 => CONNECTED to wifi
    2 => DISCONNECTED
    3 => SMART CONFIG
    4 => MQTT CONNECTED 
*/
uint8_t led_state = 2;
#define LED GPIO_NUM_2

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

//Sensors value
static float temp_value=0 , tds_value=0, ph_value=0;

//Sensors set value
float tds_set_value=500, ph_set_value=6.5;

//Sensors deadband value
float tds_deadband_value=100, ph_deadband_value=0.2;

extern double FIRMWARE_VERSION;


//Genarate JSON data from sensor output
char* GenData(Data myData)
{
    char str_PH[100];
    char str_TDS[100];
    char str_temp[100];
    
    for(int i=0;i<100;i++)
    {
        str_PH[i]=0;
        str_TDS[i]=0;
        str_temp[i]=0;
        
        JSON_buff[i]=0;
    }
    sprintf(str_PH, "%0.2f", myData.PH);
    sprintf(str_TDS, "%d", myData.TDS);
    sprintf(str_temp, "%0.1f", myData.temperature);
   
    //{"PH":"123","EC":"256", "TDS": "1", "temp":"23"}

    strcat(JSON_buff, "{\"PH\":\"");

    strcat(JSON_buff, str_PH);
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
        printf("Button 0 timeOut! \n");
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
    
    float tds_voltage, ph_voltage;
    while(1)
    {
        xSemaphoreTake( Sensor_Semaphore, portMAX_DELAY );
        tds_voltage = adc_read_tds_sensor();
        // printf("TDS voltage: %0.3f\n", tds_voltage);
        ph_voltage = adc_read_ph_sensor();
        temp_value = ds18b20_get_temp();
        // printf("Temperature: %0.1f\n\n", temp_value);

        float compensationCoefficient=1.0+0.02*(temp_value-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
        float compensationVolatge=tds_voltage/compensationCoefficient;  //temperature compensation

        tds_value = (133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value

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
    mqtt_app_start();
    while(1)
    {
        xSemaphoreTake( Sensor_Semaphore, portMAX_DELAY );
        Data myData = {ph_value, tds_value, temp_value};

        mqtt_publish_data(data_buff, GenData(myData));
        xSemaphoreGive(Sensor_Semaphore);
        vTaskDelay(5000/portTICK_RATE_MS);
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

    // Initialise the rotary encoder device with the GPIOs for A and B signals
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
    
    //Read config value from flash
    read_config_value_from_flash();
    
    while(1)
    {
        if(tds_value != t_tds || ph_value != t_ph || temp_value != t_temp)
        {
            Display_Sensor_Data(FIRMWARE_VERSION, tds_value, tds_set_value, ph_value, ph_set_value, temp_value); 
            t_tds = tds_value; 
            t_ph = ph_value; 
            t_temp = temp_value;
        }
        vTaskDelay(10/portTICK_PERIOD_MS);

        if(short_press)
        {
            short_press=0;
            LCD_clearScreen();
            int select = 0;
            xSemaphoreTake( Sensor_Semaphore, portMAX_DELAY );
            
            first_screen:
            
            LCD_setCursor(1, 0);
            LCD_writeStr("Sensors Calibration"); 
            LCD_setCursor(1, 1);
            LCD_writeStr("Sensors Settings  ");
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
            LCD_writeStr("Exit");
            
            next:
            while(1)
            {
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
                    short_press = 0;
                    select = select%8;
                    LCD_clearScreen();
                    break;
                }

            }
            switch (select)
            {
                //Sensors Calibration
                case 0:
                    // read_config_value_from_flash();
                    break;
                
                //Sensors Settings
                case 1:  
                    a:   
                    Display_sensors_settings();
                    select=0;
                    while(1)
                    {
                        Display_Settings_1(select %4);
                        if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                        {
                            event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? select++ : select--;
                            if(select<0) select=4;
                        }

                        if(short_press) 
                        {
                            short_press = 0;
                            select = select%4;
                            switch (select)
                            {
                                //TDS set value
                                case 0:
                                    Display_Settings_1(0);
                                    while(1)
                                    {
                                        Display_sensors_settings();
                                        if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                        {
                                            event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? tds_set_value+=10 : (tds_set_value-=10);
                                            if(tds_set_value<100) tds_set_value=100;
                                        }
                                        if(short_press)
                                        {
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
                                        Display_sensors_settings();
                                        if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                        {
                                            event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? tds_deadband_value+=5 : (tds_deadband_value-=5);
                                            if(tds_deadband_value<50) tds_deadband_value=50;
                                            if(tds_deadband_value>200) tds_deadband_value=200;
                                        }
                                        if(short_press)
                                        {
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
                                        Display_sensors_settings();
                                        if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                        {
                                            event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? ph_set_value+=0.1 : (ph_set_value-=0.1);
                                            if(ph_set_value<0) ph_set_value=0;
                                            if(ph_set_value>14) ph_set_value=14;
                                        }
                                        if(short_press)
                                        {
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
                                        Display_sensors_settings();
                                        if (xQueueReceive(event_queue, &event, 100/portTICK_RATE_MS) == pdTRUE)
                                        {
                                            event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? ph_deadband_value+=0.1 : (ph_deadband_value-=0.1);
                                            if(ph_deadband_value<0) ph_deadband_value=0.1;
                                            if(ph_deadband_value>0.5) ph_deadband_value=0.5;
                                        }
                                        if(short_press)
                                        {
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
                            long_press=0;
                            LCD_clearScreen();
                            write_config_value_to_flash(tds_set_value, tds_deadband_value, ph_set_value, ph_deadband_value);
                            vTaskDelay(1000/portTICK_PERIOD_MS);
                            short_press=0;
                            break;
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
                   
                    while(led_state==3) vTaskDelay(10/portTICK_PERIOD_MS);
                   
                    LCD_setCursor(0, 0);
                    if(led_state==1) 
                    {
                        LCD_clearScreen();
                        LCD_writeStr("      Success!       ");   
                        vTaskDelay(3000/portTICK_PERIOD_MS);
                    }
                    else if(led_state==2)
                    {
                        LCD_clearScreen();
                        LCD_setCursor(8, 0);
                        LCD_writeStr("Fail!");  
                        LCD_setCursor(3, 1);
                        LCD_writeStr("Restart device");
                        LCD_setCursor(3, 2);
                        LCD_writeStr("and try again!");
                        vTaskDelay(5000/portTICK_PERIOD_MS);

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
                                LCD_setCursor(4, 1);
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
                                LCD_setCursor(5, 3);
                                LCD_writeStr("Aborting...");
                                vTaskDelay(3000/portTICK_PERIOD_MS);
                                goto exit;

                            case 5:
                                LCD_clearScreen();
                                LCD_setCursor(4, 0);
                                LCD_writeStr("Successfully");
                                LCD_setCursor(6, 1);
                                LCD_writeStr("updated!");
                                LCD_setCursor(3, 3);
                                LCD_writeStr("Restarting...");
                                vTaskDelay(3000/portTICK_PERIOD_MS);
                                goto exit;
                            case 6:
                                LCD_clearScreen();
                                LCD_setCursor(0, 0);
                                LCD_writeStr("Update fail!");
                                LCD_setCursor(0, 1);
                                LCD_writeStr("aborting...");
                                vTaskDelay(3000/portTICK_PERIOD_MS);
                                goto exit;

                            default:
                                break;
                        }
                        vTaskDelay(10/portTICK_PERIOD_MS);
                    }
                    exit:
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
                        if(short_press)
                        {
                            short_press=0;
                            LCD_clearScreen();
                            break;
                        }
                        vTaskDelay(10/portTICK_PERIOD_MS);
                    }
                    break;
                //Factory reset
                case 5:
                    LCD_clearScreen();
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

            xSemaphoreGive(Sensor_Semaphore);
        }
       
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
    xTimers = xTimerCreate("Timmer_for_button_timeout", 3000/portTICK_PERIOD_MS, pdFALSE, (void *) 0, vTimerCallback);
    
    //Create mutex for sensor data
    Sensor_Semaphore = xSemaphoreCreateMutex();

    //init sensors 
    xTaskCreate(Read_sensor_task, "Read_sensor_task", 2048, NULL, 3, NULL);

    //init and start MQTT communication
    xTaskCreate(Mqtt_communication_task, "Mqtt_communication_task", 4096, NULL, 3, NULL);

    //Settings and display sensor data to LCD
    xTaskCreate(Settings_display_task, "Settings_display_task", 4096, NULL, 3, NULL);

}
