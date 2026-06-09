/**
 * @file ThermaAuth_Final_v1.ino
 * @brief Sistem Keamanan ThermaAuth v1 terintegrasi Firebase Realtime DB
 * @version 1.0
 * @date 2026
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DHT.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// ==========================================
// 1. PIN CONFIGURATION & CONSTANTS
// ==========================================
#define PIN_LCD_SDA        5
#define PIN_LCD_SCL       16

#define PIN_RFID_SDA      23
#define PIN_RFID_RST      14
#define PIN_RFID_SCK      22
#define PIN_RFID_MISO     19
#define PIN_RFID_MOSI     21

#define PIN_DHT_DATA       4
#define DHT_TYPE       DHT11

#define PIN_BUZZER         2

#define OTP_CYCLE_MS    30000 
#define WINDOW_VIEW_MS  10000 

// ==========================================
// 2. NETWORK & CLOUD CREDENTIALS
// ==========================================
#define WIFI_SSID "Indihouse"       
#define WIFI_PASSWORD "password"    
#define FIREBASE_HOST "thermaauth-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "8tTwXDdTO36sKgvEq5WCcS8jXvGyGcL5ztRJ9dr4"

// ==========================================
// 3. DEVICE INSTANTIATION & VARIABLES
// ==========================================
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(PIN_RFID_SDA, PIN_RFID_RST);
DHT dht(PIN_DHT_DATA, DHT_TYPE);

FirebaseData fbdo;
FirebaseConfig fbConfig;
FirebaseAuth fbAuth;

const String MASTER_CARD_UID = "9170F95D"; 

long currentGlobalOTP = 0;
unsigned long lastOTPRotationMillis = 0;
float currentTemperature = 0.0;
bool isOnline = false;

// ==========================================
// 4. FUNCTION PROTOTYPES
// ==========================================
void initializeSystem();
void connectWiFi();
void updateGlobalOTP();
void readEnvironmentalMetrics();
void sendDataToFirebase(bool isNewOTP);
void syncOTPFastLane();
void displayInterface(const String& line1, const String& line2);
void executeAudioFeedback(unsigned int durationMs, unsigned int count);
String extractCardUID();
void handleAuthorizedAccess();
void handleUnauthorizedAccess(const String& invalidUID);

void setup() {
  Serial.begin(115200);
  delay(500); 
  randomSeed(analogRead(34)); 
  
  initializeSystem();
  connectWiFi();

  fbConfig.host = FIREBASE_HOST;
  fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH; 
  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);
  
  currentGlobalOTP = random(100000, 1000000); 
  lastOTPRotationMillis = millis();

  displayInterface("ThermaAuth v1", "Scan Your Card  ");
}

void loop() {
  isOnline = (WiFi.status() == WL_CONNECTED);

  updateGlobalOTP();
  readEnvironmentalMetrics();

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return; 
  }

  String scannedUID = extractCardUID();
  
  Serial.println("----------------------------------------");
  Serial.print("INFO: Kartu Terdeteksi! UID = ");
  Serial.println(scannedUID);
  Serial.println("----------------------------------------");

  if (scannedUID == MASTER_CARD_UID) {
    handleAuthorizedAccess();
  } else {
    handleUnauthorizedAccess(scannedUID);
  }

  rfid.PICC_HaltA();
}

// ==========================================
// 5. HARDWARE & NETWORK SUBSYSTEM
// ==========================================

/**
 * @brief Prosedur menghubungkan perangkat ke jaringan utama WiFi
 */
void connectWiFi() {
  displayInterface("Connecting WiFi ", "Please Wait...  ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 15) {
    delay(500);
    Serial.print(".");
    attempt++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Terhubung!");
    displayInterface("WiFi Connected! ", "IP: Ready       ");
    executeAudioFeedback(100, 1);
  } else {
    Serial.println("\nWiFi Gagal Terhubung (Mode Offline)");
    displayInterface("WiFi Error!     ", "Running Offline ");
    executeAudioFeedback(300, 2);
  }
  delay(1500); 
}

/**
 * @brief Inisialisasi parameter dasar perangkat keras dan periferal
 */
void initializeSystem() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  Wire.begin(PIN_LCD_SDA, PIN_LCD_SCL);
  lcd.init();
  lcd.backlight();
  displayInterface("ThermaAuth v1", "Circuit v1 OK   ");
  delay(1500);

  SPI.begin(PIN_RFID_SCK, PIN_RFID_MISO, PIN_RFID_MOSI, PIN_RFID_SDA);
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max); 

  dht.begin();

  displayInterface("System Ready!   ", "Scan Your Card  ");
  executeAudioFeedback(100, 2); 
  Serial.println("System: Inisialisasi arsitektur v1 berhasil.");
}

/**
 * @brief Regenerasi token OTP 6-angka secara non-blocking setiap siklus 30 detik
 */
void updateGlobalOTP() {
  if (millis() - lastOTPRotationMillis >= OTP_CYCLE_MS) {
    lastOTPRotationMillis = millis();
    currentGlobalOTP = random(100000, 1000000); 
    Serial.print("Log [OTP]: Rotasi Siklus Otomatis. OTP Baru -> ");
    Serial.println(currentGlobalOTP);
    
    delay(50); 
    sendDataToFirebase(true);
  }
}

/**
 * @brief Membaca parameter metrik suhu lingkungan dari sensor DHT11 setiap 5 detik
 */
void readEnvironmentalMetrics() {
  static unsigned long lastDHTCheck = 0;
  if (millis() - lastDHTCheck >= 5000) { 
    lastDHTCheck = millis();
    float temp = dht.readTemperature();
    if (!isnan(temp)) {
      currentTemperature = temp;
      Serial.printf("Log [DHT11]: Suhu Ruang Kotak ABS = %.1f *C\n", temp);
      
      sendDataToFirebase(false);
    }
  }
}

/**
 * @brief Sinkronisasi super cepat khusus untuk saat kartu ditap (Jalur Tol)
 */
void syncOTPFastLane() {
  if (WiFi.status() == WL_CONNECTED) {
    Firebase.setString(fbdo, "/devices/TA-ESP32-V1/security_state/active_otp", String(currentGlobalOTP));
    Firebase.setBool(fbdo, "/devices/TA-ESP32-V1/security_state/is_otp_used", false); 
  }
}

/**
 * @brief Pengiriman data telemetry terstruktur ke Firebase Realtime Database
 */
void sendDataToFirebase(bool isNewOTP) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Firebase: Gagal sinkronisasi, WiFi terputus.");
    return;
  }

  Firebase.setString(fbdo, "/devices/TA-ESP32-V1/system_status/device_id", "TA-ESP32-V1");
  Firebase.setBool(fbdo, "/devices/TA-ESP32-V1/system_status/is_online", isOnline);
  Firebase.setInt(fbdo, "/devices/TA-ESP32-V1/system_status/last_sync", millis() / 1000); 

  Firebase.setFloat(fbdo, "/devices/TA-ESP32-V1/environmental_metrics/temperature", currentTemperature);
  Firebase.setString(fbdo, "/devices/TA-ESP32-V1/environmental_metrics/sensor_status", "OK");

  if (isNewOTP) {
    bool s1 = Firebase.setString(fbdo, "/devices/TA-ESP32-V1/security_state/active_otp", String(currentGlobalOTP));
    bool s2 = Firebase.setBool(fbdo, "/devices/TA-ESP32-V1/security_state/is_otp_used", false); 
    Firebase.setInt(fbdo, "/devices/TA-ESP32-V1/security_state/otp_generated_at", millis() / 1000);

    if (s1 && s2) {
      Serial.println("Firebase: SUCCESS Sync OTP State.");
    } else {
      Serial.print("Firebase: ERROR Sync Failed! Reason: ");
      Serial.println(fbdo.errorReason());
    }
  }
}

/**
 * @brief Utilitas pembaruan teks antarmuka LCD 16x2
 */
void displayInterface(const String& line1, const String& line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

/**
 * @brief Modulasi sinyal digital aktif buzzer
 */
void executeAudioFeedback(unsigned int durationMs, unsigned int count) {
  for (unsigned int i = 0; i < count; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(durationMs);
    digitalWrite(PIN_BUZZER, LOW);
    if (count > 1) {
      delay(100);
    }
  }
}

/**
 * @brief Konversi data biner register UID RFID menjadi format String biner HEX
 */
String extractCardUID() {
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();
  return uidString;
}

/**
 * @brief Eksekusi pembukaan visual OTP global selama jendela 10 detik (Flicker-Free & Clean)
 */
void handleAuthorizedAccess() {
  Serial.println("Status: Otentikasi Sukses. Menampilkan OTP Global.");
  executeAudioFeedback(80, 2); 

  currentGlobalOTP = random(100000, 1000000);
  lastOTPRotationMillis = millis();

  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("OTP: " + String(currentGlobalOTP)); 
  lcd.setCursor(0, 1);
  lcd.print("Hide In: 10 Sec ");

  syncOTPFastLane();

  unsigned long startWindowMillis = millis();
  unsigned long remainingWindowTime = 10;
  unsigned long lastSecTracker = 99; 

  while (millis() - startWindowMillis < WINDOW_VIEW_MS) {
    remainingWindowTime = 10 - ((millis() - startWindowMillis) / 1000);
    
    if (remainingWindowTime != lastSecTracker) {
      lastSecTracker = remainingWindowTime; 
      
      lcd.setCursor(0, 1);
      lcd.print("Hide In: " + String(remainingWindowTime) + " Sec ");
    }
    
    delay(20); 
  }

  Serial.println("Status: Durasi intip selesai. OTP disembunyikan kembali.");
  displayInterface("OTP Hidden!     ", "Akses Ditutup   ");
  executeAudioFeedback(300, 1); 
  delay(1500);
  
  displayInterface("ThermaAuth v1", "Scan Your Card  ");
}

/**
 * @brief Penanganan alert visual dan audio untuk kegagalan otentikasi kartu
 */
void handleUnauthorizedAccess(const String& invalidUID) {
  Serial.print("Status: Akses Ditolak! UID Tidak Dikenal -> ");
  Serial.println(invalidUID);
  
  displayInterface("Akses Ditolak!  ", "Kartu Ilegal    ");
  executeAudioFeedback(800, 1); 
  delay(1500);
  displayInterface("ThermaAuth v1", "Scan Your Card  ");
}