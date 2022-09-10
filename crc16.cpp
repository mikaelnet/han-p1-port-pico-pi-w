#include "crc16.hpp"

#define CRC_POLYNOMIAL  0x8005
#define CRC_WIDTH (8 * sizeof(crc16_t))
#define CRC_TOPBIT (1 << (CRC_WIDTH - 1))

crc16_t crcTable[256];
void crc16Init()
{
    crc16_t remainder;

    for (int dividend = 0; dividend < 256; ++ dividend) {
        remainder = dividend << (CRC_WIDTH - 1);

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

crc16_t crcFast(const uint8_t *message, int nBytes)
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

void crcStream(uint8_t byte, crc16_t& remainder)
{
    uint8_t data = byte ^ (remainder >> (CRC_WIDTH - 8));
    remainder = crcTable[data] ^ (remainder << 8);
}
