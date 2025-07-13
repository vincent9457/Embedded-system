import requests

payload = {"Longitude": 125.5, "Latitude": 37.5}

r = requests.post("http://127.0.0.1:8000/G001/", json=payload)
print("上傳完成")