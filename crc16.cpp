#include "crc16.hpp"
#include <stdio.h>
#include <stdlib.h>

//#define CRC_POLYNOMIAL  0x8005
#define CRC_POLYNOMIAL  0xA001
#define CRC_WIDTH (8 * sizeof(crc16_t))
#define CRC_TOPBIT (1 << (CRC_WIDTH - 1))

crc16_t crcTable[256];
void crc16_init()
{
    crc16_t remainder;

    for (int dividend = 0; dividend < 256; ++ dividend) {
        remainder = dividend << (CRC_WIDTH - 8);

        for (uint8_t bit = 8; bit > 0; --bit) {
            if (remainder & CRC_TOPBIT) {
                remainder = (remainder << 1) ^ CRC_POLYNOMIAL;
            }
            else {
                remainder = (remainder << 1);
            }
        }

        crcTable[dividend] = remainder;
    }
}

uint16_t crc16_fast(const uint8_t *message, int nBytes)
{
    uint8_t data;
    crc16_t remainder = 0;
    for (int i = 0; i < nBytes ; i++)
    {
        data = message[i] ^ (remainder >> (CRC_WIDTH - 8));
        remainder = crcTable[data] ^ (remainder << 8);
    }

    return remainder;
}

uint16_t crc16_stream(uint16_t seed, uint8_t byte)
{
    uint8_t data = byte ^ (seed >> (CRC_WIDTH - 8));
    return crcTable[data] ^ (seed << 8);
}
