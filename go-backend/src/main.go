package main

import (
  "fmt"
  "os"
  "os/signal"
  "syscall"

  mqtt "github.com/eclipse/paho.mqtt.golang"
)


func main() {
  var client mqtt.Client = setupClient()
  connect(client)

  var topics = [2]string{"worms/temperature", "worms/humidity"}
  subscribe(client, topics)
}

func setupClient() mqtt.Client {
  var connectionHandler mqtt.OnConnectHandler = func(client mqtt.Client) {
    fmt.Println("Connected")
  }

  var connectLostHandler mqtt.ConnectionLostHandler = func(client mqtt.Client, err error) {
    fmt.Printf("Connection lost: %v", err)
  }

  var broker = "192.168.178.44"
  var port = 1884

  opts := mqtt.NewClientOptions()
  opts.AddBroker(fmt.Sprintf("tcp://%s:%d", broker, port))
  opts.SetClientID("Martin")
  opts.OnConnect = connectionHandler
  opts.OnConnectionLost = connectLostHandler
  client := mqtt.NewClient(opts)

  return client
}

func connect(client mqtt.Client) {
  if token := client.Connect(); token.Wait() && token.Error() != nil {
    panic(token.Error())
  }
}

func subscribe(client mqtt.Client, topics [2]string) {
  c := make(chan os.Signal, 1) // Create channel for incoming messages
  signal.Notify(c, os.Interrupt, syscall.SIGTERM)

  for _, topic := range topics {
    token := client.Subscribe(topic, 1, onMessageReceived)
    token.Wait()
    fmt.Printf("Subscribed to topic: %s\n", topic)
  }

  <-c // Read from channel but let incoming messages be processed by onMessageReceived
}

func onMessageReceived(client mqtt.Client, message mqtt.Message) {
  fmt.Printf("Received message on topic: %s\nMessage: %s\n", message.Topic(), message.Payload())
}
