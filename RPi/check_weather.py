import requests
import logging
from secrets.secrets import api_key, lat, lon
import paho.mqtt.client as mqtt


class CappedLogFileHandler(logging.FileHandler):
    def __init__(self, filename, capacity=500, mode='a', encoding=None, delay=False):
        super().__init__(filename, mode, encoding, delay)
        self.capacity = capacity
        self.truncate_log()

    def emit(self, record):
        if self.should_truncate_log():
            self.truncate_log()
        super().emit(record)

    def should_truncate_log(self):
        with open(self.baseFilename, 'r') as file:
            return sum(1 for line in file) >= self.capacity

    def truncate_log(self):
        with open(self.baseFilename, 'r+') as file:
            lines = file.readlines()
            file.seek(0)
            file.truncate()
            file.writelines(lines[-self.capacity:])


logger = logging.getLogger('capped_logger')
logger.setLevel(logging.INFO)

# Define the log file name and capacity
log_file_name = '/home/pi/code/Home_IoT/RPi/weather_log.txt'
log_capacity = 500 #retain last 500 lines

capped_handler = CappedLogFileHandler(log_file_name, capacity=log_capacity)
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
capped_handler.setFormatter(formatter)

logger.addHandler(capped_handler)

url = f'https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={api_key}'

client = mqtt.Client("rpi_weather_client") #this name should be unique
client.connect('192.168.1.100',1883)
client.loop_start()


def open_blinds(wind_speed):
    try:
        msg = "100"
        pubMsg = client.publish(
            topic='stue/set_blind_pos',
            payload=msg.encode('utf-8'),
            qos=0,
        )
    
        pubMsg.wait_for_publish()
        logging.info(f' Wind speed: {wind_speed} m/s, Blinds opened!')

    except Exception as e:
        # print(e)
        logging.error(f' Wind speed: {wind_speed} m/s, {e}')


response = requests.get(url)

if response.status_code == 200:
    data = response.json()
    wind_speed = data['wind']['speed']
    if wind_speed > 15:
        open_blinds(wind_speed)
    else:
        logging.info(f' Wind speed: {wind_speed} m/s')
        
else:
    error_message = f"Failed to retrieve weather data. Status code: {response.status_code}"
    logging.error(error_message)