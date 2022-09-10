#ifndef CRC16_HPP
#define CRC16_HPP

#include "pico/stdlib.h"

typedef uint16_t crc16_t;

extern void crc16_init();
//extern void crc16_print();
extern uint16_t crc16_fast(const uint8_t *message, int nBytes);
//extern crc16_t crc16_slow(const uint8_t *message, int nBytes);
extern uint16_t crc16_stream(uint16_t seed, uint8_t byte);


#endif