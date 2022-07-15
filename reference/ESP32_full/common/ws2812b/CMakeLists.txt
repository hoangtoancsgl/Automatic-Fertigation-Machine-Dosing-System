set(app_src ws2812b.c)
set(pri_reg led_strip)

idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_reg})
