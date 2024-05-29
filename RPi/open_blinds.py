# import paho.mqtt.client as mqtt
# import os
# import datetime
# import logging


# class CappedLogFileHandler(logging.FileHandler):
#     def __init__(self, filename, capacity=500, mode='a', encoding=None, delay=False):
#         super().__init__(filename, mode, encoding, delay)
#         self.capacity = capacity
#         self.truncate_log()

#     def emit(self, record):
#         if self.should_truncate_log():
#             self.truncate_log()
#         super().emit(record)

#     def should_truncate_log(self):
#         with open(self.baseFilename, 'r') as file:
#             return sum(1 for line in file) >= self.capacity

#     def truncate_log(self):
#         with open(self.baseFilename, 'r+') as file:
#             lines = file.readlines()
#             file.seek(0)
#             file.truncate()
#             file.writelines(lines[-self.capacity:])


# logger = logging.getLogger('capped_logger')
# logger.setLevel(logging.INFO)

# # Define the log file name and capacity
# log_file_name = '/home/pi/code/Home_IoT/RPi/log.txt'
# log_capacity = 500 #retain last 500 lines

# capped_handler = CappedLogFileHandler(log_file_name, capacity=log_capacity)
# formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
# capped_handler.setFormatter(formatter)

# logger.addHandler(capped_handler)


# def open_blinds(client):
#     try:
#         msg = "25"
#         pubMsg = client.publish(
#             topic='stue/set_blind_pos',
#             payload=msg.encode('utf-8'),
#             qos=0,
#         )
#         pubMsg.wait_for_publish()

#         pubMsg = client.publish(
#             topic='sove/set_blind_pos',
#             payload=msg.encode('utf-8'),
#             qos=0,
#         )
#         pubMsg.wait_for_publish()

#         logger.info("stue and bedroom blinds opened!")


#     except Exception as e:
#         logger.error(e)
    
    
# def main(args=None):
#     client = mqtt.Client("rpi_morning script")
#     client.connect('192.168.1.100',1883)

#     client.loop_start()

#     open_blinds(client)

#     client.loop_stop()


# if __name__ == '__main__':
#     main()


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


def open_blinds(logger, client, message):
    try:
        pubMsg = client.publish(
            topic="sove/set_blind_pos",
            payload=message.encode('utf-8'),
            qos=0,
        )
        pubMsg = client.publish(
            topic="stue/set_blind_pos",
            payload=message.encode('utf-8'),
            qos=0,
        )

        pubMsg.wait_for_publish()

    except Exception as e:
        print(e)
        logger.error(str(e))


def on_message(client, userdata, msg):
    global status1_received, status1_message, status2_received, status2_message

    if msg.topic == 'stue/get_blind_pos':
        status1_message = msg.payload.decode('utf-8')
        status1_received = True
    elif msg.topic == 'sove/get_blind_pos':
        status2_message = msg.payload.decode('utf-8')
        status2_received = True
    


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

    global status1_received, status2_received, status1_message, status2_message
    status1_received = False
    status2_received = False
    status1_message = ""
    status2_message = ""

    client.loop_start()

    client.subscribe('stue/get_blind_pos')
    client.subscribe('sove/get_blind_pos')

    message = "25"

    open_blinds(logger, client, message)

    start_time = time.time()

    while (not status1_received or not status2_received) and (time.time() - start_time) < 10:
        time.sleep(0.1)

    client.loop_stop()

    if status1_received and status2_received:
        if status1_message == message and status2_message == message:
            logger.info("Both devices returned success")
        else:
            logger.warning(f"Mismatched values! Should be: {message}, got from livingroom: {status1_message}, from bedroom: {status2_message}")
    else:
        if not status1_received:
            logger.warning("No response from livingroom within 5 seconds")
        if not status2_received:
            logger.warning("No response from bedroom within 5 seconds")



if __name__ == "__main__":
    main()
