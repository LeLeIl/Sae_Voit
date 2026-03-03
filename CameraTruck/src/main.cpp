#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
void startCameraServer();
void cameraServerLoop();
//////////////////////////////////////////////////
// CONFIG CAMERA (NE PAS TOUCHER)
//////////////////////////////////////////////////
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
//////////////////////////////////////////////////
// MOTEUR
//////////////////////////////////////////////////
#define AIN1 4
#define AIN2 5
#define BIN1 6
#define BIN2 7
//////////////////////////////////////////////////
// WIFI
//////////////////////////////////////////////////
const char* ap_ssid     = "CamTruck";
const char* ap_password = "12345678";
WiFiServer controlServer(81);   // PORT 81 POUR LE MOTEUR
//////////////////////////////////////////////////
// FONCTIONS MOTEUR
//////////////////////////////////////////////////
void forward() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
}
void backward() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
}
void left() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
}
void right() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
}
void stopCar() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}
//////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  delay(2000);
  // PINS MOTEUR
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  //////////////////////////////////////////////////
  // INIT CAMERA
  //////////////////////////////////////////////////
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
    Serial.println("Camera init failed");
    return;
  }
  Serial.println("Camera OK");
  //////////////////////////////////////////////////
  // WIFI AP
  //////////////////////////////////////////////////
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("IP: http://");
  Serial.println(ip);
  //////////////////////////////////////////////////
  // SERVEURS
  //////////////////////////////////////////////////
  controlServer.begin();   // IMPORTANT POUR PORT 81
  startCameraServer();     // PORT 80
  Serial.println("Serveur prêt !");
}
//////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////
void loop() {
  // -------- CONTROLE MOTEUR --------
 WiFiClient client = controlServer.available();
if (client) {
    Serial.println("conexion");
    while (client.connected()) {
        if (client.available()) {
            char c = client.read();
            Serial.print("Commande: ");
            Serial.println(c);
            if (c == 'F') forward();
            else if (c == 'B') backward();
            else if (c == 'L') left();
            else if (c == 'R') right();
            else if (c == 'S') stopCar();
        }
        delay(5);
    }
    Serial.println("deconexion");
    client.stop();
}
  // -------- CAMERA --------
  cameraServerLoop();
}
