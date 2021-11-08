#include "http_server_app.h"

/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>

#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "string.h"

#include <esp_http_server.h>



static const char *TAG = "HTTPSERVER";
static httpd_handle_t server = NULL;

static httpd_req_t *REG;


// extern const uint8_t index_html_start[] asm("_binary_anh_jpg_start");
// extern const uint8_t index_html_end[] asm("_binary_anh_jpg_end");

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

static http_post_callback_t    http_post_switch_callback = NULL;
static http_get_callback_t     http_get_dht11_callback = NULL;
static http_post_callback_t    http_post_slider_callback = NULL;
static http_post_callback_t    http_post_wifi_infor_callback = NULL;


/* An HTTP GET html handler */
static esp_err_t dht11_get_handler(httpd_req_t *req)
{

    // const char* resp_str = (const char*) "Hello!";
    httpd_resp_set_type(req, "text/html");
    // httpd_resp_set_type(req, "image/jpg");
    httpd_resp_send(req, (const char*) index_html_start, index_html_end - index_html_start);
    // httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;

}

static const httpd_uri_t get_dht11 = {
    .uri       = "/dht11",
    .method    = HTTP_GET,
    .handler   = dht11_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};

void dht11_response(char* data, int len)
{
    httpd_resp_send(REG, data, len);
}


/* An HTTP GET dht11 handler */
static esp_err_t dht11_data_get_handler(httpd_req_t *req)
{
    // static int temp=20, hum=70;
    // Data myData = {7, 3, 5, temp++, hum++};
   
    // httpd_resp_send(req, GenData(myData), strlen(GenData(myData)));
    REG = req;
    http_get_dht11_callback();

    return ESP_OK;

}

static const httpd_uri_t get_data_dht11 = {
    .uri       = "/get_data_dht11",
    .method    = HTTP_GET,
    .handler   = dht11_data_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};




/* An HTTP POST switch handler */
static esp_err_t sw1_post_handler(httpd_req_t *req)
{
    char buf[100];

    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    // printf("Data nhan duoc: %.*s\n", sizeof(buf), buf);
    http_post_switch_callback(buf, req ->content_len);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t sw1_post_data = {
    .uri       = "/switch1",
    .method    = HTTP_POST,
    .handler   = sw1_post_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};


/* An HTTP POST slider handler */
static esp_err_t slider_post_handler(httpd_req_t *req)
{
    char buf[100];

    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    // printf("Data nhan duoc: %.*s\n", sizeof(buf), buf);
    http_post_slider_callback(buf, req ->content_len);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t slider_post_data = {
    .uri       = "/slider",
    .method    = HTTP_POST,
    .handler   = slider_post_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};



/* An HTTP POST wifi_infor handler */
static esp_err_t wifi_infor_post_handler(httpd_req_t *req)
{
    char buf[100];
    /* Read the data for the request */
    httpd_req_recv(req, buf, req->content_len);
    // printf("Data nhan duoc: %.*s\n", sizeof(buf), buf);
    http_post_wifi_infor_callback(buf, req ->content_len);
    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t wifi_infor_post_data = {
    .uri       = "/wifi_infor",
    .method    = HTTP_POST,
    .handler   = wifi_infor_post_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

/* An HTTP 404 handler */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/dht11", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}


void start_webserver(void)
{
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) 
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");

        httpd_register_uri_handler(server, &get_dht11);
        httpd_register_uri_handler(server, &get_data_dht11);

        httpd_register_uri_handler(server, &sw1_post_data);
        httpd_register_uri_handler(server, &slider_post_data);
        httpd_register_uri_handler(server, &wifi_infor_post_data);
        
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    else
    {
        ESP_LOGI(TAG, "Error starting server!");
    }

}


void stop_webserver(void)
{
    // Stop the httpd server
    httpd_stop(server);
}

void http_set_callback_switch(void *cb)
{
    http_post_switch_callback = cb;
}

void http_set_callback_dht11(void *cb)
{
    http_get_dht11_callback = cb;
}

void http_set_callback_slider(void *cb)
{
    http_post_slider_callback = cb;
}
void http_set_callback_wifi_infor(void *cb)
{
    http_post_wifi_infor_callback = cb;
}


