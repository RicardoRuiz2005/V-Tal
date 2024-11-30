#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Wire.h>
#include "MAX30105.h"

// Credenciales de WiFi
const char* ssid = "TuSSID";
const char* password = "TuPassword";

// Configuración de Ubidots
const char* ubidots_token = "TuTokenUbidots";
const char* ubidots_url = "http://industrial.api.ubidots.com/api/v1.6/devices/esp32";

// Configuración del DHT11
#define DHTPIN 15 // Pin al que está conectado el DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configuración del MAX30102
MAX30105 particleSensor;

// Configuración de la pantalla OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configuración de NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Actualización cada minuto

// Variables globales
float temperatura = 0.0;
float humedad = 0.0;
int pulso = 0;
int oxigenacion = 0;
int pulsoData[60]; // Almacena los valores del pulso del último minuto
int pulsoIndex = 0;

void setup() {
  Serial.begin(115200);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi!");

  // Iniciar NTPClient
  timeClient.begin();

  // Iniciar DHT11
  dht.begin();

  // Iniciar MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 no detectado. Verifica la conexión.");
    while (1);
  }
  particleSensor.setup(); // Configuración predeterminada del sensor

  // Iniciar pantalla OLED
  if (!display.begin(SSD1306_I2C_ADDRESS, 0x3C)) {
    Serial.println("Error iniciando la pantalla OLED.");
    while (1);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  // Actualizar hora
  timeClient.update();
  String currentTime = timeClient.getFormattedTime();

  // Leer DHT11
  temperatura = dht.readTemperature();
  humedad = dht.readHumidity();
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error leyendo el DHT11");
    temperatura = 0.0;
    humedad = 0.0;
  }

  // Leer MAX30102
  pulso = particleSensor.getIR(); // Suponiendo un umbral
  oxigenacion = random(90, 100); // Placeholder para simulación

  // Guardar el pulso en el arreglo y graficar cada minuto
  pulsoData[pulsoIndex] = pulso;
  pulsoIndex = (pulsoIndex + 1) % 60; // Rotar índice

  // Mostrar datos en OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.printf("Hora: %s\nTemp: %.1fC\nHumedad: %.1f%%\nPulso: %d\nO2: %d%%", 
                 currentTime.c_str(), temperatura, humedad, pulso, oxigenacion);

  // Graficar pulso
  for (int i = 0; i < SCREEN_WIDTH && i < pulsoIndex; i++) {
    int y = map(pulsoData[i], 0, 1023, SCREEN_HEIGHT - 1, 0); // Escalar valores
    display.drawPixel(i, y, SSD1306_WHITE);
  }
  display.display();

  // Enviar datos a Ubidots
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(ubidots_url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Auth-Token", ubidots_token);

    String payload = String("{\"temperatura\":") + temperatura +
                     ",\"humedad\":" + humedad +
                     ",\"oxigenacion\":" + oxigenacion +
                     ",\"pulso\":" + pulso + "}";
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.printf("Datos enviados a Ubidots: %s\n", payload.c_str());
    } else {
      Serial.printf("Error enviando datos: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi desconectado. No se pudo enviar datos.");
  }

  delay(1000); // Esperar un segundo antes de la próxima iteración
}
