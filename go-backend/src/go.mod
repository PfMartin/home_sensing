module home_sensing/backend

go 1.18

replace mqttClient => ./mqttClient

require (
	github.com/eclipse/paho.mqtt.golang v1.3.5 // indirect
	github.com/gorilla/websocket v1.4.2 // indirect
	golang.org/x/net v0.0.0-20200425230154-ff2c4b7c35a0 // indirect
	mqttClient v1.0.0
)
