#define HOST_IP_ADDR "255.255.255.255"
#define PORT 1900

static const char * TAG = "UDP";
static const char * payload = "Message from ESP32 ";

static void taskUDPClient(void * pvParameters) {
  char rx_buffer[128];
  int addr_family = 0;
  int ip_protocol = 0;
  while (1) {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
      send_uart("UDP", "Unable to create socket", NULL);
      break;
    }
    //send_uart("UDP", "Socket created, sending", NULL);
    ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);
    while (1) {
      int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
      if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);[Codebox=c file=Untitled.c][/Codebox]
        break;
      }
      send_uart("UDP", "Message sent", NULL);

      struct sockaddr_in source_addr;
      socklen_t socklen = sizeof(source_addr);
      int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

      if (len < 0) {
        /* Ошибка при получении */
        ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
        break;
      } else {
        /* Данные получены */
        rx_buffer[len] = '\0';
        send_uart("UDP", "Received Data", rx_buffer);
        if (strncmp(rx_buffer, "OK: ", 4) == 0) {
          send_uart("UDP", "Received expected message, reconnecting", NULL);
          break;
        }
      }
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    if (sock != -1) {
      send_uart("UDP", "Socket Restarting", NULL);
      shutdown(sock, 0);
      close(sock);
    }
  }
  vTaskDelete(NULL);
}

// Инициализация UDP Server
esp_err_t udpClientInit() {
  if (xTaskCreate(taskUDPClient, "taskUDPClient", 4096, NULL, 5, NULL) != pdPASS) {
    return ESP_FAIL;
  }
  return ESP_OK;