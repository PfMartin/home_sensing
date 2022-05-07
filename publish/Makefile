PATH_TO_IDF=/home/martin/esp/esp-idf

build:
	python3 ${PATH_TO_IDF}/tools/idf.py build

upload:
	python3 ${PATH_TO_IDF}/tools/idf.py -p /dev/ttyUSB0 flash

upload-monitor:
	python3 ${PATH_TO_IDF}/tools/idf.py -p /dev/ttyUSB0 flash monitor
