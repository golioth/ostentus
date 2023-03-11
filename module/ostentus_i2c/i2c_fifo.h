#ifndef I2C_FIFO_H
#define I2C_FIFO_H

#define OSTENTUS_VERSION "0.2.0"

void fifo_init(void);
int fifo_pop(uint8_t * r_buf);
bool fifo_has_data(void);

#endif
