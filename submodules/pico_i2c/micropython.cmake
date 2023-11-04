# Create an INTERFACE library for our C module.
add_library(usermod_ostentus_i2c INTERFACE)

# Add our source files to the lib
target_sources(usermod_ostentus_i2c INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/i2c_fifo.c
    ${CMAKE_CURRENT_LIST_DIR}/supercon_data_cache.c
    ${CMAKE_CURRENT_LIST_DIR}/supercon_data_transfer.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_ostentus_i2c INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_ostentus_i2c)
