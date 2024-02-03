import requests
import logging
from secrets.secrets import api_key, lat, lon
import paho.mqtt.client as mqtt

logging.basicConfig(filename = '/home/pi/simon/code/Home_IoT/RPi/weather_log.txt', level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')


url = f'https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={api_key}'


client = mqtt.Client("rpi_weather_client") #this name should be unique
client.connect('192.168.1.100',1883)
client.loop_start()


def open_blinds():
    try:
        msg = "100"
        pubMsg = client.publish(
            topic='stue/set_blind_pos',
            payload=msg.encode('utf-8'),
            qos=0,
        )
    
        pubMsg.wait_for_publish()
        logging.info("Blinds opened due to high wind!")

    except Exception as e:
        print(e)
        logging.error(e)


response = requests.get(url)

if response.status_code == 200:
    data = response.json()
    wind_speed = data['wind']['speed']
    # print(f'Wind speed: {wind_speed} m/s')
    logging.info(f' Wind speed: {wind_speed} m/s')
    if wind_speed > 15:
        open_blinds()
        
else:
    error_message = f"Failed to retrieve weather data. Status code: {response.status_code}"
    logging.error(error_message)
    print(error_message)