# Run on haippi9
from escpos import *
from PIL import Image, ImageDraw, ImageFont
from io import BytesIO
from datetime import datetime
import logging
import traceback
import requests
import urllib.parse
import time

p = printer.Usb(0x04b8, 0x0e15)
SERVER_URL = "http://192.168.1.22"

def update():
    global p, SERVER_URL
    try:
        resp = requests.get(SERVER_URL+":9090/select?query={}".format(
            urllib.parse.quote_plus("[\"$ wish $url would be thermal printed\"]")))
        results = resp.json()
        print(results)
        if len(results) > 0:
            batch = [{"type": "retract", "fact": "$ wish $ would be thermal printed"}]
            req = requests.post(SERVER_URL+':9090/batch', json=batch)
            print(req)
            for result in results:
                url = result['url'][1]
                logging.info("PRINTING {} {}".format(datetime.now().isoformat(), url))
                try:
                    if not ("http" in url):
                        url = SERVER_URL+":8000/{}".format(url)
                    PIL_image = Image.open(requests.get(url, stream=True).raw)
                    p.image(PIL_image)
                    p.cut()
                except Exception as e:
                    logging.error(traceback.format_exc())
    except requests.exceptions.RequestException as e:  # This is the correct syntax
        print("could not connect to broker")

while True:
    update()
    time.sleep(1)
