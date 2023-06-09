import time
import paho.mqtt.client as paho
from paho import mqtt
import json
import datetime
import random

def on_connect(client, userdata, flags, rc, properties=None):
    print("CONNACK received with code %s." % rc)

def on_publish(client, userdata, mid, properties=None):
    print("mid: " + str(mid))

def on_subscribe(client, userdata, mid, granted_qos, properties=None):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))

if __name__ == '__main__':
    client = paho.Client(client_id="", userdata=1, protocol=paho.MQTTv5)
    client.on_connect = on_connect

    client.username_pw_set("bao", "123456")
    client.connect("192.168.1.62", 1883)

    client.on_subscribe = on_subscribe
    client.on_message = on_message
    client.on_publish = on_publish

    while True:
        time.sleep(2)
        client.publish("log", payload=str(random.uniform(0, 50)) + " " + str(random.uniform(0, 50)), qos=0)