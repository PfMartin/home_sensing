package mqttClient

import (
  "fmt"
  "os"
  "os/signal"
  "syscall"
  "strconv"

  mqtt "github.com/eclipse/paho.mqtt.golang"
)

var humData []float64
var tempData []float64

func SetupClient() mqtt.Client {
  humData = make([]float64, 20)
  tempData = make([]float64, 20)

  var connectionHandler mqtt.OnConnectHandler = func(client mqtt.Client) {
    fmt.Println("Connected")
  }


  var connectLostHandler mqtt.ConnectionLostHandler = func(client mqtt.Client, err error) {
    fmt.Printf("Connection lost: %v\n", err)
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
    fmt.Printf("Subscribed to topic: %s\n", topic)
  }

  <-c // Read from channel but let incoming messages be processed by onMessageReceived
}

func onMessageReceived(client mqtt.Client, message mqtt.Message) {
  topic := message.Topic()

  fmt.Printf("Received message on topic: %s\nMessage: %s\n", topic, message.Payload())


  const bitSize = 64
  value, err := strconv.ParseFloat(string(message.Payload()), bitSize)
  if err == nil {
    // data = append(data, floatNum)
    if (topic == "worms/humidity") {
      addToStack(humData, value)
    } else if (topic == "worms/temperature") {
      addToStack(tempData, value)
    }
  }


  fmt.Println(humData)
  fmt.Println(tempData)

}

func addToStack(a []float64, value float64) {
  for index, _ := range a {
    if index + 1 < cap(a) {
      a[index] = a[index + 1]
    }
  }

  a[cap(a) - 1] = value
}

// Only accept data within 3 standard deviations
// func removeOutliers(value float64) float64 {
//
// }
