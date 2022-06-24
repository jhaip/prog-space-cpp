"""
UART Service
-------------

An example showing how to write a simple program using the Nordic Semiconductor
(nRF) UART service.

"""

import asyncio
import sys

from bleak import BleakScanner, BleakClient
from bleak.backends.scanner import AdvertisementData
from bleak.backends.device import BLEDevice
import requests
import urllib.parse
import time
import json
import math

SERVER_URL = "http://192.168.1.22"

UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
UART_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
UART_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

# All BLE devices have MTU of at least 23. Subtracting 3 bytes overhead, we can
# safely send 20 bytes at a time to any device supporting this service.
UART_SAFE_SIZE = 20


async def onSubscriptionUpdate(deviceSubscriptionId, raw_results, client):
    print("subscription update", raw_results)

    # convert value type from ["", "val"] to just val
    results = raw_results
    for i,result in enumerate(results):
        new_result = {}
        for k in result:
            new_result[k] = int(result[k][1]) # TODO: handle other types
        results[i] = new_result

    serialized_result = json.dumps(results)[1:-1]
    result_ble_msg = "{}{}\n".format(deviceSubscriptionId, serialized_result)
    msgBytes = result_ble_msg.encode("utf-8")
    chunkSize = 20
    for i in range(0, math.ceil(len(msgBytes)/chunkSize)):
        chunk = msgBytes[(i*chunkSize):((i+1)*chunkSize)];
        await client.write_gatt_char(UART_RX_CHAR_UUID, chunk)
        print("wrote chunk {}".format(i))


async def uart_terminal():
    """This is a simple "terminal" program that uses the Nordic Semiconductor
    (nRF) UART service. It reads from stdin and sends each line of data to the
    remote device. Any data received from the device is printed to stdout.
    """

    def match_nus_uuid(device: BLEDevice, adv: AdvertisementData):
        # This assumes that the device includes the UART service UUID in the
        # advertising data. This test may need to be adjusted depending on the
        # actual advertising data supplied by the device.
        if UART_SERVICE_UUID.lower() in adv.service_uuids:
            return True

        return False

    device = await BleakScanner.find_device_by_filter(match_nus_uuid)

    def handle_disconnect(_: BleakClient):
        print("Device was disconnected, goodbye.")
        # cancelling all tasks effectively ends the program
        for task in asyncio.all_tasks():
            task.cancel()

    async def handle_rx(_: int, data: bytearray):
        print("received:", data)
        s = data.decode()
        batch = [{"type": "retract", "fact": "$ ble {} says %".format(device.address)}]
        if s[:2] == "N:":
            batch.append({"type": "claim", "fact": "#0ble ble {} says {}".format(device.address, s[2:-1])})
        elif s[:2] == "S:":
            subscription_id = s[2:6]
            sub_query_str = s[7:-1]
            sub_query_parts = sub_query_str.split("::")
            while True:
                resp = requests.get("http://localhost:9090/select?query={}".format(
                    urllib.parse.quote_plus(json.dumps(sub_query_parts))))
                # [{'url': ['', 'http://...']}]
                results = resp.json()
                print(results)
                # BUG: progspace_room.mpy doesn't seem to handle empty results
                if len(results) > 0:
                    await onSubscriptionUpdate(subscription_id, results, client)
                time.sleep(0.25)
        print("SENDING:")
        print(batch)
        req = requests.post(SERVER_URL+':9090/batch', json=batch)

    async with BleakClient(device, disconnected_callback=handle_disconnect) as client:
        await client.start_notify(UART_TX_CHAR_UUID, handle_rx)

        print("Connected, start typing and press ENTER...")

        loop = asyncio.get_running_loop()

        while True:
            # This waits until you type a line and press ENTER.
            # A real terminal program might put stdin in raw mode so that things
            # like CTRL+C get passed to the remote device.
            data = await loop.run_in_executor(None, sys.stdin.buffer.readline)

            # data will be empty on EOF (e.g. CTRL+D on *nix)
            if not data:
                break

            # some devices, like devices running MicroPython, expect Windows
            # line endings (uncomment line below if needed)
            # data = data.replace(b"\n", b"\r\n")

            await client.write_gatt_char(UART_RX_CHAR_UUID, data)
            print("sent:", data)


if __name__ == "__main__":
    try:
        asyncio.run(uart_terminal())
    except asyncio.CancelledError:
        # task is cancelled on disconnect, so we ignore this error
        pass