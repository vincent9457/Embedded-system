#include "esp_http_server.h"

static const char* stream_html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>ESP32-CAM Stream</title>
    <style>
      body { margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; background-color: black; }
      img { width: auto; height: 100vh; }
    </style>
  </head>
  <body>
    <img src="/stream" />
  </body>
</html>
)rawliteral";

esp_err_t stream_handler(httpd_req_t *req) {
    httpd_resp_send(req, stream_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  
  httpd_handle_t server = NULL;
  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t stream_uri = {
      .uri       = "/",
      .method    = HTTP_GET,
      .handler   = stream_handler,
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &stream_uri);

    httpd_uri_t video_uri = {
      .uri       = "/stream",
      .method    = HTTP_GET,
      .handler   = stream_handler,  
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &video_uri);
  }
}
