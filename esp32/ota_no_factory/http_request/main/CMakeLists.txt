idf_component_register(SRCS "http_request_example_main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_partition esp_wifi nvs_flash app_update esp_http_client esp_https_ota
                    PRIV_REQUIRES spi_flash)
