import json
from flask import Flask, render_template
import paho.mqtt.client as mqtt

app = Flask(__name__)

# Global variable to store the sensor value
sensor_value = None

# MQTT settings
mqtt_server = "172.20.10.12"
mqtt_port = 1883
mqtt_user = "wz"
mqtt_password = "123456"
mqtt_topic = "testTopic"

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with result code: {rc}")
    client.subscribe(mqtt_topic)

def on_message(client, userdata, msg):
    global sensor_value
    sensor_value = msg.payload.decode("utf-8")
    print(f"Received message on topic {msg.topic}: {sensor_value}")

client = mqtt.Client()
client.username_pw_set(mqtt_user, mqtt_password)
client.on_connect = on_connect
client.on_message = on_message

client.connect(mqtt_server, mqtt_port, 60)
client.loop_start()

@app.route("/")
def index():
    return render_template("index.html", value=sensor_value)

@app.route("/data")
def data():
    return str(sensor_value)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
