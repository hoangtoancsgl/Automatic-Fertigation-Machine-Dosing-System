set(app_src https_ota.c)
set(pri_reg json esp_http_client esp_https_ota esp-tls)

idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_reg}
                    EMBED_TXTFILES ${project_dir}/server_certs/certs.pem)
