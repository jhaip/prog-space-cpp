import requests
import urllib.parse
import time

def update():
    # get wishes from broker
    try:
        resp = requests.get("http://localhost:9090/select?query={}".format(
            urllib.parse.quote_plus("[\"$ wish $url would be thermal printed\"]")))
        # [{'url': ['', 'http://...']}]
        results = resp.json()
        print(results)
        if len(results) > 0:
            batch = [{"type": "retract", "fact": "$ wish $ would be thermal printed"}]
            req = requests.post('http://localhost:9090/batch', json=batch)
            print(req)
            for r in results:
                url = r['url'][1]
                print("thermal printing: {}".format(url))
    except requests.exceptions.RequestException as e:  # This is the correct syntax
        print("could not connect to broker")

while True:
    update()
    time.sleep(1)