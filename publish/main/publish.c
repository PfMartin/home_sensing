// mqtt: https://github.com/SIMS-IOT-Devices/MQTT-ESP-IDF/blob/main/mqtt_tcp_pub_sub.c
// i2c: https://github.com/espressif/esp-idf/blob/master/examples/peripherals/i2c/i2c_simple/main/i2c_simple_main.c

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"
#include "driver/i2c.h"

#define SSID                        "FRITZ!Box 7582 PJ"
#define PASSPHRASE                  "95605533072376088713"

#define I2C_MASTER_SCL              22
#define I2C_MASTER_SDA              21
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000

#define DPS310_SENSOR_ADDR          0x77
#define DPS310_PRESSURE_ADDR        0x30


static const char *MQTT_TAG = "MQTT_TCP";
static const char *I2C_TAG = "I2C";

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

void wifi_connection()
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
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(client, "worms", 0);
        esp_mqtt_client_publish(client, "worms", "ESP32 publishing to topic 'worms'", 0, 1, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
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
        // .uri = "mqtt://mqtt.eclipseprojects.io",
        .host = "192.168.178.40",
        .port = 1884,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    return client;
}

static esp_err_t i2c_master_init(void) {
  int i2c_master_port = I2C_MASTER_NUM;

  i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_MASTER_SDA,
    .scl_io_num = I2C_MASTER_SCL,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  i2c_param_config(i2c_master_port, &conf);

  return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static esp_err_t dps310_register_read(uint8_t reg_addr, uint8_t *data, size_t len) {
  return i2c_master_write_read_device(I2C_MASTER_NUM, DPS310_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

static esp_err_t dps310_register_write_byte(uint8_t reg_addr, uint8_t data) {
  int ret;

  uint8_t write_buf[2] = { reg_addr, data };
  ret = i2c_master_write_to_device(I2C_MASTER_NUM, DPS310_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

  return ret;
}

void app_main(void)
{
    nvs_flash_init();
    wifi_connection();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n");

    esp_mqtt_client_handle_t client = mqtt_app_start();

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    ESP_LOGI(I2C_TAG, "Initializing I2C");
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(I2C_TAG, "I2C initialized successfully!");

    vTaskDelay(2000 / portTICK_PERIOD_MS);


    // Read coefficients 0x10 - 0x20

    // Configure pressure measurements 0x06
    ESP_ERROR_CHECK(dps310_register_write_byte(0x06, 0x00));

    // Configure temperature measurements 0x07
    ESP_ERROR_CHECK(dps310_register_write_byte(0x07, 0x00));

    // Configure interrupt FIFO configuration 0x09
    ESP_ERROR_CHECK(dps310_register_write_byte(0x09, 0x00));




    while (1) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);

      // Set to temperature measurement mode
      ESP_ERROR_CHECK(dps310_register_write_byte(0x08, 0x02));

      uint8_t temp1[2];
      uint8_t temp2[2];
      uint8_t temp3[2];

      // Read temperature 0x03 - 0x05
      ESP_ERROR_CHECK(dps310_register_read(0x03, temp1, 1));
      ESP_ERROR_CHECK(dps310_register_read(0x04, temp2, 1));
      ESP_ERROR_CHECK(dps310_register_read(0x05, temp3, 1));

      uint8_t t_raw = ((temp1[0] << 16) | (temp2[0] << 8) | temp3[0]);
      uint8_t temperature = (double)(t_raw) / (65536)*165 - 40;


      // ESP_ERROR_CHECK(dps310_register_read(DPS310_PRESSURE_ADDR, data, 1));
      // ESP_ERROR_CHECK(dps310_register_read(DPS310_PRESSURE_ADDR, data, 2));


      ESP_LOGI(I2C_TAG, "Temperature: %d", temperature);
      //
      // sprintf(pressure_0, "Pressure 0: %d", data[0]);
      // sprintf(pressure_1, "Pressure 1: %d", data[1]);

      // esp_mqtt_client_publish(client, "worms", pressure_0, 0, 1, 1);
      // esp_mqtt_client_publish(client, "worms", pressure_1, 0, 1, 1);
    }


}
