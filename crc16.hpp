#ifndef CRC16_HPP
#define CRC16_HPP

#include "pico/stdlib.h"

typedef uint16_t crc16_t;

extern void crc16_init();
extern crc16_t crc16_fast(const uint8_t *data, size_t length);

#endif