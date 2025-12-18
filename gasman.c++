#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define GAS_PIN 34
#define LED_PIN 18
#define BUZZER_PIN 25

const char* ssid = "Joni";
const char* password = "12345678";

const char* token = "8263182263:AAHgLRqYDMti12IrEkLnyB7Q5IG8KhhC7z8";
const char* chat_id = "1796628201";

// Ambang batas tambahan di atas udara normal
int deltaThreshold = 300;

bool kirimAman = true;
unsigned long terakhirKirim = 0;
const unsigned long interval = 8000;

WiFiClientSecure client;
UniversalTelegramBot bot(token, client);

int baseline = 0;

void setup() {
  Serial.begin(115200);

  pinMode(GAS_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setInsecure();

  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi terhubung");

  bot.sendMessage(chat_id, "✅ ESP32 Sensor Gas Aktif");

  // Warm-up MQ2
  Serial.println("🔥 Pemanasan sensor MQ-2...");
  delay(20000);

  // Kalibrasi baseline
  Serial.println("⚙️ Kalibrasi udara normal...");
  long total = 0;
  for (int i = 0; i < 60; i++) {
    total += analogRead(GAS_PIN);
    delay(100);
  }

  baseline = total / 60;

  Serial.print("✅ Baseline udara normal: ");
  Serial.println(baseline);
}

void loop() {
  const int sampleCount = 20;
  long total = 0;

  for (int i = 0; i < sampleCount; i++) {
    total += analogRead(GAS_PIN);
    delay(5);
  }

  int adc = total / sampleCount;
  int nilaiGas = adc - baseline;
  unsigned long sekarang = millis();

  Serial.print("ADC: ");
  Serial.print(adc);
  Serial.print(" | Selisih: ");
  Serial.println(nilaiGas);

  if (nilaiGas > deltaThreshold) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    if (sekarang - terakhirKirim > interval) {
      String pesan = "🚨 GAS TERDETEKSI!\nADC: " + String(adc) +
                     "\nSelisih: " + String(nilaiGas);
      bot.sendMessage(chat_id, pesan);
      terakhirKirim = sekarang;
      kirimAman = false;
    }
  } 
  else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    if (!kirimAman) {
      bot.sendMessage(chat_id, "✅ Gas kembali normal");
      kirimAman = true;
    }
  }

  delay(500);
}
