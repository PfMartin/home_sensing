PATH_TO_IDF=/home/martin/esp/esp-idf
MAIN_PATH=./main/

build: ./main/main.c ./main/mqtt.c ./main/mqtt.h ./main/sleep.c ./main/sleep.h ./main/wifi.c ./main/wifi.h
	python3 ${PATH_TO_IDF}/tools/idf.py build

upload:
	python3 ${PATH_TO_IDF}/tools/idf.py -p /dev/ttyUSB0 flash

upload-monitor:
	python3 ${PATH_TO_IDF}/tools/idf.py -p /dev/ttyUSB0 flash monitor
