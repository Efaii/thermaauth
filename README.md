# 🔌 ThermaAuth v1 - ESP32 Firmware

Repositori ini berisi kode sumber (*source code*) C++ untuk mikrokontroler **ESP32** yang bertindak sebagai "otak" perangkat keras dari sistem keamanan pintu masuk (Smart Gate) **ThermaAuth**. 

Perangkat ini terintegrasi dengan sensor RFID, sensor suhu, layar LCD, dan melakukan sinkronisasi dua arah (*bidirectional sync*) secara mulus dengan **Firebase Realtime Database** untuk dihubungkan ke [Web Portal ThermaAuth](https://web-thermaauth.vercel.app/).

---

## 🚀 Fitur Utama (Hardware)

- **Rotasi OTP Cerdas**: Menghasilkan sandi 6-digit (OTP) dinamis yang diperbarui otomatis setiap 30 detik.
- **RFID Trigger (Fast Lane)**: Memaksa pembuatan OTP baru secara instan jika kartu Master RFID disetujui (mencegah pencurian sisa waktu sandi).
- **Layar Interaktif**: Menggunakan LCD 16x2 dengan komunikasi I2C untuk menampilkan panduan pengguna dan waktu siaga OTP (10 detik).
- **Proteksi Re-Use**: Mengkomunikasikan status `is_otp_used` ke *cloud* untuk memastikan sandi hanya dapat digunakan satu kali oleh portal Web.
- **Monitoring Suhu Ruangan**: DHT11 terintegrasi untuk membaca suhu lingkungan dan mengunggah telemetrinya setiap 5 detik di belakang layar.

---

## 🛠️ Persyaratan Komponen (Hardware Requirements)

1. **Board**: ESP32 Development Board
2. **RFID Reader**: MFRC522 (13.56MHz)
3. **Sensor Suhu**: DHT11
4. **Display**: LCD 16x2 + I2C Backpack
5. **Notifikasi Audio**: Active Buzzer (5V/3.3V)
6. **Lainnya**: Kabel Jumper, Breadboard, dan Adaptor Daya Listrik (Jangan gunakan Baterai 18650 secara langsung tanpa modul regulator yang mumpuni untuk mencegah *Brownout WiFi*).

---

## 📌 Konfigurasi Pin (Pinout Mapping)

Pastikan pemasangan kabel mengikuti konfigurasi berikut:

| Komponen | Pin Modul | Pin ESP32 | Keterangan |
| :--- | :--- | :--- | :--- |
| **LCD I2C** | SDA | GPIO 5 | Komunikasi Data I2C |
| | SCL | GPIO 16 | Komunikasi Clock I2C |
| **RFID MFRC522** | SDA (SS) | GPIO 23 | SPI Chip Select |
| | SCK | GPIO 22 | SPI Clock |
| | MOSI | GPIO 21 | SPI Master Out |
| | MISO | GPIO 19 | SPI Master In |
| | RST | GPIO 14 | Reset Pin |
| **DHT11** | DATA | GPIO 4 | Pin Data Suhu |
| **Buzzer** | VCC/IN | GPIO 2 | Output Sinyal Audio |

---

## 💻 Instalasi & Cara Upload (Arduino IDE)

### 1. Persiapan Library
Pastikan Anda sudah menginstal *libraries* berikut di Arduino IDE Anda (melalui *Library Manager*):
- `LiquidCrystal I2C` (oleh Frank de Brabander)
- `MFRC522` (oleh GithubCommunity)
- `DHT sensor library` (oleh Adafruit)
- `Firebase ESP32 Client` (oleh Mobizt)

### 2. Penyesuaian Kredensial
Buka file `ThermaAuth_Final_v1.ino` lalu ubah baris berikut sesuai dengan jaringan dan database Anda:

```cpp
#define WIFI_SSID "NAMA_WIFI_ANDA"
#define WIFI_PASSWORD "PASSWORD_WIFI_ANDA"

// HANYA MASUKKAN DOMAIN UTAMA, TANPA "https://" ATAU "/" DI AKHIR
#define FIREBASE_HOST "thermaauth-default-rtdb.asia-southeast1.firebasedatabase.app"
// MASUKKAN DATABASE SECRET DARI FIREBASE PROJECT SETTINGS ANDA
#define FIREBASE_AUTH "MASUKKAN_DATABASE_SECRET_ANDA_DISINI"
```

> **Cara mendapatkan FIREBASE_AUTH**: 
> Buka Firebase Console > *Project Settings* (Ikon Roda Gigi) > *Service accounts* > *Database secrets*.

### 3. Mendaftarkan Kartu Master
Jika ingin mendaftarkan UID Kartu RFID lain, temukan variabel ini di kode:
```cpp
const String MASTER_CARD_UID = "8131105D"; 
```
Ganti string di atas dengan kode HEX kartu atau gantungan kunci (*keyfob*) RFID Anda sendiri (tanpa spasi).

### 4. Upload ke Board
Sambungkan ESP32 ke komputer, pilih Board (`DOIT ESP32 DEVKIT V1` atau sejenisnya) dan Port yang benar. Klik **Upload**. 
*(Jika terminal tertulis `Connecting...`, segera tekan dan tahan tombol **BOOT** di fisik ESP32 Anda sampai proses persentase berjalan).*

---

## 🔗 Tautan Terkait
* **Situs Web Live**: [ThermaAuth Portal](https://web-thermaauth.vercel.app/)
* **Repositori Web/Frontend**: [Efaii/web-thermaauth](https://github.com/Efaii/web-thermaauth)
