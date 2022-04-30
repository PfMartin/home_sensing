from paho.mqtt import client as mqtt_client
import os
from os.path import join, dirname
from dotenv import load_dotenv
from datetime import datetime
from database_client import DatabaseClient
from IqrFilter import IqrFilter

dotenv_path = join(dirname(__file__), "..", "..", "Mqtt.env")
load_dotenv(dotenv_path)

temperatures = []
humidities = []

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code: {rc}")


def on_message(client, userdata, msg):
    if msg.payload.decode() != "ESP32 connected to MQTT Broker":
        [location, topic] = msg.topic.split("/")
        timestamp = datetime.now()

        data_point = {
            "location": location,
            "topic": topic,
            "value": msg.payload.decode()
        }

        if (topic == 'temperature'):
            temperatures.append(data_point)
        elif (topic == 'humidity'):
            humidities.append(data_point)

        if (len(temperatures) >= 20):
            print("\nNOW IT's FILTERING TIME FOR TEMPS")
            temperatures.clear()

        if (len(humidities) >= 20):
            print("\nNOW IT'S FILTERING TIME FOR HUMS")
            humidities.clear()

        # database_client = DatabaseClient()
        # database_client.connect()
        # database_client.insert_data(topic, msg.payload.decode(), timestamp, location)
        #
        # database_client.disconnect()


def init_client():
    client = mqtt_client.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    return client


def subscribe():
    topics = [os.environ.get("MQTT_TOPIC_TEMP"), os.environ.get("MQTT_TOPIC_HUM")]

    for topic in topics:
        client.subscribe(topic, 0)


if __name__ == "__main__":
    client = init_client()

    client.connect(
        os.environ.get("MQTT_BROKER_HOST"), int(os.environ.get("MQTT_BROKER_PORT")), 60
    )

    subscribe()

    client.loop_forever()
