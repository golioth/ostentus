#include <stdint.h>

#define I2C_BUFFER_SIZE 64

void fifo_init(void);
int fifo_pop(uint8_t * r_buf);
bool fifo_has_data(void);
