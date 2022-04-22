# Publish

Starts a FreeRTOS task to publish data.

## Wiring
1. 4.7 k&#8486; pull up resistor for the data line
1. GPIO4 - 7th on the right from the bottom

## Workflow for building the software

1. Export the idf environment variable, in order to be able to use the environment variable $IDF_PATH
    ```bash
    . $HOME/esp/esp-idf/export.sh
    ```
1. Build the project
    ```bash
    python3 $IDF_PATH/tools/idf.py menuconfig
    python3 $IDF_PATH/tools/idf.py build
    ```
1. Flash the controller
    ```bash
    sudo dmesg | grep tty # find the COM Port
    python3 $IDF_PATH/tools/idf.py -p /dev/ttyUSB0 flash
    python3 $IDF_PATH/tools/idf.py -p /dev/ttyUSB0 flash monitor
    ```
1. Monitor the output separately
    ```bash
    screen /dev/ttyUSB0 115200
    ```
1. Close monitoring with `STRG+a` then `k` then `y`. You enter control mode with `STRG+a`

## Preset for this example
[Tutorial](https://github.com/SIMS-IOT-Devices/MQTT-ESP-IDF/blob/main/mqtt_tcp_pub_sub.c)

## Deep sleep and wakeup

[Example Code](https://github.com/espressif/esp-idf/blob/master/examples/system/deep_sleep/main/deep_sleep_example_main.c)

1. Configure deep sleep wakeup options
  - esp_err_t esp_sleep_enable_timer_wakeup(uint64_t time_in_us)
1. Put into deep sleep after reading the first set of data
  - esp_err_t esp_deep_sleep_start(void)
