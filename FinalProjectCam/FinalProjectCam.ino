#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include <HTTPClient.h>
#include <time.h>
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// WiFi 設定
const char* ssid = "iphone15";
const char* password = "kanameshuka";

// Django 伺服器 API (G005)
const char* django_url_base = "http://172.20.10.3:8000/G005/?filename=";

// 時區
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;

WebServer server(80);

void handle_capture() {
  String name = "";
  String status = "ERROR";
  if (server.hasArg("status")) status = server.arg("status");
  if (server.hasArg("name")) name = server.arg("name");

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera Error");
    return;
  }

  // 時間字串
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    server.send(500, "text/plain", "Time Error");
    esp_camera_fb_return(fb);
    return;
  }
  char datetime[32];
  strftime(datetime, sizeof(datetime), "%Y%m%d_%H%M%S", &timeinfo);

  // 組合檔名
  String filename;
  if (status == "SUCCESS" && name.length() > 0) {
    filename = String(datetime) + "_" + name + "_SUCCESS.jpg";
  } else {
    filename = String(datetime) + "_ERROR.jpg";
  }

  // 上傳到 Django API
  String url = String(django_url_base) + filename;
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/octet-stream");
  int code = http.POST(fb->buf, fb->len);
  String resp = http.getString();
  http.end();

  esp_camera_fb_return(fb);

  Serial.println("照片已上傳，檔名：" + filename);
  server.send(200, "text/plain", "Done");
}

void setup() {
  Serial.begin(115200);

  // WiFi連線
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("WiFi connected: " + WiFi.localIP().toString());

  // Camera初始化（依照你的板子調整腳位！）
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  // 時間同步
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // 啟動 HTTP Server
  server.on("/capture", handle_capture);
  server.begin();

  Serial.println("ESP32-CAM Ready!");
  Serial.print("Stream: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");
}

void loop() {
  server.handleClient();
}