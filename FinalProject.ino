#include <pitches.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <deprecated.h>
#include <require_cpp11.h>
#include <SPI.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LedControlMS.h>

// --- 硬體腳位設定 ---
#define DHTPIN 27
#define DHTTYPE DHT11
#define SS_PIN 5
#define RST_PIN 22
#define BUZZER_PIN 13   // 蜂鳴器
#define LED_BUILTIN 2   // 內建LED

// MAX7219 8x8 LED矩陣（DIN, CLK, CS, 模組數量）
LedControl lc = LedControl(16, 4, 17, 1);

DHT dht(DHTPIN, DHTTYPE);
MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "iphone15";
const char* password = "kanameshuka";
const char* api_env = "http://172.20.10.3:8000/upload_env/";
const char* api_rfid = "http://172.20.10.3:8000/upload_rfid/";

unsigned long previousEnvMillis = 0;
const long envInterval = 30000;

// ---- 你自己的RFID查表 ----
String getNameFromTag(String tag) {
  if (tag == "1D958B79") return "王小明";
  if (tag == "85AF851A") return "陳小華";
  // 其他RFID依實際新增
  return "";
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  dht.begin();
  SPI.begin();
  rfid.PCD_Init();

  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  WiFi.begin(ssid, password);
  Serial.print("連線中");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi 連線成功！");
}

void loop() {
  // --- 每 30 秒上傳一次溫濕度 ---
  if (millis() - previousEnvMillis >= envInterval) {
    previousEnvMillis = millis();
    uploadTemperatureHumidity();
  }

  // --- RFID 檢查 ---
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String tagID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      tagID += String(rfid.uid.uidByte[i], HEX);
    }
    tagID.toUpperCase();
    String name = getNameFromTag(tagID);
    Serial.print("讀取卡片："); Serial.println(tagID);
    uploadRFID(tagID, name);  // <-- 多一個name參數
    delay(1500); // 防止重複讀取同一張卡
  }
}

// --- 上傳 RFID 卡號 ---
void uploadRFID(String rfid, String name) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(api_rfid);
    http.addHeader("Content-Type", "application/json");
    String jsonData = "{\"rfid\": \"" + rfid + "\"}";
    int responseCode = http.POST(jsonData);
    String res = http.getString();
    Serial.println("伺服器回傳: " + res);

    bool success = res.indexOf("\"status\": \"success\"") != -1;

    if (responseCode > 0) {
      Serial.println("刷卡上傳成功：" + res);
      showResult(success, name);
    } else {
      Serial.println("刷卡上傳失敗");
      showResult(false, name);
    }
    http.end();
  } else {
    showResult(false, name);
  }
}

// --- 溫濕度上傳 ---
void uploadTemperatureHumidity() {
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();

  if (isnan(temp) || isnan(humi)) {
    Serial.println("溫濕度讀取失敗");
    return;
  }

  Serial.printf("溫度: %.2f °C, 濕度: %.2f %%\n", temp, humi);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(api_env);
    http.addHeader("Content-Type", "application/json");
    String jsonData = "{\"temperature\": " + String(temp, 2) + ", \"humidity\": " + String(humi, 2) + "}";
    int responseCode = http.POST(jsonData);
    if (responseCode > 0) {
      Serial.println("環境上傳成功：" + http.getString());
    } else {
      Serial.println("環境上傳失敗");
    }
    http.end();
  }
}

// --- showResult 帶入姓名參數 ---
void showResult(bool success, String name) {
  // 蜂鳴器音效
  if (success) {
    tone(BUZZER_PIN, NOTE_C6, 120); delay(130);
    tone(BUZZER_PIN, NOTE_E6, 120); delay(130);
    noTone(BUZZER_PIN);
  } else {
    for (int i = 0; i < 2; i++) {
      tone(BUZZER_PIN, NOTE_DS4, 200); delay(220);
      noTone(BUZZER_PIN); delay(100);
    }
  }
  checkPeopleAndSetLed();
  notifyCamToCapture(name, success);   // <-- 呼叫時帶姓名參數

  lc.clearAll();
  const char* msg = success ? "SUCCESS" : "ERROR";
  for (int i = 0; msg[i] != '\0'; i++) {
    scrollLeft(msg[i]);      // 一個字母一個字母捲動進來
    delay(200);
  }
  lc.clearAll();
}

void scrollLeft(char ch) {
  int pos = lc.getCharArrayPosition(ch);
  for (int scroll = 0; scroll < 6; scroll++) {
    for (int i = scroll; i < 6; i++) {
      lc.setRow(0, i - scroll, alphabetBitmap[pos][i]);
    }
    delay(300);
  }
  lc.clearDisplay(0);
}

void checkPeopleAndSetLed() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://172.20.10.3/get_company_count1326.php");  // 改成你的 API
    int httpCode = http.GET();
    Serial.print("httpCode: "); Serial.println(httpCode);
    if (httpCode == 200) {
      String payload = http.getString();
      Serial.println("公司人數API回傳: " + payload);
      int idx = payload.indexOf("in_office_count");
      int count = 0;
      if (idx != -1) {
        int start = payload.indexOf(":", idx);
        int end = payload.indexOf("}", start);
        if (start != -1 && end != -1) {
          String number = payload.substring(start + 1, end);
          number.trim();
          count = number.toInt();
        }
      }
      Serial.print("目前公司人數: "); Serial.println(count);
      digitalWrite(LED_BUILTIN, count > 0 ? HIGH : LOW);
    }
    http.end();
  }
}

// --- 通知CAM拍照 ---
void notifyCamToCapture(String employee, bool success) {
  String status = success ? "SUCCESS" : "ERROR";
  String camIP = "172.20.10.2"; // <<< 改成你的 ESP32-CAM IP！
  String url;
  if (success && employee.length() > 0) {
    url = "http://" + camIP + "/capture?name=" + employee + "&status=" + status;
  } else {
    url = "http://" + camIP + "/capture?status=" + status;
  }
  HTTPClient http;
  http.begin(url);
  int code = http.GET();
  Serial.print("ESP32-CAM回應: ");
  Serial.println(http.getString());
  http.end();
}
