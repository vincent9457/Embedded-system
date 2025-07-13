import requests, json
import pandas as pd

get_rdata = requests.get("http://127.0.0.1:8000/G002/")

print(pd.DataFrame(get_rdata.json()))
print("-----------------------------------------------------")
payload = {"Longitude": 121.5}
post_rdata = requests.post("http://127.0.0.1:8000/G002/", json=payload)
print(pd.DataFrame(post_rdata.json()))