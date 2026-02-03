#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"

void startCameraServer();
void cameraServerLoop();

// ==== CONFIG CAMERA XIAO ESP32S3 SENSE ====
// (pins officiels OV2640)
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39

#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15

#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

// WiFi AP (créé par le XIAO)
const char* ap_ssid     = "CamTruck";
const char* ap_password = "12345678";

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\nBoot ESP32-S3 Cam...");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

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
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count     = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.print("Camera init failed: ");
    Serial.println(err);
    return;
  }

  Serial.println("Camera OK");

  // ==== MODE POINT D'ACCÈS ====
  WiFi.mode(WIFI_AP);
  bool ap_ok = WiFi.softAP(ap_ssid, ap_password);
  if (!ap_ok) {
    Serial.println("Erreur AP");
    return;
  }

  IPAddress ip = WiFi.softAPIP();
  Serial.println("AP démarre");
  Serial.print("SSID : ");
  Serial.println(ap_ssid);
  Serial.print("MDP  : ");
  Serial.println(ap_password);
  Serial.print("IP   : http://");
  Serial.println(ip);

  // Lancer le serveur caméra
  startCameraServer();
  Serial.println("Serveur caméra prêt. Ouvre /stream dans ton navigateur");
}

void loop() {
  // On laisse le serveur web traiter les requêtes
  cameraServerLoop();
  delay(10);
}
