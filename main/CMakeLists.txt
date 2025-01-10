idf_component_register(SRCS "main.c"
                   
                    "rotary_encoder_module.c"
                    PRIV_REQUIRES spi_flash driver
                    INCLUDE_DIRS "")
