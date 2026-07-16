#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>

const char* ssid = "Kontrakan Teknik";
const char* password = "sabang-merauke23";

unsigned long channelID = 3428498;
const char *writeAPI = "VC66C8ULAO8ORWEM";

#define DHTPIN D1
#define DHTTYPE DHT11

#define TRIG D5
#define ECHO D6

#define LED_HIJAU D2
#define LED_MERAH D3

WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);

void connectWiFi()
{
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("==================================");
  Serial.println("WiFi Berhasil Terhubung");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());
  Serial.println("==================================");
}

float bacaJarak()
{
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);

  if (duration == 0)
  {
    return -1;
  }

  float distance = duration * 0.0343 / 2.0;

  return distance;
}

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);

  digitalWrite(LED_HIJAU, LOW);
  digitalWrite(LED_MERAH, LOW);

  dht.begin();

  connectWiFi();

  ThingSpeak.begin(client);

  Serial.println();
  Serial.println("===== SMART AQUARIUM IoT =====");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }

  float distance = bacaJarak();
  float suhu = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(suhu) || isnan(humidity))
  {
    Serial.println("Gagal membaca DHT11");
    delay(2000);
    return;
  }

  String statusAkuarium = "Normal";
  int status = 0;

  if (distance > 10 && suhu > 30)
  {
    statusAkuarium = "Air Rendah & Suhu Tinggi";
    status = 3;

    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_MERAH, HIGH);
  }
  else if (distance > 10)
  {
    statusAkuarium = "Air Rendah";
    status = 1;

    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_MERAH, HIGH);
  }
  else if (suhu > 30)
  {
    statusAkuarium = "Suhu Tinggi";
    status = 2;

    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_MERAH, HIGH);
  }
  else
  {
    statusAkuarium = "Normal";
    status = 0;

    digitalWrite(LED_HIJAU, HIGH);
    digitalWrite(LED_MERAH, LOW);
  }

  Serial.println("====================================");

  Serial.print("Jarak Air     : ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Suhu          : ");
  Serial.print(suhu);
  Serial.println(" °C");

  Serial.print("Kelembapan    : ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Status        : ");
  Serial.println(statusAkuarium);

  Serial.print("Kode Status   : ");
  Serial.println(status);

  ThingSpeak.setField(1, distance);
  ThingSpeak.setField(2, suhu);
  ThingSpeak.setField(3, humidity);
  ThingSpeak.setField(4, status);

  ThingSpeak.setStatus(statusAkuarium);

  int response = ThingSpeak.writeFields(channelID, writeAPI);

  if (response == 200)
  {
    Serial.println("Upload ThingSpeak BERHASIL");
  }
  else
  {
    Serial.print("Upload GAGAL. Error : ");
    Serial.println(response);
  }

  Serial.println();

  delay(20000);
}
