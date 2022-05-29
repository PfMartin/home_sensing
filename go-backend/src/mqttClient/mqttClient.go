package mqttClient

import (
  "fmt"
  "log"
  "os"
  "os/signal"
  "syscall"
  "strconv"
  "go-backend/dataProcessor"

  mqtt "github.com/eclipse/paho.mqtt.golang"
)

var processor dataProcessor.DataProcessor

func SetupClient() mqtt.Client {

  processor = dataProcessor.DataProcessor{}
  processor.Init()

  var connectionHandler mqtt.OnConnectHandler = func(client mqtt.Client) {
    log.Println("Connected")
  }


  var connectLostHandler mqtt.ConnectionLostHandler = func(client mqtt.Client, err error) {
    log.Printf("Connection lost: %v\n", err)
  }

  broker := "192.168.178.44"
  port := 1884

  opts := mqtt.NewClientOptions()
  opts.AddBroker(fmt.Sprintf("tcp://%s:%d", broker, port))
  opts.SetClientID("Raspi")
  opts.OnConnect = connectionHandler
  opts.OnConnectionLost = connectLostHandler
  client := mqtt.NewClient(opts)

  return client
}

func Connect(client mqtt.Client) {
  if token := client.Connect(); token.Wait() && token.Error() != nil {
    panic(token.Error())
  }
}

func Subscribe(client mqtt.Client, topics [2]string) {
  c := make(chan os.Signal, 1) // Create channel for incoming messages
  signal.Notify(c, os.Interrupt, syscall.SIGTERM)

  for _, topic := range topics {
    token := client.Subscribe(topic, 1, onMessageReceived)
    token.Wait()
    log.Printf("Subscribed to topic: %s\n", topic)
  }

  <-c // Read from channel but let incoming messages be processed by onMessageReceived
}

func onMessageReceived(client mqtt.Client, message mqtt.Message) {
  topic := message.Topic()

  const bitSize = 64
  value, err := strconv.ParseFloat(string(message.Payload()), bitSize)
  if err == nil {
    processor.AddData(topic, value)
  }


  log.Println(processor.HumData)
  log.Println(processor.TempData)

}

// Only accept data within 3 standard deviations
// func removeOutliers(value float64) float64 {
//
// }
