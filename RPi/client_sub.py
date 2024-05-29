import paho.mqtt.client as mqtt
import time


def on_connect(client, userdata, flags, rc):
   global flag_connected
   flag_connected = 1
   client_subscriptions(client)
   print("Connected to MQTT server")

def on_disconnect(client, userdata, rc):
   global flag_connected
   flag_connected = 0
   print("Disconnected from MQTT server")
   
# a callback functions 
def callback_esp32_sensor1(client, userdata, msg):
    print('ESP sensor1 data: ', msg.payload.decode('utf-8'))

def callback_stue_motionsensor(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

def callback_koekken_light(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

def callback_koekken_motionsensor(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

def callback_sove_motionsensor(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))


def callback_rpi_broadcast(client, userdata, msg):
    print('RPi Broadcast message:  ', str(msg.payload.decode('utf-8')))

def callback_all(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))
        # print("Topics are detected, msg: "+ msg.payload.decode('utf-8'))
        # # client.unsubscribe("#")


def client_subscriptions(client):
    # client.subscribe("esp32/#")
    # client.subscribe("stue/#")
    # client.subscribe("sove/#")
    # client.subscribe("koekken/#")
    # client.subscribe("rpi/broadcast")
    client.subscribe("#")

client = mqtt.Client("rpi_client1") #this should be a unique name
flag_connected = 0

client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.message_callback_add('#', callback_all)

# client.message_callback_add('stue/motionsensor', callback_stue_motionsensor)
# client.message_callback_add('stue/blind_pos', callback_stue_motionsensor)

# client.message_callback_add('koekken/motionsensor', callback_koekken_motionsensor)
# client.message_callback_add('koekken/light', callback_koekken_light)

# client.message_callback_add('sove/motionsensor', callback_sove_motionsensor)
# client.message_callback_add('sove/blind_pos', callback_sove_motionsensor)

# client.message_callback_add('rpi/broadcast', callback_rpi_broadcast)
client.connect('192.168.1.100',1883)
# start a new thread
client.loop_start()
client_subscriptions(client)
print("......client setup complete............")


while True:
    time.sleep(4)
    if (flag_connected != 1):
        print("trying to connect MQTT server..")
        
