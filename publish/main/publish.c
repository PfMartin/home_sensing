#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_sleep.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"
#include "DHT.h"

#define SSID                "FRITZ!Box 7582 PJ"
#define PASSPHRASE          "95605533072376088713"
#define WIFI_MAX_RETRY_NUM  10

#define MQTT_BROKER_HOST    "ubuntu"
#define MQTT_BROKER_PORT    1884
#define MQTT_TAG            "MQTT_TCP"
#define TOPIC_TEMP          "worms/temperature"
#define TOPIC_HUM           "worms/humidity"

#define DHT22_GPIO_NUM      4
#define DHT22_TAG           "DHT22"

#define BUTTON_GPIO_NUM     23

#define SLEEP_WAKEUP_TIME 60

static RTC_DATA_ATTR struct timeval sleep_enter_time;


static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

esp_err_t init_wifi()
{
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASSPHRASE}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_err_t start_err = esp_wifi_start();

    return start_err;
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    char connect_msg[31] = "ESP32 connected to MQTT Broker";
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_publish(client, TOPIC_HUM, connect_msg, 0, 1, 0);
        esp_mqtt_client_publish(client, TOPIC_TEMP, connect_msg, 0, 1, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
        printf("\nTOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(MQTT_TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(MQTT_TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static esp_mqtt_client_handle_t mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .host = MQTT_BROKER_HOST,
        .port = MQTT_BROKER_PORT,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    return client;
}

void dht22_init(void) {
  setDHTgpio(DHT22_GPIO_NUM);
  ESP_LOGI(DHT22_TAG, "Starting DHT22 measurements");
}

void publish_dht22_measurements(esp_mqtt_client_handle_t client) {
  ESP_LOGI(DHT22_TAG, "Reading DHT22");
  int ret = readDHT();
  errorHandler(ret);

  vTaskDelay(100 / portTICK_PERIOD_MS);

  float humidity = getHumidity();
  float temperature = getTemperature();

  ESP_LOGI(DHT22_TAG, "Hum: %.1f Tmp: %.1f\n", humidity, temperature);

  char humidity_string[5];
  char temperature_string[5];

  sprintf(humidity_string, "%.1f", humidity);
  sprintf(temperature_string, "%.1f", temperature);

  esp_mqtt_client_publish(client, TOPIC_HUM, humidity_string, 0, 1, 1);
  esp_mqtt_client_publish(client, TOPIC_TEMP, temperature_string, 0, 1, 1);
}

void handle_deep_sleep_wakeup() {
  struct timeval now;
  gettimeofday(&now, NULL);
  int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

  switch(esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_TIMER: {
      printf("Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);
      break;
    }
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
      printf("Not a deep sleep reset\n");
  }
}

void set_wakeup_timer(int wakeup_time_sec) {
  printf("Enabling time wakeup, %ds\n", wakeup_time_sec);
  esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
}

void app_main(void)
{

    handle_deep_sleep_wakeup();
    set_wakeup_timer(SLEEP_WAKEUP_TIME);

    nvs_flash_init();

    esp_err_t init_err;
    do {
      printf("Initializing WIFI\n");
      init_err = init_wifi();
      vTaskDelay(3000 / portTICK_PERIOD_MS);
    } while (init_err != ESP_OK);

    esp_err_t connect_err;
    do {
      printf("Trying to connect to WIFI\n");
      connect_err = esp_wifi_connect();
      vTaskDelay(3000 / portTICK_PERIOD_MS);
    }  while (connect_err != ESP_OK);

    printf("Connecting to MQTT Broker \n");
    esp_mqtt_client_handle_t client = mqtt_app_start();

    dht22_init();
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    for (int i = 0; i < 10; i++) {
      publish_dht22_measurements(client);
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_mqtt_client_stop(client);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_mqtt_client_disconnect(client);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_wifi_stop();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("Entering deep sleep\n");
    esp_deep_sleep_start();
}
