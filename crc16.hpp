#ifndef CRC16_HPP
#define CRC16_HPP

#include "pico/stdlib.h"

typedef uint16_t crc16_t;

extern void crc16Init();
extern crc16_t crcFast(const uint8_t *message, int nBytes);
extern void crcStream(uint8_t byte, crc16_t& remainder);

#endif