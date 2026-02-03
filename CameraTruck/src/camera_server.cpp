
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

void handle_jpg_stream();
void handle_jpg();

void startCameraServer() {
  // Page d'accueil simple
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html",
      "<html><body><h1>ESP32-S3 Cam</h1>"
      "<p><a href='/stream'>Voir le stream</a></p>"
      "<img src='/stream' />"
      "</body></html>");
  });

  // Une image unique
  server.on("/jpg", HTTP_GET, handle_jpg);

  // Stream MJPEG
  server.on("/stream", HTTP_GET, handle_jpg_stream);

  server.begin();
  Serial.println("Serveur HTTP démarré sur port 80");
}

void cameraServerLoop() {
  server.handleClient();
}

void handle_jpg() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    server.send(503, "text/plain", "Camera capture failed");
    return;
  }

  server.setContentLength(fb->len);
  server.send(200, "image/jpeg", "");

  WiFiClient client = server.client();
  client.write(fb->buf, fb->len);

  esp_camera_fb_return(fb);
}

void handle_jpg_stream() {
  WiFiClient client = server.client();
  String boundary = "frame";
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=" + boundary + "\r\n\r\n"
  );

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) break;

    client.printf("--%s\r\n", boundary.c_str());
    client.print("Content-Type: image/jpeg\r\n");
    client.printf("Content-Length: %u\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    esp_camera_fb_return(fb);
    delay(10);
  }
}