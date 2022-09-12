#include "crc16.hpp"
#include <stdio.h>
#include <stdlib.h>

#define CRC_POLYNOMIAL  0x8005
#define CRC_WIDTH (8 * sizeof(crc16_t))
#define CRC_TOPBIT (1 << (CRC_WIDTH - 1))

#define CRC_INITIAL_VALUE 0x0000
#define CRC_FINAL_XOR_VALUE 0x0000

#define REFLECT_DATA(X)       ((uint8_t) reflect((X), 8))
#define REFLECT_REMAINDER(X)  ((crc16_t) reflect((X), CRC_WIDTH))

static bool crcTableInitialized = false;
static crc16_t crcTable[256];
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

    crcTableInitialized = true;
}

#if 0
void crc16_print_lookup_table()
{
    for (int i=0 ; i < 256 ; i ++) {
        if (i % 16 == 0)
            printf("\n");
        printf("0x%04X ", crcTable[i]);
    }
    printf("\n");
}
#endif

uint32_t reflect(uint32_t data, uint8_t nBits)
{
    uint32_t  reflection = 0;

    for (uint8_t bit = 0; bit < nBits; ++bit)
    {
        if (data & 0x01)
        {
            reflection |= (1 << ((nBits - 1) - bit));
        }

        data = (data >> 1);
    }

    return (reflection);
}

crc16_t crc16_fast(const uint8_t *data, size_t length)
{
    if (!crcTableInitialized)
        crc16_init();

    crc16_t crc = CRC_INITIAL_VALUE;
    const uint8_t *ptr = data;
    if (ptr != NULL) {
        for (int i=0 ; i < length ; i++) {
            uint8_t d = REFLECT_DATA(*ptr ++) ^ (crc >> (CRC_WIDTH - 8));
            crc = crcTable[d] ^ (crc << 8);
        }
    }

    return (REFLECT_REMAINDER(crc) ^ CRC_FINAL_XOR_VALUE);
}
