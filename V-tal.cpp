#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

// Configuración de la pantalla OLED
#define ANCHO 128
#define ALTO 64
#define OLED_RESET -1
Adafruit_SSD1306 oled(ANCHO, ALTO, &Wire, OLED_RESET);

// Configuración de Wi-Fi
const char* ssid = "RED";         // Reemplaza con el nombre de tu red Wi-Fi
const char* password = "CONTRASEÑA"; // Reemplaza con la contraseña de tu red Wi-Fi

// Configuración de NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -21600; // UTC-6 para México Central (ajusta según tu zona horaria)
const int daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(23, 19); // Configurar SDA = 23, SCL = 19 para la pantalla OLED

  // Inicializar pantalla OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error: No se pudo inicializar la pantalla OLED.");
    while (true);
  }
  oled.clearDisplay();

  // Conectar a Wi-Fi
  Serial.print("Conectando a Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConexión Wi-Fi establecida.");

  // Configurar hora mediante NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sincronizando hora...");
}

void loop() {
  // Obtener la hora actual
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error al obtener la hora.");
    return;
  }

  // Limpiar pantalla
  oled.clearDisplay();

  // Mostrar "Hora actual:"
  oled.setCursor(0, 0);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.print("Hora actual:");

  // Mostrar la hora en formato HH:MM:SS
  oled.setCursor(10, 30);
  oled.setTextSize(2);
  if (timeinfo.tm_hour < 10) oled.print('0'); // Añadir cero si es menor a 10
  oled.print(timeinfo.tm_hour);
  oled.print(':');
  if (timeinfo.tm_min < 10) oled.print('0'); // Añadir cero si es menor a 10
  oled.print(timeinfo.tm_min);
  oled.print(':');
  if (timeinfo.tm_sec < 10) oled.print('0'); // Añadir cero si es menor a 10
  oled.print(timeinfo.tm_sec);

  // Actualizar pantalla
  oled.display();

  delay(1000); // Actualizar cada segundo
}
