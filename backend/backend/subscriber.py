from paho.mqtt import client as mqtt_client
import os
from os.path import join, dirname
from dotenv import load_dotenv
from time import time

dotenv_path = join(dirname(__file__), "..", "..", "Mqtt.env")
load_dotenv(dotenv_path)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

def on_message(client, userdata, msg):
    print(f'{msg.topic} | {msg.payload.decode()} | {int(time())}')

client = mqtt_client.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(os.environ.get("MQTT_BROKER_HOST"), int(os.environ.get("MQTT_BROKER_PORT")), 60)

client.subscribe(os.environ.get("MQTT_TOPIC_TEMP"), 0)
client.subscribe(os.environ.get("MQTT_TOPIC_HUM"), 0)

client.loop_forever()
