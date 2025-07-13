from django.shortcuts import render

# Create your views here.
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
import pymysql, json
from django.http import JsonResponse
# import numpy as np
import os


@csrf_exempt
def G000(request):
    if request.method == 'GET':
        sql = ("CREATE TABLE GPS (ID Int AUTO_INCREMENT,"
               "Longitude Float,"
               "Latitude Float,"
               "Map BLOB,"
               "PRIMARY KEY (ID))")
        try:
            db = pymysql.connect(host='localhost', user='FCU1326', password='vincent20050507', db='test')
            cursor = db.cursor()
            cursor.execute("DROP TABLE IF EXISTS GPS")
            cursor.execute(sql)
            db.commit()
            print("創建完成")
            status = "OK"
        except:
            db.rollback()
            print("創建失敗")
            status = "ERROR"
        db.close()
        return HttpResponse(status)
    
@csrf_exempt
def G001(request):
    if request.method == 'POST':
        data = json.loads(request.body)
        print(data['Longitude'])
        sql = ("INSERT INTO GPS (Longitude, Latitude) VALUES ('%f', '%f')")  % (data['Longitude'], data['Latitude'])
        print(sql)
        try:
            db = pymysql.connect(host='localhost', user='FCU1326', password='vincent20050507', db='test')
            cursor = db.cursor()
            cursor.execute(sql)
            db.commit()
            print("上傳完成")
            status = "OK"
        except:
            db.rollback()
            print("上傳失敗")
            status = "ERROR"
        db.close()
        return HttpResponse(status)

@csrf_exempt
def G002(request):
    if request.method == 'POST':
        print(request.body)
        data = json.loads(request.body)
        sql = ("SELECT * FROM GPS WHERE Longitude >= '%f'") % (data['Longitude'])
        rejdata = []
        try:
            db = pymysql.connect(host='localhost', user='FCU1326', password='vincent20050507', db='test')
            cursor = db.cursor()
            cursor.execute(sql)
            db.commit()
            result = cursor.fetchall()
            for row in result:
                id = row[0]
                longitude = row[1]
                latitude = row[2]
                jdata = {"Id": id, "Longitude": longitude, "Latitude": latitude}
                rejdata.append(jdata)
            print("查詢完成")
            status = "OK"
        except:
            db.rollback()
            print("查詢失敗")
            status = "ERROR"
        db.close()
        return HttpResponse(json.dumps(rejdata), content_type="application/json")
    elif request.method == 'GET':
        sql = ("SELECT * FROM GPS")
        rejdata = []
        try:
            db = pymysql.connect(host='localhost', user='FCU1326', password='vincent20050507', db='test')
            cursor = db.cursor()
            cursor.execute(sql)
            db.commit()
            result = cursor.fetchall()
            for row in result:
                print(rejdata)
                id = row[0]
                longitude = row[1]
                latitude = row[2]
                jdata = {"Id": id, "Longitude": longitude, "Latitude": latitude}
                rejdata.append(jdata)
                print(rejdata)
            print("查詢完成")
            status = "OK"
        except:
            db.rollback()
            print("查詢失敗")
            status = "ERROR"
        db.close()
        return HttpResponse(json.dumps(rejdata), content_type="application/json")

@csrf_exempt
def upload_sensor_data(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            temperature = data['temperature']
            humidity = data['humidity']
            sensor_id = data['id']

            db = pymysql.connect(host='localhost', user='FCU1326', password='vincent20050507', db='test')
            cursor = db.cursor()

            # 如果 id 存在，就更新；否則就新增
            sql = """
                INSERT INTO temp1 (id, Temperature, Humidity)
                VALUES (%s, %s, %s)
                ON DUPLICATE KEY UPDATE
                    Temperature = VALUES(Temperature),
                    Humidity = VALUES(Humidity)
            """
            cursor.execute(sql, (sensor_id, temperature, humidity))

            db.commit()
            db.close()
            return JsonResponse({'status': 'success'})
        except Exception as e:
            print("Error:", e)
            return JsonResponse({'status': 'error', 'message': str(e)})

@csrf_exempt
def get_temp_data(request):
    if request.method == 'GET':
        try:
            db = pymysql.connect(host='localhost', user='FCU1326', password='vincent20050507', db='test')
            cursor = db.cursor()
            cursor.execute("SELECT * FROM temp1 ORDER BY id ASC")
            result = cursor.fetchall()
            db.close()

            data = []
            for row in result:
                data.append({
                    "id": row[0],
                    "Temperature": row[1],
                    "Humidity": row[2]
                })

            return JsonResponse(data, safe=False)
        except Exception as e:
            return JsonResponse({'status': 'error', 'message': str(e)})

@csrf_exempt
def upload_sensor_to_new_db(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            temperature = data['temperature']
            humidity = data['humidity']

            # 連線到 sensor_db
            db = pymysql.connect(host='localhost', user='FCU1326', password='vincent20050507', db='sensor_db')
            cursor = db.cursor()

            sql = "INSERT INTO sensor_data (temperature, humidity) VALUES (%s, %s)"
            cursor.execute(sql, (temperature, humidity))

            db.commit()
            db.close()
            return JsonResponse({'status': 'success'})
        except Exception as e:
            print("Error:", e)
            return JsonResponse({'status': 'error', 'message': str(e)})

@csrf_exempt
def G005(request):
    if request.method == 'POST':
        filename = request.GET.get('filename', 'latest.png')
        save_dir = r"C:\Users\USER\Desktop\作業\嵌入式系統\code\0424\API(PYTHON)\fcu\ESP32_CAM"
        os.makedirs(save_dir, exist_ok=True)
        save_path = os.path.join(save_dir, filename)
        try:
            with open(save_path, "wb") as fh:
                fh.write(request.body)
            status = "OK"
        except Exception as e:
            status = "ERROR"
            print("Error:", e)
        return HttpResponse(status)

@csrf_exempt
def G004(request):
    if request.method == 'GET':
        with open(r"C:\Users\USER\Desktop\作業\嵌入式系統\code\0424\API(PYTHON)\fcu\ESP32_CAM\latest.png", "rb") as artifact:
            rejdata = artifact.read()
            artifact.close()
            print(type(rejdata))
        return HttpResponse(rejdata, content_type="image/png")

@csrf_exempt
def upload_rfid_checkin(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            rfid = data.get('rfid', '')

            db = pymysql.connect(host='localhost', user='FCU1326', password='vincent20050507', db='project')
            cursor = db.cursor()

            # 查找該 RFID 是否為公司員工
            sql_select = "SELECT id FROM users WHERE rfid_tag = %s"
            cursor.execute(sql_select, (rfid,))
            user = cursor.fetchone()

            if not user:
                return JsonResponse({'status': 'error', 'message': 'Unknown RFID'})  # 非公司員工

            user_id = user[0]

            # 寫入打卡紀錄
            sql_insert = "INSERT INTO attendance_log (user_id) VALUES (%s)"
            cursor.execute(sql_insert, (user_id,))
            db.commit()
            db.close()

            return JsonResponse({'status': 'success', 'message': 'Check-in recorded'})
        except Exception as e:
            print("RFID error:", e)
            return JsonResponse({'status': 'error', 'message': str(e)})

@csrf_exempt
def upload_environment_data(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            temperature = data['temperature']
            humidity = data['humidity']

            db = pymysql.connect(host='localhost', user='FCU1326', password='vincent20050507', db='project')
            cursor = db.cursor()

            sql = "INSERT INTO environment_log (temperature, humidity) VALUES (%s, %s)"
            cursor.execute(sql, (temperature, humidity))
            db.commit()
            db.close()

            # 這邊要回傳 JSON，不能用 HttpResponse
            return JsonResponse({'status': 'success', 'message': 'Environment data saved'})
        except Exception as e:
            print("ENV error:", e)
            return JsonResponse({'status': 'error', 'message': str(e)})
