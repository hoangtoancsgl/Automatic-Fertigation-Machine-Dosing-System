set(app_src smart_config.c)
set(pri_reg nvs_flash wpa_supplicant)

idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_reg})
