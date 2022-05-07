#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "mqtt_client.h"
#include "mqtt.h"

#define MQTT_BROKER_HOST    "ubuntu"
#define MQTT_BROKER_PORT    1884
#define MQTT_TAG            "MQTT_TCP"
#define LOCATION            "worms"

char topic_hum[30] = "";
char topic_temp[30] = "";

void get_topic(char *target, char measurement_type[15]) {
    char topic[30] = LOCATION;

    strncat(topic, measurement_type, 25);
    strncat(target, topic, 25);
}

esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    char connect_msg[31] = "ESP32 connected to MQTT Broker";
    esp_mqtt_client_handle_t client = event->client;

    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_publish(client, topic_hum, connect_msg, 0, 1, 0);
        esp_mqtt_client_publish(client, topic_temp, connect_msg, 0, 1, 0);
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

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(MQTT_TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}


esp_mqtt_client_handle_t mqtt_app_start(void)
{
    get_topic(topic_hum, "/humidity");
    get_topic(topic_temp, "/temperature");

    esp_mqtt_client_config_t mqtt_cfg = {
        .host = MQTT_BROKER_HOST,
        .port = MQTT_BROKER_PORT,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    return client;
}

void mqtt_publish(esp_mqtt_client_handle_t client, char topic[20], char data[5]) {
    esp_mqtt_client_publish(client, topic, data, 0, 1, 1);
}

void mqtt_cleanup(esp_mqtt_client_handle_t client) {
    esp_mqtt_client_stop(client);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp_mqtt_client_disconnect(client);

}
