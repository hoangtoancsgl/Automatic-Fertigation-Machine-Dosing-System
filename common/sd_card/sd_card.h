#ifndef SD_CARD_H
#define SD_CARD_H


//SD card data struct
typedef struct Data_SD
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    
    float PH;
    int TDS;
    float temperature;

    int nutri1;
    int nutri2;
    int acid_li;
    int base_li;

}Data_SD;

esp_err_t init_sd_card(void);
esp_err_t sd_card_write_file(char *file_name, bool write_type, char *data);
char* creat_file_name(int day);


#endif