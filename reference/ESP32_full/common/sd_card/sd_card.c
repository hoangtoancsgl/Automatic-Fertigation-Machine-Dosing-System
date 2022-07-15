
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"

#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "sd_card.h"


static const char *TAG = "SD_CARD";

#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   13

extern char filename_arr[20];
extern bool sd_card_status;

esp_err_t init_sd_card(void)
{
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t* card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return 1;
    }
    
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) 
    {
        if (ret == ESP_FAIL) 
        {
            ESP_LOGE(TAG, "Failed to mount filesystem");        
        } 
        else 
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return 1;
    }
    return ESP_OK;
}

esp_err_t sd_card_write_file(char *file_name, bool write_type, char *data)
{
    FILE* f = fopen(file_name, write_type ? "w" : "a");

    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        sd_card_status = 0;
        return 1;
    }
    fprintf(f, data);
    fprintf(f, "\n");
    fclose(f);
    ESP_LOGI(TAG, "%s", write_type ? "Overwritten successfully" : "Write successfully");
    sd_card_status = 1;
    return ESP_OK;
}

char* creat_file_name(int day) 
{
    for(int i=0;i<20;i++) filename_arr[i] = NULL;
    strcat(filename_arr, "/sdcard/log");
    char day_arr[10];
    
    sprintf(day_arr, "%d.txt",day);
    strcat(filename_arr, day_arr);
    return filename_arr;
}  


