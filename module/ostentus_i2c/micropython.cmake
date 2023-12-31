include(${CMAKE_CURRENT_LIST_DIR}/../../VERSION)

add_definitions( -DVERSION_MAJOR=${VERSION_MAJOR} )
add_definitions( -DVERSION_MINOR=${VERSION_MINOR} )
add_definitions( -DVERSION_PATCH=${VERSION_PATCH} )

message( "###############################################")
message( "    Building Ostentus version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
message( "###############################################")

# Locate Pico-SDK
include(${CMAKE_CURRENT_LIST_DIR}/../../submodules/micropython/lib/pico-sdk/external/pico_sdk_import.cmake)

# Create an INTERFACE library for our C module.
add_library(usermod_ostentus_i2c INTERFACE)

# Add our source files to the lib
target_sources(usermod_ostentus_i2c INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ostentus_i2c.c
    ${CMAKE_CURRENT_LIST_DIR}/i2c_fifo.c
    ${CMAKE_CURRENT_LIST_DIR}/led_ctrl.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_ostentus_i2c INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../libostentus/include
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_ostentus_i2c pico_i2c_slave)
