set(app_src sd_card.c)
set(pri_reg sdmmc fatfs)

idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_reg})
