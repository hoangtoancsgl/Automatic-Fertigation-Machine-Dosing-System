set(app_src ds3231.c i2cdev.c )
set(pri_reg)

idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_reg})
