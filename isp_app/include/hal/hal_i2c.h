#ifndef __HAL_I2C_H__
#define __HAL_I2C_H__

#include <ebase/types.h>




int i2c_open(int index);
int i2c_close();
int i2c_write16( const uint16_t slave_addr, unsigned int reg_addr, unsigned char reg_val);
int i2c_read16( const uint16_t slave_addr, unsigned int reg_addr, unsigned char *reg_val);
int i2c_write8( const uint16_t slave_addr, unsigned char reg_addr, unsigned char reg_val);
int i2c_read8( const uint16_t slave_addr, unsigned char reg_addr, unsigned char *reg_val);

int32_t i2c_write_common( const uint16_t slave_addr, unsigned int reg_addr, unsigned int addr_size, unsigned int reg_val,  unsigned int val_size);
int32_t i2c_read_common( const uint16_t slave_addr, unsigned int reg_addr, unsigned int addr_size, unsigned int *reg_val, unsigned int val_size );



#endif

