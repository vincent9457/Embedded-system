import serial
import requests
import time

ser = serial.Serial('COM3', 115200, timeout=1)

api_url = "http://127.0.0.1:8000/upload/"

while True:
    try:
        line = ser.readline().decode().strip()
        if line:
            print("串口收到：", line)
            parts = line.split(',')
            if len(parts) == 2:
                temp = float(parts[0])
                hum = float(parts[1])

                # 傳送 id = 1
                payload1 = {"id": 1, "temperature": temp, "humidity": hum}
                response1 = requests.post(api_url, json=payload1)
                print("上傳結果 id=1:", response1.text)

                # 傳送 id = 2
                payload2 = {"id": 2, "temperature": temp, "humidity": hum}
                response2 = requests.post(api_url, json=payload2)
                print("上傳結果 id=2:", response2.text)

        time.sleep(5)  # 每 5 秒傳一次
    except Exception as e:
        print("錯誤：", e)
