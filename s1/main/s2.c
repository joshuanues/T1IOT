/* BSD Socket API Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "addr_from_stdin.h"

#include "generator.c"
#include "deep_sleep_clk.c"

#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_IPV6)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
#define HOST_IP_ADDR ""
#endif

#define PORT CONFIG_EXAMPLE_PORT

static const char *TAG = "example";


static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {

#if defined(CONFIG_EXAMPLE_IPV4)
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_IPV6)
        struct sockaddr_in6 dest_addr = { 0 };
        inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
        struct sockaddr_storage dest_addr = { 0 };
        ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
#endif

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

        while (1) {
            char payload[19216];
            char payload_initial[16] = {(char)1,(char)value_generator(),(char)(20042022194300>>24),(char)(20042022194300>>16), (char)(20042022194300>>8), (char)(20042022194300),
                                (char)temp_generator(), 
                                (char)(presion>>24), (char)(presion>>16), (char)(presion>>8), (char)presion,
                                (char)hum_generator(), (char)(co>>24), (char)(co>>16), (char)(co>>8), (char)co};
            
            for(int i=0; i < 16; i++){
                payload[i] = payload_initial[i];
            }
            
            float acc_x[1600];
            for(int n=0; n<1600; n++) {
                float val_x = 2*sin(2*M_PI*0.001*n);
                acc_x[n] = val_x;
            }

            for(int i=0; i<1600; i++){
                payload[(4*i)+16] = (char)(acc_x[i]>>24);
                payload[(4*i)+1+16] = (char)(acc_x[i]>>16);
                payload[(4*i)+2+16] = (char)(acc_x[i]>>8);
                payload[(4*i)+3+16] = (char)(acc_x[i]);
            }

            float acc_y[1600];
            for(int n=0; n<1600; n++) {
                float val_y = 3*cos(2*M_PI*0.001*n);
                acc_y[n] = val_y;
            }

            for(int i=0; i<1600; i++){
                payload[(4*i)+16+6400] = (char)(acc_y[i]>>24);
                payload[(4*i)+1+16+6400] = (char)(acc_y[i]>>16);
                payload[(4*i)+2+16+6400] = (char)(acc_y[i]>>8);
                payload[(4*i)+3+16+6400] = (char)(acc_y[i]);
            }

            float acc_z[1600];
            for(int n=0; n<1600; n++) {
                float val_z = 10*sin(2*M_PI*0.001*n);
                acc_z[n] = val_z;
            }

            for(int i=0; i<1600; i++){
                payload[(4*i)+16+12800] = (char)(acc_z[i]>>24);
                payload[(4*i)+1+16+12800] = (char)(acc_z[i]>>16);
                payload[(4*i)+2+16+12800] = (char)(acc_z[i]>>8);
                payload[(4*i)+3+16+12800] = (char)(acc_z[i]);
            }

            int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");

            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
                if (strncmp(rx_buffer, "OK: ", 4) == 0) {
                    ESP_LOGI(TAG, "Received expected message, reconnecting");
                    break;
                }
            }

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}