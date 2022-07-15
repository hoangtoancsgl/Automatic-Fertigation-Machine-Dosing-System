#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "esp_sntp.h"
#include "ds3231.h"

#define CHECK_ARG(ARG) do { if (!ARG) return ESP_ERR_INVALID_ARG; } while (0)

static const char *TAG = "DS3231";

uint8_t bcd2dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0f);
}

uint8_t dec2bcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}

esp_err_t ds3231_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    CHECK_ARG(dev);

    dev->port = port;
    dev->addr = DS3231_ADDR;
    dev->sda_io_num = sda_gpio;
    dev->scl_io_num = scl_gpio;
    dev->clk_speed = I2C_FREQ_HZ;
    // return i2c_master_init(port, sda_gpio, scl_gpio);
    return ESP_OK;
}

esp_err_t ds3231_set_time(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t data[7];

    /* time/date data */
    data[0] = dec2bcd(time->tm_sec);
    data[1] = dec2bcd(time->tm_min);
    data[2] = dec2bcd(time->tm_hour);
    /* The week data must be in the range 1 to 7, and to keep the start on the
     * same day as for tm_wday have it start at 1 on Sunday. */
    data[3] = dec2bcd(time->tm_wday + 1);
    data[4] = dec2bcd(time->tm_mday);
    data[5] = dec2bcd(time->tm_mon + 1);
    data[6] = dec2bcd(time->tm_year - 2000);

    return i2c_dev_write_reg(dev, DS3231_ADDR_TIME, data, 7);
}

esp_err_t ds3231_get_raw_temp(i2c_dev_t *dev, int16_t *temp)
{
    CHECK_ARG(dev);
    CHECK_ARG(temp);

    uint8_t data[2];

    esp_err_t res = i2c_dev_read_reg(dev, DS3231_ADDR_TEMP, data, sizeof(data));
    if (res == ESP_OK)
        *temp = (int16_t)(int8_t)data[0] << 2 | data[1] >> 6;

    return res;
}

esp_err_t ds3231_get_temp_integer(i2c_dev_t *dev, int8_t *temp)
{
    CHECK_ARG(temp);

    int16_t t_int;

    esp_err_t res = ds3231_get_raw_temp(dev, &t_int);
    if (res == ESP_OK)
        *temp = t_int >> 2;

    return res;
}

esp_err_t ds3231_get_temp_float(i2c_dev_t *dev, float *temp)
{
    CHECK_ARG(temp);

    int16_t t_int;

    esp_err_t res = ds3231_get_raw_temp(dev, &t_int);
    if (res == ESP_OK)
        *temp = t_int * 0.25;

    return res;
}

esp_err_t ds3231_get_time(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t data[7];

    /* read time */
    esp_err_t res = i2c_dev_read_reg(dev, DS3231_ADDR_TIME, data, 7);
        if (res != ESP_OK) return res;

    /* convert to unix time structure */
    time->tm_sec = bcd2dec(data[0]);
    time->tm_min = bcd2dec(data[1]);
    if (data[2] & DS3231_12HOUR_FLAG)
    {
        /* 12H */
        time->tm_hour = bcd2dec(data[2] & DS3231_12HOUR_MASK) - 1;
        /* AM/PM? */
        if (data[2] & DS3231_PM_FLAG) time->tm_hour += 12;
    }
    else time->tm_hour = bcd2dec(data[2]); /* 24H */
    time->tm_wday = bcd2dec(data[3]) - 1;
    time->tm_mday = bcd2dec(data[4]);
    time->tm_mon  = bcd2dec(data[5] & DS3231_MONTH_MASK) - 1;
    time->tm_year = bcd2dec(data[6]) + 2000;
    time->tm_isdst = 0;

    // apply a time zone (if you are not using localtime on the rtc or you want to check/apply DST)
    //applyTZ(time);

    return ESP_OK;
}


void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    ESP_LOGI(TAG, "Your NTP Server is %s", NTP_SERVER);
    sntp_setservername(0, NTP_SERVER);
    sntp_init();
}

bool obtain_time(void)
{
    // initialize_sntp();

    // wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(8000/portTICK_PERIOD_MS);
    }

    if (retry == retry_count) return false;
    return true;
}

esp_err_t setClock(void)
{
    // obtain time over NTP
    ESP_LOGI(TAG, "Connecting to WiFi and getting time over NTP.");
    if(!obtain_time()) 
    {
        ESP_LOGE(TAG, "Fail to getting time over NTP.");
        return 1;
    }

    // update 'now' variable with current time
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    time(&now);
    now = now + (CONFIG_TIMEZONE*60*60);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);

    // Initialize RTC
    i2c_dev_t dev;
    if (ds3231_init_desc(&dev, I2C_NUM_0, SDA_PIN, SCL_PIN) != ESP_OK) {
        ESP_LOGE(TAG, "Could not init device descriptor.");
        return 1;
    }

    ESP_LOGD(TAG, "timeinfo.tm_sec=%d",timeinfo.tm_sec);
    ESP_LOGD(TAG, "timeinfo.tm_min=%d",timeinfo.tm_min);
    ESP_LOGD(TAG, "timeinfo.tm_hour=%d",timeinfo.tm_hour);
    ESP_LOGD(TAG, "timeinfo.tm_wday=%d",timeinfo.tm_wday);
    ESP_LOGD(TAG, "timeinfo.tm_mday=%d",timeinfo.tm_mday);
    ESP_LOGD(TAG, "timeinfo.tm_mon=%d",timeinfo.tm_mon);
    ESP_LOGD(TAG, "timeinfo.tm_year=%d",timeinfo.tm_year);

    struct tm time = {
        .tm_year = timeinfo.tm_year + 1900,
        .tm_mon  = timeinfo.tm_mon,  // 0-based
        .tm_mday = timeinfo.tm_mday,
        .tm_hour = timeinfo.tm_hour,
        .tm_min  = timeinfo.tm_min,
        .tm_sec  = timeinfo.tm_sec
    };

    if (ds3231_set_time(&dev, &time) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Could not set time.");
        return 1;
    }
    ESP_LOGI(TAG, "Set initial date time done");
    return ESP_OK;
}

esp_err_t getClock(int *year, int *month, int *day, int *hour, int *minute, int *second, float *tempt)
{
    // Initialize RTC
    i2c_dev_t dev;
    if (ds3231_init_desc(&dev, I2C_NUM_0, SDA_PIN, SCL_PIN) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Could not init device descriptor.");
        return 1;
    }

    // Get RTC date and time
    float temp;
    struct tm rtcinfo;

    if (ds3231_get_temp_float(&dev, &temp) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Could not get temperature.");
        return 1;
    }

    if (ds3231_get_time(&dev, &rtcinfo) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Could not get time.");
        return 1;
    }

    *year = rtcinfo.tm_year;
    *month = rtcinfo.tm_mon + 1;
    *day = rtcinfo.tm_mday;

    *hour = rtcinfo.tm_hour;
    *minute = rtcinfo.tm_min;
    *second = rtcinfo.tm_sec;

    *tempt = temp;

    return ESP_OK;
    
}

