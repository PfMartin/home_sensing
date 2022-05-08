package main

import (
  "mqttClient"
  mqtt "github.com/eclipse/paho.mqtt.golang"
)


func main() {
  var client mqtt.Client = mqttClient.SetupClient()
  mqttClient.Connect(client)

  var topics = [2]string{"worms/temperature", "worms/humidity"}
  mqttClient.Subscribe(client, topics)
}
