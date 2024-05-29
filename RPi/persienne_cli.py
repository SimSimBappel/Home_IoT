import time
import paho.mqtt.client as mqtt
import os
import datetime
import logging

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


def open_blinds(logger, client, topic, message):
    try:
        pubMsg = client.publish(
            topic=topic,
            payload=message.encode('utf-8'),
            qos=0,
        )

        pubMsg.wait_for_publish()
        print("message sent!")

    except Exception as e:
        print(e)
        logger.error(str(e))


def get_user_input(logger):
    print("Select a topic to publish to:")
    print("1. stue/set_blind_pos")
    print("2. bedroom/set_blind_pos")
    print("Else; enter custom topic")

    selection = input("Enter the number corresponding to your choice (or 4 for custom): ")

    if selection == '1':
        topic = 'stue/set_blind_pos'
    elif selection == '2':
        topic = 'sove/set_blind_pos'
    else:
        print("Invalid, type custom topic")
        topic = input("Enter the custom topic: ")


    message = input("Enter the message to publish: ")

    logger.info(f"Sending; {message}, to {topic}")

    return topic, message


def on_message(client, userdata, msg):
    global status_received, status_message
    status_message = msg.payload.decode('utf-8')
    status_received = True
    print(f"Received: {status_message}, on:{msg.topic}")
    


def main():
    logger = logging.getLogger('capped_logger')
    logger.setLevel(logging.INFO)

    log_file_name = '/home/pi/code/Home_IoT/RPi/log.txt'
    log_capacity = 500 #lines to retain

    capped_handler = CappedLogFileHandler(log_file_name, capacity=log_capacity)
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    capped_handler.setFormatter(formatter)

    logger.addHandler(capped_handler)
    client = mqtt.Client("CLI")
    client.on_message = on_message
    client.connect('192.168.1.100', 1883)

    global status_received, status_message 
    status_received = False
    status_message = ""

    client.loop_start()

    client.subscribe('stue/get_blind_pos')
    client.subscribe('stue/blind_error')
    client.subscribe('sove/get_blind_pos')
    client.subscribe('stue/blind_error')

    topic, message = get_user_input(logger)
    open_blinds(logger, client, topic, message)

    start_time = time.time()


    while not status_received and (time.time() - start_time) < 10:
        time.sleep(0.1)

    client.loop_stop()

    if status_received:
        if status_message == message:
            logger.info("Device returned successfully")
        else:
            logger.warning(f"Mismatched values! Should be: {message}, got {status_message}")
    else:
        logger.error(f"No reply from topic: {topic}")


if __name__ == "__main__":
    main()
