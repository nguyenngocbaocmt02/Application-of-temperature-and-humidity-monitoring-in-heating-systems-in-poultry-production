from urllib import request
import paho.mqtt.client as paho
from paho import mqtt
from flask import Flask, jsonify, request, render_template
from flask_cors import CORS, cross_origin
from flask_mqtt import Mqtt
import mysql.connector as sql
from datetime import datetime as dt
import json
import datetime
from flask_mysqldb import MySQL
import pandas as pd
import plotly.graph_objs as go
import plotly.io as pio
import schedule
import time
from threading import Thread


mydb = sql.connect(
    host = 'localhost',
    port="3307",
    user = 'root',
    password = '123456',
    database = 'IOT', 
    auth_plugin='mysql_native_password'
)

app = Flask(__name__, template_folder='./templates', static_folder='./static')
app.config['MQTT_BROKER_URL'] = '192.168.82.227'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = 'bao'
app.config['MQTT_PASSWORD'] = '123456'
app.config['MQTT_REFRESH_TIME'] = 1.0  # refresh time in seconds

app.config['MYSQL_HOST'] = 'localhost'
app.config['MYSQL_USER'] = 'root'
app.config['MYSQL_PASSWORD'] = '123456'
app.config['MYSQL_DB'] = 'IOT'
app.config['MYSQL_PORT'] = 3307
mysql = MySQL(app)

mqtt = Mqtt(app)
CORS(app)
app.config['CORS_HEADERS'] = 'Content-Type'

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe('log', qos=0)

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    # data = json.loads(message.payload)
    # print(data)
    mycursor = mydb.cursor()
    sql = "INSERT INTO `log` (`time`, `temp`, `hum`) VALUES (%s, %s, %s)"
    key = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    data = message.payload.decode('utf-8')
    data = data.split(" ")
    print(data)

    val = (key, float(data[0]), float(data[1]))
    mycursor.execute(sql, val)
    mydb.commit()

def update():
    with app.app_context():
        cur = mysql.connection.cursor()
        cur.execute("SELECT * FROM log")
    data = cur.fetchall()

    df = pd.DataFrame(list(data), columns=['time', 'temp', 'hum'])

    temp_chart = go.Scatter(x=df['time'], y=df['temp'], name='Temperature')
    hum_chart = go.Scatter(x=df['time'], y=df['hum'], name='Humidity')

    temp_layout = go.Layout(title='Temperature', xaxis=dict(title='Time'), yaxis=dict(title='Temperature'))
    hum_layout = go.Layout(title='Humidity', xaxis=dict(title='Time'), yaxis=dict(title='Humidity'))

    temp_fig = go.Figure(data=temp_chart, layout=temp_layout)
    hum_fig = go.Figure(data=hum_chart, layout=hum_layout)

    temp_html = pio.to_html(temp_fig)
    hum_html = pio.to_html(hum_fig)
    with app.app_context():
        return render_template('dashboard.html', temp_fig=temp_html, hum_fig=hum_html)


schedule.every(5).seconds.do(update)


@app.route('/')
def index():
    return update()


def run_scheduler():
    while True:
        schedule.run_pending()
        time.sleep(1)

if __name__ == '__main__':
    thread = Thread(target=run_scheduler)
    thread.start()
    app.run(debug=True)