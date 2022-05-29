package dataProcessor

import (
  "fmt"
)

type DataProcessor struct {
  HumData []float64
  TempData []float64
}

func (d *DataProcessor) Init() {
  d.HumData = make([]float64, 20)
  d.TempData = make([]float64, 20)
}

func (d *DataProcessor) AddData(topic string, value float64) {
  if (topic == "worms/humidity") {
    addToStack(d.HumData, value)
  } else if (topic == "worms/temperature") {
    addToStack(d.TempData, value)
  }
}

func addToStack(a []float64, value float64) {

  fmt.Println(len(a))
  for index, _ := range a {
    if index + 1 < cap(a) {
      a[index] = a[index + 1]
    }
  }

  a[cap(a) - 1] = value
}
