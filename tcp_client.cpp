#include "tcp_client.hpp"
#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

/*
POST /test HTTP/1.0
Host: hostname.com
Content-Type: multipart/form-data;boundary="boundary"

--boundary
Content-Disposition: form-data; name="data"

value1
--boundary

§*/


uint8_t send_buffer[2048];

static TCP_CLIENT_T* tcp_client_init() {
    TCP_CLIENT_T *state = (TCP_CLIENT_T *) calloc(1, sizeof(TCP_CLIENT_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    ip4addr_aton(SERVER_HOST, &state->tcp_server_address);

    return state;
}

static err_t tcp_client_close(void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    err_t err = ERR_OK;
    if (state->tcp_pcb != NULL) {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        err = tcp_close(state->tcp_pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            tcp_abort(state->tcp_pcb);
            err = ERR_ABRT;
        }
        state->tcp_pcb = NULL;
    }
    return err;
}

static err_t tcp_result(void *arg, int status) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    if (status == 0) {
        printf("test success\n");
    } else {
        printf("test failed %d\n", status);
    }
    state->complete = true;
    return tcp_client_close(arg);
}



void tcp_client_send_http_post_request(uint8_t *han_buf)
{
    char *buf = (char *)send_buffer;
    strcpy (buf, "POST /test HTTP/1.0\r\n"
        "Host: hostname.com\r\n"
        "Content-Type: multipart/form-data;boundary=\"boundary\"\r\n"
        "\r\n"
        "--boundary\r\n"
        "Content-Disposition: form-data; name=\"data\"\r\n"
        "\r\n");
    strcat(buf, (char *)han_buf);
    strcat(buf, "\r\n"
        "--boundary\r\n"
        "\r\n");
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tcp_result(arg, err);
    }
    state->connected = true;
    
    err = tcp_write(tpcb, state->send_buffer, state->send_buffer_len, 0 /*TCP_WRITE_FLAG_COPY*/);
    if (err != ERR_OK) {
        printf("Failed to write data %d\n", err);
        return tcp_result(arg, -1);
    }
    err = tcp_output(tpcb);
    if (err != ERR_OK) {
        printf("Failed to transmit data %d\n", err);
        return tcp_result(arg, -1);
    }

    return ERR_OK;
}

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    printf("tcp_client_sent %u\n", len);
    /*state->sent_len += len;

    if (state->sent_len >= BUF_SIZE) {*/

        /*state->run_count++;
        if (state->run_count >= TEST_ITERATIONS) {
            tcp_result(arg, 0);
            return ERR_OK;
        }*/

        // We should receive a new buffer from the server
        /*state->recv_buffer_len = 0;
        state->send_buffer_len = 0;
        printf("Waiting for buffer from server\n");
    }*/

    return ERR_OK;
}

err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    if (!p) {
        return tcp_result(arg, -1);
    }
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        printf("recv %d err %d\n", p->tot_len, err);
        // Receive the buffer
        const uint16_t buffer_left = RECV_BUFFER_SIZE - state->recv_buffer_len;
        state->recv_buffer_len += pbuf_copy_partial(p, state->recv_buffer + state->recv_buffer_len,
                                               p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
        tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    // Parse response here.

    return ERR_OK;
}

static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb) {
    printf("tcp_client_poll\n");
    return tcp_result(arg, -1); // no response is an error?
}

static void tcp_client_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        printf("tcp_client_err %d\n", err);
        tcp_result(arg, err);
    }
}

static bool tcp_client_open(TCP_CLIENT_T *state) {
    printf("Connecting to %s port %u\n", ip4addr_ntoa(&state->tcp_server_address), SERVER_PORT);
    state->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&state->tcp_server_address));
    if (!state->tcp_pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    tcp_arg(state->tcp_pcb, state);
    //tcp_poll(state->tcp_pcb, tcp_client_poll, POLL_TIME_S * 2);
    tcp_sent(state->tcp_pcb, tcp_client_sent);
    tcp_recv(state->tcp_pcb, tcp_client_recv);
    tcp_err(state->tcp_pcb, tcp_client_err);

    state->send_buffer_len = 0;
    state->recv_buffer_len = 0;

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(state->tcp_pcb, 
        &state->tcp_server_address, SERVER_PORT, tcp_client_connected);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}



TCP_CLIENT_T *wifi_init() {
    int error;
    while ((error = cyw43_arch_init_with_country(CYW43_COUNTRY_SWEDEN)) != PICO_OK) {
        printf("WiFi init failed: %d\n", error);
        sleep_ms(1000);
    }
    cyw43_arch_enable_sta_mode();

    do {
        printf("Connecting to WiFi...\n");
        error = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
        if (error != PICO_OK) {
            printf("failed to connect: %d\n", error);
        }
    } while (error != PICO_OK);
    printf("Connected.\n");
    printf ("IP: %s\n", ipaddr_ntoa(&cyw43_state.netif[0].ip_addr));
    printf ("MSK: %s\n", ipaddr_ntoa(&cyw43_state.netif[0].netmask));
    printf ("GW: %s\n", ipaddr_ntoa(&cyw43_state.netif[0].gw));

    return tcp_client_init();
}

err_t tcp_send_data(TCP_CLIENT_T *state, const char *message)
{
    int len = strlen(message);
    memcpy(state->send_buffer, message, len+1);
    state->send_buffer_len = len;
    
    if (!tcp_client_open(state)) {
        tcp_result(state, -1);
        return 1;
    }

    while (!state->complete) {
        cyw43_arch_poll();
        sleep_ms(1);
    }
    return 0;
}

