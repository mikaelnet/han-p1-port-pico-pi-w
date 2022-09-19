#ifndef __UART_HPP
#define __UART_HPP

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Initialise the uart1 for reading the HAN port on 115200 bps
 */
extern void han_uart_init();

/*! \brief Returns true once HAN data is available for reading
 * 
 * Returns true once the ending '!' exclamation mark has been read.
 * Wait another ~0.5ms for the checksum to be recieved.
 * @return true if data is available for reading
 */
extern bool han_data_available();

/*! \brief Copies the HAN data to a target string
 * \param dst The target buffer the HAN packet will be written to
 * \param n   The max length of the target buffer
 * 
 * Copies a NULL terminated string to the target buffer and returns
 * the length of the buffer read. This also marks the internal buffer
 * as read so that new HAN data can be loaded into the internal buffer.
 * 
 * @return The number of bytes transferred to the target buffer
 */
extern int han_copy_buffer(uint8_t *dst, size_t n);

#ifdef __cplusplus
}
#endif

#endif

