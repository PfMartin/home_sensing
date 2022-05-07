#ifndef MQTT_H
#define MQTT_H

esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

esp_mqtt_client_handle_t mqtt_app_start(void);

void mqtt_publish(esp_mqtt_client_handle_t client, char topic[20], char data[5]);

void mqtt_cleanup(esp_mqtt_client_handle_t client);

#endif /* MQTT_H */
