#ifndef __UART_HPP
#define __UART_HPP

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Initialise the uart1 for reading the HAN port on 115200 bps
 */
extern void han_uart_init();

// TODO: Add some kind of enable/disable functionality here
// so that the buffer can't be overwritten.

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

/*! \brief Gets a pointer to the raw recv buffer
 * 
 * This is a mutating buffer, so han_data_available()
 * must return true first for valid data to be there
 * Also, the buffer will be overwritten unless the data
 * is taken care of before a new buffer is recieved.
 * Therefore it would make sense to not request (DREQ)
 * any new data until the data has been sent.
 * 
 * @return Pointer to the internal HAN recv buffer
 */ 
extern const uint8_t *han_raw_buffer();

/*! \brief Mark the HAN recv buffer as read
 *
 * This marks the buffer as read, so that han_data_available
 * will return false. 
 */
extern void han_mark_buffer_read();

#ifdef __cplusplus
}
#endif

#endif

