from paho.mqtt import client as mqtt_client
import os
from os.path import join, dirname
from dotenv import load_dotenv
from datetime import datetime
from DatabaseClient import DatabaseClient
from IqrFilter import IqrFilter

dotenv_path = join(dirname(__file__), "..", "..", "Mqtt.env")
load_dotenv(dotenv_path)

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code: {rc}")


def on_message(client, userdata, msg):
    if msg.payload.decode() != "ESP32 connected to MQTT Broker":
        [location, topic] = msg.topic.split("/")
        timestamp = datetime.now()
        message = msg.payload.decode()

        value_list = [float(value) for value in message.split('|')[1:]]

        iqr_filter = IqrFilter(value_list)
        mean_value = iqr_filter.clean_mean()

        print(f'{location}/{topic} | {timestamp} | {mean_value}')

        database_client = DatabaseClient()
        database_client.connect()
        database_client.insert_data(topic, mean_value, timestamp, location)

        database_client.disconnect()


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
