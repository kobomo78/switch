idf_component_register(	SRCS "main.cpp"
						SRCS "wifi.c"
						SRCS "blynk_management.c"
						SRCS "ota.cpp"
						SRCS "Server_Exchange.cpp"
						SRCS "ntp.cpp"
                    	INCLUDE_DIRS "."
                    	INCLUDE_DIRS "../include"
                    	REQUIRES driver blynk esp_wifi esp_https_ota nvs_flash app_update esp_partition
                    	EMBED_FILES ${PROJECT_DIR}/cert/ca_cert.pem)

