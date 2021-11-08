
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "led_strip.h"
#include "ws2812b.h"

static const char *TAG = "RMT";

#define RMT_TX_CHANNEL RMT_CHANNEL_0

led_strip_t *strip;

void ws2812b_init(int tx_pin, int num_led)
{

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(tx_pin, RMT_TX_CHANNEL);

    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(num_led, (led_strip_dev_t)config.channel);
    strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 100));

    
}
void ws2812b_set_color(int index, int r, int g, int b)
{
    ESP_ERROR_CHECK(strip->set_pixel(strip, index, r, g, b));
}

void ws2812b_update_color()
{
    ESP_ERROR_CHECK(strip->refresh(strip, 100));
}



