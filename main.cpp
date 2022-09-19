#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

#include "crc16.hpp"
#include "uart.hpp"
#include "ntp.hpp"
#include "tcp_client.hpp"

#define UART_RX_PIN     5
#define DREQ_PIN        2

/*static void dns_lookup_done(const char *hostname, const ip_addr_t *ip, void *arg) {

}*/

typedef struct {
    struct tcp_pcb *tcp_pcb;
    ip_addr_t ip;
    uint16_t port;
} tcp_client_t;

err_t tcpClose(void *arg) {
    tcp_client_t *state = (tcp_client_t *)arg;
    err_t err = ERR_OK;

    puts("Closing connection");
    if (state->tcp_pcb) {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        tcp_close(state->tcp_pcb);
        
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            tcp_abort(state->tcp_pcb);
            err = ERR_ABRT;
        }
        state->tcp_pcb = NULL;
    }

    return err;
}

uint32_t tcp_send_packet(void *arg)
{
    tcp_client_t *state = (tcp_client_t *)arg;

    const char *string = "GET /?data1=12&data2=5 HTTP/1.0\r\nHost: docker.mikael.com\n\r\n";
    uint32_t len = strlen(string);

    u16_t bufSize = tcp_sndbuf(state->tcp_pcb);
    printf("%d / %d bytes available in send buffer\n", 
        bufSize, tcp_sndqueuelen(state->tcp_pcb));

    printf("Sending %d bytes of data\n", len);
    /* push to buffer */
    err_t error = tcp_write(state->tcp_pcb, string, strlen(string), TCP_WRITE_FLAG_COPY);
    if (error) {
        printf("ERROR: Code: %d (tcp_send_packet :: tcp_write)\n", error);
        return 1;
    }

    puts("Flushing output buffer");
    /* now send */
    error = tcp_output(state->tcp_pcb);
    if (error) {
        printf("ERROR: Code: %d (tcp_send_packet :: tcp_output)\n", error);
        return 1;
    }
    return 0;
}

err_t tcpSendCallback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    printf("Server ACK'ed %d bytes of data\n", len);
    return ERR_OK;
}

err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    tcp_client_t *state = (tcp_client_t *)arg;

    puts("Data recieved.");
    if (!p) {
        puts("The remote host closed the connection.");
        puts("Now I'm closing the connection.");
        tcpClose(arg);
        return ERR_ABRT;
    } 

    printf("Number of pbufs %d\n", pbuf_clen(p));
    printf("Contents of pbuf %s\n", (char *)p->payload);

    return 0;
}

/* connection established callback, err is unused and only return 0 */
err_t connectCallback(void *arg, struct tcp_pcb *tcp_pcb, err_t err) 
{
    puts("Connection established");
    tcp_send_packet(arg);
    return 0;
}

void tcpErrorHandler(void *args, err_t err) {
    
    if (err != ERR_ABRT) {
        printf("tcpErrorHandler %d\n", err);
        tcpClose(args);
    }
    else {
        puts("Regular abort");
    }
}

int main()
{
    tcp_client_t myState;
    tcp_client_t *state = &myState;
    IP4_ADDR(&(state->ip), 192, 168, 1, 49);
    state->port = 80;

    stdio_init_all();
    puts("Testing connectivity");
    puts("Enabling WiFi");
    int error;
    while ((error = cyw43_arch_init_with_country(CYW43_COUNTRY_SWEDEN)) != PICO_OK) {
        printf("WiFi init failed: %d\n", error);
        sleep_ms(1000);
    }
    
    cyw43_arch_enable_sta_mode();

    do {
        puts("Connecting to WiFi...");
        error = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
        if (error != PICO_OK) {
            printf("failed to connect: %d\n", error);
        }
    } while (error != PICO_OK);
    printf("Connected. IP %s\n", ipaddr_ntoa(&cyw43_state.netif[0].ip_addr));

    state->tcp_pcb = tcp_new();
    if (!state->tcp_pcb) {
        puts("Failed to create tcp_pcb object");
        return -1;
    }
    tcp_arg(state->tcp_pcb, state);

    tcp_err(state->tcp_pcb, tcpErrorHandler);
    tcp_recv(state->tcp_pcb, tcpRecvCallback);
    tcp_sent(state->tcp_pcb, tcpSendCallback);

    printf("Connecting to %s:%d\n", ipaddr_ntoa(&state->ip), state->port);
    cyw43_arch_lwip_begin();
    tcp_connect(state->tcp_pcb, &state->ip, state->port, connectCallback);
    cyw43_arch_lwip_end();

    while (true) {
        cyw43_arch_poll();
        sleep_ms(1);
    }
}

#if 0
int main()
{
    stdio_init_all();
    gpio_init(DREQ_PIN);
    gpio_set_dir(DREQ_PIN, GPIO_OUT);

    printf("Hello world!\n");

    han_uart_init();

    TCP_CLIENT_T *state = wifi_init();
    if (!state) {
        printf("No state returned from wifi_init()");
        return -1;
    }
    
    cyw43_arch_poll();
    //ip_addr_t targetIp;
    //error = dns_gethostbyname("docker.mikael.com", &targetIp, )

    while (true) {
        gpio_put(DREQ_PIN, 1);
        
        absolute_time_t timeout = make_timeout_time_ms(15*1000);
        while (absolute_time_diff_us(get_absolute_time(), timeout) > 0) {
            if (han_data_available())
                break;
            cyw43_arch_poll();
            sleep_ms(1);
        }

        if (!han_data_available()) {
            printf("15 second timeout. No data recieved.\n");
        }
        else {
            char msg[2048];
            han_copy_buffer((uint8_t *)msg, sizeof(msg));
            cyw43_arch_poll();
            printf ("Length: %d\n", strlen(msg));
            printf (msg);
            cyw43_arch_poll();
            tcp_send_data(state, msg);
        }

        gpio_put(DREQ_PIN, 0);

        for (int i = 0 ; i < 20000 ; i ++) {
            sleep_ms(1);
            cyw43_arch_poll();
        }
    }


    cyw43_arch_deinit();
    return 0;
}
#endif
#if 0
int main() 
{
    
    crc16_t checksum;
    
    stdio_init_all();
    crc16_init();
    int index = 0;
    while (true) {
        crc16_t checksum = crc16_fast((const uint8_t *)example_data, strlen(example_data));
        printf("%d CRC of %s (%d bytes): 0x%04X\n", index ++, 
            example_data, strlen(example_data), checksum);
        sleep_ms(5000);
    }
    return 0;

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_SWEDEN)) {
        printf("WiFi init failed");
        return -1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    }
    //run_tcp_client_test();
    run_ntp_test();
    cyw43_arch_deinit();

#if 0
    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(250);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(250);
    }
#endif 
#if 0
    while (true) {
        printf(messageTest);
        //crc16_print();
        printf("\r\n");

        int count = 0;
        checksum = 0;
        for (uint8_t *ptr = (uint8_t *)messageTest; *ptr ; ptr ++, count ++) {
            checksum = crc16_stream(checksum, *ptr);
            if (count % 8 == 0)
                printf ("\r\n");
            printf("%04X", checksum);
            if (checksum == 0x7945)
                printf("!!! ");
            else
                printf("    ");
        }
        printf ("\r\n");
        //checksum = crc16_fast((const uint8_t *)messageTest, strlen(messageTest));
        //printf("Checksum fast: %04X\r\n", checksum); // 7945
        //checksum = crc16_slow((const uint8_t *)messageTest, strlen(messageTest));
        //printf("Checksum slow: %04X\r\n", checksum); // 7945
        sleep_ms(5000);
    }
#endif
    return 0;
}
#endif