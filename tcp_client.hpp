#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H

#define RECV_BUFFER_SIZE 2048
#define SEND_BUFFER_SIZE 2048
#define POLL_TIME_S 5

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TCP_CLIENT_T_ {
    struct tcp_pcb *tcp_pcb;
    ip_addr_t tcp_server_address;
    bool tcp_server_address_resolved;
    int tcp_server_port;
    absolute_time_t tcp_sent_time;
    uint8_t send_buffer[SEND_BUFFER_SIZE];
    uint8_t recv_buffer[RECV_BUFFER_SIZE];
    int send_buffer_len;
    int recv_buffer_len;
    
    bool connected;
    bool complete;
} TCP_CLIENT_T;

extern TCP_CLIENT_T *wifi_init();
extern err_t tcp_send_data(TCP_CLIENT_T *state, const char *message);

#ifdef __cplusplus
}
#endif

#endif
