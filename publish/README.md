# Publish

Starts a FreeRTOS task to publish data.

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
