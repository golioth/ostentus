// Include MicroPython API.
#include "py/runtime.h"
#include "i2c_fifo.h"

STATIC mp_obj_t init(void) {
    fifo_init();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(init_obj, init);

STATIC mp_obj_t has_data(void) {
    return mp_obj_new_bool(fifo_has_data());
}
MP_DEFINE_CONST_FUN_OBJ_0(has_data_obj, has_data);

STATIC mp_obj_t pop(void) {
    // Your code here!
    uint8_t f_buf[64];
    fifo_pop(f_buf);

    // signature: mp_obj_t mp_obj_new_bytes(const byte* data, size_t len);
    return mp_obj_new_bytes((const unsigned char *)f_buf, f_buf[1]+2);
}
MP_DEFINE_CONST_FUN_OBJ_0(pop_obj, pop);

// Define all properties of the module.
// Table entries are key/value pairs of the attribute name (a string)
// and the MicroPython object reference.
// All identifiers and strings are written as MP_QSTR_xxx and will be
// optimized to word-sized integers by the build system (interned strings).
STATIC const mp_rom_map_elem_t example_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ostentus_i2c) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&init_obj) },
    { MP_ROM_QSTR(MP_QSTR_pop), MP_ROM_PTR(&pop_obj) },
    { MP_ROM_QSTR(MP_QSTR_has_data), MP_ROM_PTR(&has_data_obj) },
};
STATIC MP_DEFINE_CONST_DICT(example_module_globals, example_module_globals_table);

// Define module object.
const mp_obj_module_t ostentus_i2c_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&example_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_ostentus_i2c, ostentus_i2c_cmodule);
