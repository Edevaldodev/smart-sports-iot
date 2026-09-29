#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ArduinoJson.h>

// ===============================
// CONFIGURAÇÕES
// ===============================

const char* WIFI_SSID = "NOME_DA_SUA_REDE";
const char* WIFI_PASSWORD = "SENHA_DA_SUA_REDE";

// IP DO COMPUTADOR ONDE O FASTAPI ESTÁ RODANDO
const char* SERVER_HOST = "192.168.1.100";
const int SERVER_PORT = 8000;

const char* ENDPOINT = "/api/telemetria";

const char* ATLETA_ID = "ATLETA_07";

// ===============================
// OBJETOS
// ===============================

Adafruit_MPU6050 mpu;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, SERVER_HOST, SERVER_PORT);

// ===============================
// SETUP
// ===============================

void setup() {

  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" SMART SPORTS IoT");
  Serial.println(" UNO R4 WiFi");
  Serial.println("==============================");

  // -------------------------------
  // MPU6050
  // -------------------------------

  Wire.begin();

  Serial.println("Inicializando MPU6050...");

  if (!mpu.begin()) {

    Serial.println("ERRO: MPU6050 nao encontrado!");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("MPU6050 encontrado!");

  // Configuração do acelerômetro
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // Configuração do giroscópio
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // Filtro
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // -------------------------------
  // WIFI
  // -------------------------------

  Serial.println();
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);

  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) {

    Serial.print(".");

    delay(3000);
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");

  Serial.print("IP do Arduino: ");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.println("Sistema iniciado!");
}

// ===============================
// LOOP
// ===============================

void loop() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi desconectado. Tentando reconectar...");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    delay(5000);

    return;
  }

  // -------------------------------
  // LER MPU6050
  // -------------------------------

  sensors_event_t aceleracao;
  sensors_event_t giro;
  sensors_event_t temperatura;

  mpu.getEvent(
    &aceleracao,
    &giro,
    &temperatura
  );

  // -------------------------------
  // MOSTRAR DADOS NO SERIAL
  // -------------------------------

  Serial.println();
  Serial.println("----- MPU6050 -----");

  Serial.print("Acel X: ");
  Serial.print(aceleracao.acceleration.x);
  Serial.println(" m/s2");

  Serial.print("Acel Y: ");
  Serial.print(aceleracao.acceleration.y);
  Serial.println(" m/s2");

  Serial.print("Acel Z: ");
  Serial.print(aceleracao.acceleration.z);
  Serial.println(" m/s2");

  Serial.print("Giro X: ");
  Serial.print(giro.gyro.x);
  Serial.println(" rad/s");

  Serial.print("Giro Y: ");
  Serial.print(giro.gyro.y);
  Serial.println(" rad/s");

  Serial.print("Giro Z: ");
  Serial.print(giro.gyro.z);
  Serial.println(" rad/s");

  // -------------------------------
  // CRIAR JSON
  // -------------------------------

  JsonDocument doc;

  doc["atleta_id"] = ATLETA_ID;

  doc["acel_x"] = aceleracao.acceleration.x;
  doc["acel_y"] = aceleracao.acceleration.y;
  doc["acel_z"] = aceleracao.acceleration.z;

  doc["giro_x"] = giro.gyro.x;
  doc["giro_y"] = giro.gyro.y;
  doc["giro_z"] = giro.gyro.z;

  String json;

  serializeJson(doc, json);

  Serial.println();
  Serial.println("JSON enviado:");
  Serial.println(json);

  // -------------------------------
  // ENVIAR PARA FASTAPI
  // -------------------------------

  Serial.println();
  Serial.println("Enviando para FastAPI...");

  client.beginRequest();

  client.post(ENDPOINT);

  client.sendHeader(
    "Content-Type",
    "application/json"
  );

  client.sendHeader(
    "Content-Length",
    json.length()
  );

  client.beginBody();

  client.print(json);

  client.endRequest();

  // -------------------------------
  // RESPOSTA DA API
  // -------------------------------

  int statusCode = client.responseStatusCode();

  String response = client.responseBody();

  Serial.print("HTTP Status: ");
  Serial.println(statusCode);

  Serial.print("Resposta: ");
  Serial.println(response);

  if (statusCode == 200) {

    Serial.println(">>> DADO ENVIADO COM SUCESSO!");

  } else {

    Serial.println(">>> ERRO AO ENVIAR DADO!");

  }

  // 5 leituras por segundo
  delay(200);
}
