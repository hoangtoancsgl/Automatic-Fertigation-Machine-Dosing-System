// MQTT (over TCP) Example

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"


#include "esp_log.h"

#include "mqtt_client.h"

#include "mqtt_client_app.h"



static const char *TAG = "MQTT_CLIENT";
static mqtt_data_callback_t    mqtt_data_callback = NULL;
extern uint8_t led_state;
extern char *ID_mqtt_status;

esp_mqtt_client_handle_t client;

esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://ngoinhaiot.com",
        .port  = 1111,
        .username = "hoangtoancsgl",
        .password = "850B3436127D4E73",
        .lwt_topic = "hoangtoancsgl/1fac1308/status",
        .lwt_msg = "offline",
        .lwt_qos = 0,
        .lwt_retain = 0,
        .keepalive = 10

    };

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            led_state = 4;
            esp_mqtt_client_publish(client, "hoangtoancsgl/1fac1308/status", "online", 0, 1, 0);
            msg_id = esp_mqtt_client_subscribe(client, "#", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED, retry...");
            led_state = 4;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("Topic: %.*s\r\n", event->topic_len, event->topic);
            printf("Message: %.*s\r\n", event->data_len, event->data);
            mqtt_data_callback(event->topic, event->data);
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_app_start(void)
{
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

esp_err_t mqtt_app_stop(void)
{
    esp_mqtt_client_disconnect(client);

    return esp_mqtt_client_stop(client);
}

void mqtt_publish_data(char* topic, char* mess)
{
    esp_mqtt_client_publish(client, topic, mess, 0, 1, 0); 
}

void mqtt_set_callback_data_subscribed(void *cb)
{
    mqtt_data_callback = cb;
}
