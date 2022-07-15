set(app_src mqtt_client_app.c)
set(pri_reg mqtt)

idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_reg})
