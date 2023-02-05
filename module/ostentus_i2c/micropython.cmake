# Create an INTERFACE library for our C module.
add_library(usermod_ostentus_i2c INTERFACE)

# Add our source files to the lib
target_sources(usermod_ostentus_i2c INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ostentus_i2c.c
    ${CMAKE_CURRENT_LIST_DIR}/i2c_fifo.c
    ${CMAKE_CURRENT_LIST_DIR}/i2c_multi.c
)

pico_generate_pio_header(usermod_ostentus_i2c ${CMAKE_CURRENT_LIST_DIR}/i2c_multi.pio)
# add_dependencies(usermod_ostentus_i2c INTERFACE firmware)

# Add the current directory as an include directory.
target_include_directories(usermod_ostentus_i2c INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_ostentus_i2c)
