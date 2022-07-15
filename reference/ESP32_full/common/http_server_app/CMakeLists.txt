set(app_src http_server_app.c)
set(pri_reg esp_http_server)

idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_reg}
                    EMBED_TXTFILES ${project_dir}/html/index.html)
