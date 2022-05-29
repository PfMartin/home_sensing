#include <stdio.h>

#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"
#include "DHT.h"
#include "mqtt.h"
#include "wifi.h"
#include "sleep.h"

#define DHT22_GPIO_NUM      4
#define DHT22_TAG           "DHT22"
#define NUM_READINGS        20
#define ENDL_MSG            "255"    // Endler message tells the backend that the data can be processed

#define SLEEP_WAKEUP_TIME   60

/* PROTOTYPES */
void dht22_init(void);

void publish_data(esp_mqtt_client_handle_t client, char data_type[4], char topic[20]);

void cleanup(esp_mqtt_client_handle_t client);

/* MAIN FUNCTION*/
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
      connect_err = connect_wifi();
      vTaskDelay(3000 / portTICK_PERIOD_MS);
    }  while (connect_err != ESP_OK);

    printf("Connecting to MQTT Broker \n");
    esp_mqtt_client_handle_t client = mqtt_app_start();

    dht22_init();
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    char hum_topic[30] = "";
    char temp_topic[30] = "";

    get_topic(hum_topic, "/humidity");
    get_topic(temp_topic, "/temperature");

    for (int i = 0; i < NUM_READINGS; i++) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      publish_data(client, "hum", hum_topic);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      publish_data(client, "temp", temp_topic);
    }

    cleanup(client);
    start_deep_sleep();
}

/* FUNCTION IMPLEMENTATIONS */
void dht22_init(void) {
    setDHTgpio(DHT22_GPIO_NUM);
    ESP_LOGI(DHT22_TAG, "Starting DHT22 measurements");
}

void publish_data(esp_mqtt_client_handle_t client, char data_type[4], char topic[30]) {
    float measurement;
    char measurement_string[5];

    printf("%s\n%s\n", data_type, topic);

    ESP_LOGI(DHT22_TAG, "Reading DHT22");
    int ret = readDHT();
    errorHandler(ret);

    vTaskDelay(100 / portTICK_PERIOD_MS);

    if (strcmp(data_type, "hum") == 0) {
      measurement = getHumidity();
    } else {
      measurement = getTemperature();
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
    sprintf(measurement_string, "%.1f", measurement);
    printf("Publishing to topic: %s", topic);
    mqtt_publish(client, topic, measurement_string);
}

void cleanup(esp_mqtt_client_handle_t client) {
    char hum_topic[30] = "";
    char temp_topic[30] = "";

    get_topic(hum_topic, "/humidity");
    get_topic(temp_topic, "/temperature");

    mqtt_publish(client, hum_topic, ENDL_MSG);
    mqtt_publish(client, temp_topic, ENDL_MSG);

    vTaskDelay(500 / portTICK_PERIOD_MS);
    mqtt_cleanup(client);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    stop_wifi();
    vTaskDelay(500 / portTICK_PERIOD_MS);
}
