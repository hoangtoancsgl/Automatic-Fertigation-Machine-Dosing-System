set(app_src adc.c)
set(pri_reg esp_adc_cal)

idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_reg})
