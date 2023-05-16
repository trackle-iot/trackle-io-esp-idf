idf_component_register(

    # Source files
    SRCS
    "${COMPONENT_DIR}/button_gpio.c"
    "${COMPONENT_DIR}/iot_button.c"
    "${COMPONENT_DIR}/trackle_io_cloud.c"

    # Public interface headers
    INCLUDE_DIRS
    "${COMPONENT_DIR}/include"

    # Components required by units that have to be built inside this component
    REQUIRES
    trackle-library-esp-idf
)
