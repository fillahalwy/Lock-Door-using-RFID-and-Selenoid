// This code made by group 5:
// Ahmad Fillah Alwy, 
// M Ridho Ramadham, 
// Akmal Harizulhaq,
// Ardhis Parahita,
// Nathan Farroz Kusuma Barracuda
// Sepvilia Fahturahma

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// RFID Pins
#define SS_PIN D4
#define RST_PIN D3
#define LED_G D1 // Pin untuk LED hijau
#define LED_R D2 // Pin untuk LED merah
#define RELAY D8 // Pin untuk relay
#define ACCESS_DELAY 1000 // Waktu delay untuk akses
#define DENIED_DELAY 500 // Waktu delay untuk akses ditolak

// Wi-Fi Credentials
const char* ssid = "iPhone";          // Ganti dengan SSID Wi-Fi Anda
const char* password = "123456789";  // Ganti dengan password Wi-Fi Anda

// Telegram Bot Credentials
const char* botToken = "7901596402:AAGXm2dr199eZEavsRrU08i07ecEyyKbwAc"; // Ganti dengan token bot Telegram Anda
const char* chatID = "6837994169";     // Ganti dengan Chat ID Anda

MFRC522 mfrc522(SS_PIN, RST_PIN); // Inisialisasi MFRC522
WiFiClientSecure client;          // Koneksi HTTPS

// Daftar UID dan nama kartu
struct Card {
  String uid;
  String name;
};

Card cardList[] = {
  {"04242E2AEF6A80", " Akmal"},
  {"04315C82667780", " Alwy"},
  // Tambahkan kartu lainnya di sini
};

// Fungsi untuk mendapatkan nama kartu dari UID
String getCardName(String uid) {
  for (Card card : cardList) {
    Serial.println("Memeriksa kartu: " + card.uid);
    if (card.uid == uid) {
      Serial.println("Kartu ditemukan: " + card.name);
      return card.name; // Jika UID ditemukan, kembalikan nama kartu
    }
  }
  Serial.println("Kartu tidak ditemukan!");
  return "Kartu Tidak Dikenal"; // Jika UID tidak ditemukan
}

void setup() {
  Serial.begin(115200); // Memulai komunikasi serial
  SPI.begin(); // Memulai komunikasi SPI
  mfrc522.PCD_Init(); // Inisialisasi modul RFID

  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(RELAY, OUTPUT);

  digitalWrite(RELAY, LOW); // Memastikan relay mati saat awal
  digitalWrite(LED_G, LOW); // Memastikan LED hijau mati saat awal
  digitalWrite(LED_R, LOW); // Memastikan LED merah mati saat awal

  Serial.println("Menghubungkan ke Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi terhubung!");
  Serial.println("Alamat IP: " + WiFi.localIP().toString());
  Serial.println("Tempatkan kartu Anda di pembaca...");
}

void loop() {
  // Memeriksa apakah ada kartu baru yang terdeteksi
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Memilih salah satu kartu
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Membaca UID kartu
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uid += "0";  // Tambahkan '0' tanpa spasi
    }
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase(); // Ubah menjadi huruf kapital
  Serial.println("UID yang dibaca: " + uid);

  // Mendapatkan nama kartu
  String cardName = getCardName(uid);

  if (cardName != "Kartu Tidak Dikenal") {
    Serial.println("Akses diizinkan untuk: " + cardName);
    grantAccess();
    sendToTelegram("Akses diizinkan untuk: " + cardName);
  } else {
    Serial.println("Akses ditolak untuk UID: " + uid);
    denyAccess();
    sendToTelegram("Akses ditolak untuk UID: " + uid);
  }

  // Reset state RFID untuk membaca kartu berikutnya
  mfrc522.PICC_HaltA();      // Menghentikan komunikasi dengan kartu
  mfrc522.PCD_StopCrypto1(); // Mengakhiri sesi otentikasi
  resetRFID();               // Reset modul RFID
}

void grantAccess() {
  digitalWrite(RELAY, HIGH); // Mengaktifkan relay
  digitalWrite(LED_G, HIGH); // Menyalakan LED hijau
  delay(ACCESS_DELAY); // Menunggu sesuai waktu akses
  digitalWrite(RELAY, LOW); // Mematikan relay
  digitalWrite(LED_G, LOW); // Mematikan LED hijau
}

void denyAccess() {
  digitalWrite(LED_R, HIGH); // Menyalakan LED merah
  delay(DENIED_DELAY); // Menunggu sesuai waktu penolakan
  digitalWrite(LED_R, LOW); // Mematikan LED merah
}

// Fungsi untuk mengirim log ke Telegram
void sendToTelegram(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    String url = String("https://api.telegram.org/bot") + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + message;
    client.setInsecure(); // Mengabaikan sertifikat
    if (client.connect("api.telegram.org", 443)) {
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: api.telegram.org\r\n" +
                   "Connection: close\r\n\r\n");
      Serial.println("Log dikirim ke Telegram: " + message);
    } else {
      Serial.println("Gagal terhubung ke Telegram");
    }
  } else {
    Serial.println("Wi-Fi tidak terhubung");
  }
}

void resetRFID() {
  mfrc522.PCD_Reset(); // Reset modul RFID
  mfrc522.PCD_Init();  // Reinisialisasi modul RFID
}
