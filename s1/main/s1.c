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
#include "addr_from_stdin.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
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

static const char *TAG = "s1";

static void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int *ip_protocol = {0, 1, 2, 3};

    while (1) {
#if defined(CONFIG_EXAMPLE_IPV4)
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(host_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_IPV6)
        struct sockaddr_in6 dest_addr = { 0 };
        inet6_aton(host_ip, &dest_addr.sin6_addr);
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
        struct sockaddr_storage dest_addr = { 0 };
        ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_STREAM, &ip_protocol, &addr_family, &dest_addr));
#endif
        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Successfully connected");

        while (1) {
            int presion = pres_generator();
            int co = co_generator();
            int rms = RMS_generator();
            int ampx = amp_x_generator();
            int ampy = amp_y_generator();
            int ampz = amp_z_generator();
            char payload0[6] = {(char)1,(char)value_generator(),(char)(19042022194300>>24),(char)(19042022194300>>16), (char)(19042022194300>>8), (char)(19042022194300)};
            char payload1[16] = {payload0[0],payload0[1],payload0[2], payload0[3], payload0[4], payload0[5], payload0[6], (char)temp_generator(), 
                                (char)(presion>>24), (char)(presion>>16), (char)(presion>>8), (char)presion,
                                (char)hum_generator(), (char)(co>>24), (char)(co>>16), (char)(co>>8), (char)co};
            char payload2[20] = {payload0[0],payload0[1],payload0[2], payload0[3], payload0[4], payload0[5], payload0[6], (char)temp_generator(), 
                                (char)(presion>>24), (char)(presion>>16), (char)(presion>>8), (char)presion,
                                (char)hum_generator(), (char)(co>>24), (char)(co>>16), (char)(co>>8), (char)co,
                                (char)(rms>>24), (char)(rms>>16), (char)(rms>>8), (char)rms};
            char payload3[44] = {payload0[0],payload0[1],payload0[2], payload0[3], payload0[4], payload0[5], payload0[6], (char)temp_generator(), 
                                (char)(presion>>24), (char)(presion>>16), (char)(presion>>8), (char)presion,
                                (char)hum_generator(), (char)(co>>24), (char)(co>>16), (char)(co>>8), (char)co,
                                (char)(rms>>24), (char)(rms>>16), (char)(rms>>8), (char)rms,
                                (char)(ampx>>24), (char)(ampx>>16), (char)(ampx>>8), (char)(ampx),
                                (char)(ampy>>24), (char)(ampy>>16), (char)(ampy>>8), (char)(ampy),
                                (char)(ampz>>24), (char)(ampz>>16), (char)(ampz>>8), (char)(ampz)};

            int err0 = send(sock, payload0, strlen(payload0), 0);
            deep_sleep_clk(1, 60);
            int err1 = send(sock, payload1, strlen(payload1), 0);
            deep_sleep_clk(1, 60);
            int err2 = send(sock, payload2, strlen(payload2), 0);
            deep_sleep_clk(1, 60);
            int err3 = send(sock, payload3, strlen(payload3), 0);
            deep_sleep_clk(1, 60);
            
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }

            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
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

    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}