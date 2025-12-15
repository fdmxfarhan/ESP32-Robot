#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// =====================
// WiFi
// =====================
const char* ssid = "Farhan-STC";
const char* password = "ZXasQW@12";

// =====================
// Server URLs
// =====================
const char* uploadUrl = "http://45.90.72.56:3008/upload";
const char* commandUrl = "http://45.90.72.56:3008/command";

// =====================
// Motor Pins
// =====================
#define LEFT_IN1 12
#define LEFT_IN2 13
#define LEFT_PWM 14

#define RIGHT_IN1 15
#define RIGHT_IN2 2
#define RIGHT_PWM 4

// =====================
// AI Thinker Camera Pins
// =====================
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define FLASH_LED_PIN 4

// =====================
void setupMotors() {
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);
}

// =====================
void driveMotor(int in1, int in2, int channel, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

// =====================
void controlMotors(int x, int y) {
  int left = y + x;
  int right = y - x;

  left = constrain(left, -100, 100);
  right = constrain(right, -100, 100);

  int pwmL = map(abs(left), 0, 100, 0, 255);
  int pwmR = map(abs(right), 0, 100, 0, 255);

  driveMotor(LEFT_IN1, LEFT_IN2, 0, left >= 0 ? pwmL : -pwmL);
  driveMotor(RIGHT_IN1, RIGHT_IN2, 1, right >= 0 ? pwmR : -pwmR);
}

// =====================
void getJoystick(int& x, int& y) {
  HTTPClient http;
  http.begin(commandUrl);

  if (http.GET() == 200) {
    String payload = http.getString();
    StaticJsonDocument<64> doc;
    deserializeJson(doc, payload);
    x = doc["x"] | 0;
    y = doc["y"] | 0;
    Serial.print("X: ");
    Serial.print(x);
    Serial.print("\tY: ");
    Serial.println(y);
  }
  http.end();
}

// =====================
void setup() {
  Serial.begin(115200);
  setupMotors();
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, HIGH);  // OFF by default
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_camera_init(&config);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// =====================
void loop() {
  // ---- CAMERA UPLOAD ----
  camera_fb_t* fb = esp_camera_fb_get();
  if (fb) {
    HTTPClient http;
    http.begin(uploadUrl);
    http.addHeader("Content-Type", "image/jpeg");
    http.POST(fb->buf, fb->len);
    http.end();
    esp_camera_fb_return(fb);
  }

  // ---- JOYSTICK CONTROL ----
  int x = 0, y = 0;
  getJoystick(x, y);
  controlMotors(x, y);

  delay(80);  // sync with FPS
}
