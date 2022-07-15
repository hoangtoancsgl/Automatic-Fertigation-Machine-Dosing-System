#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_tls.h"
#include "https_ota.h"

#include "../mqtt_client_app/mqtt_client_app.h"


#define UPDATE_JSON_URL		"http://hoangtoancsgl.000webhostapp.com/firmware.json"


static const char *TAG = "OTA_UPDATE";
double FIRMWARE_VERSION = 3.0;
// server certificates https://
extern const char server_cert_pem_start[] asm("_binary_certs_pem_start");
extern const char server_cert_pem_end[] asm("_binary_certs_pem_end");

// receive buffer
char rcv_buffer[200];

extern uint8_t OTA_status;

// esp_http_client event handler
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    
	switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
				strncpy(rcv_buffer, (char*)evt->data, evt->data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            break;
        case HTTP_EVENT_DISCONNECTED:
            break;
    }
    return ESP_OK;
}


// Check update task
// downloads every 30sec the json file with the latest firmware
void Check_update_task(void *pvParameter) 
{
	while(1) 
	{
		ESP_LOGI(TAG, "Looking for a new firmware...");
		OTA_status = 1;
		vTaskDelay(2000/portTICK_PERIOD_MS);
	
		// configure the esp_http_client
		esp_http_client_config_t config = {
        .url = UPDATE_JSON_URL,
        .event_handler = _http_event_handler,
		// .cert_pem = server_cert_pem_start,
		};
		esp_http_client_handle_t client = esp_http_client_init(&config);
	
		// downloading the json file
		esp_err_t err = esp_http_client_perform(client);
		if(err == ESP_OK) {
			
			// parse the json file	
			cJSON *json = cJSON_Parse(rcv_buffer);
			if(json == NULL) ESP_LOGI(TAG, "downloaded file is not a valid json, aborting...");
			else {	
				cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
				cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");
				
				// check the version
				if(!cJSON_IsNumber(version)) ESP_LOGI(TAG, "unable to read new version, aborting...");
				else {
					
					double new_version = version->valuedouble;
					if(new_version > FIRMWARE_VERSION) {
						
						ESP_LOGI(TAG, "New firmware available");
						OTA_status = 2;
						vTaskDelay(2000/portTICK_PERIOD_MS);


						if(cJSON_IsString(file) && (file->valuestring != NULL)) {

							ESP_LOGI(TAG, "Downloading and installing new firmware (%s)...\n", file->valuestring);
							OTA_status = 3;

							esp_http_client_config_t ota_client_config = {
								.url = file->valuestring,
								.cert_pem = server_cert_pem_start,
							};
							esp_err_t ret = esp_https_ota(&ota_client_config);
							
							if (ret == ESP_OK) {
								ESP_LOGI(TAG, "OTA OK, restarting...");
								OTA_status = 5;
								vTaskDelay(5000/portTICK_PERIOD_MS);
								esp_restart();
							} else {
								OTA_status = 6;
								vTaskDelay(5000/portTICK_PERIOD_MS);
								ESP_LOGI(TAG, "OTA failed...");
							}
						}
						else 
						{
							OTA_status = 6;
							vTaskDelay(5000/portTICK_PERIOD_MS);
							ESP_LOGI(TAG, "Unable to read the new file name, aborting...");
						}
					}
					else 
					{
						OTA_status = 4;
						vTaskDelay(5000/portTICK_PERIOD_MS);
						ESP_LOGI(TAG, "Current firmware version is lower or equal than the available one, nothing to do...");
					}
				}
			}
		}
		else 
		{
			OTA_status = 6;
			vTaskDelay(5000/portTICK_PERIOD_MS);
			ESP_LOGI(TAG, "Unable to download the json file, aborting...");
		}
		
		// cleanup and restart MQTT
		esp_http_client_cleanup(client);	
		mqtt_app_start();	

        vTaskDelete(NULL);
    }
}

TaskHandle_t xHandle = NULL;

void OTA_start(void)
{
	xTaskCreate(Check_update_task, "check_update_task", 8192, NULL, 0, &xHandle);
}
void OTA_stop(void)
{
	vTaskDelete(xHandle);
}
