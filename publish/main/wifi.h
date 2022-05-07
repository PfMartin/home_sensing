#ifndef WIFI_H
#define WIFI_H

void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

esp_err_t init_wifi();

void stop_wifi();

esp_err_t connect_wifi();

#endif /* WIFI_H */
