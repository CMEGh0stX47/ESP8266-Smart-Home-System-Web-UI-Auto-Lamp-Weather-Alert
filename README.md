# 🏠 Smart Home Pro – ESP8266

Sistem **Smart Home berbasis ESP8266** dengan Web Dashboard untuk monitoring sensor, kontrol perangkat, dan integrasi **Weather API** secara real-time.

## ✨ Fitur
- Monitoring suhu & kelembaban (DHT11)
- Monitoring intensitas cahaya (LDR)
- Kontrol lampu (Manual & Auto)
- Alarm / Buzzer peringatan
- Deteksi hujan (estimasi)
- Integrasi **Weather API (OpenWeatherMap)**
- Web UI responsif & real-time

## 🛠️ Komponen
- ESP8266 (NodeMCU)
- DHT11
- LDR
- Lamp / Relay
- Buzzer

## 📌 Koneksi Pin
- DHT11 → GPIO 2  
- LDR → A0  
- Lamp → GPIO 5  
- Buzzer → GPIO 4  

## ⚙️ Proses Pemasangan
1. Rangkai sensor dan output sesuai konfigurasi pin.
2. Install **Arduino IDE** dan board **ESP8266**.
3. Install library: `ESP8266WiFi`, `ESP8266WebServer`, `DHT`, `ArduinoJson`.
4. Masukkan SSID WiFi dan **API Key OpenWeatherMap** pada kode.
5. Upload program ke ESP8266.
6. Hubungkan HP/PC ke WiFi **SMART_CLOCK**.
7. Akses dashboard melalui browser: `192.168.4.1`.

## ☁️ Weather API
- Menggunakan **OpenWeatherMap API**
- Data yang diambil: suhu lingkungan
- Digunakan sebagai informasi cuaca tambahan di dashboard

⚠️ Jangan upload API Key asli ke repository publik.

![image](https://github.com/user-attachments/assets/e096d7b7-bb7d-497a-b716-69ac4b2d373a)


## 🧠 Cara Kerja
ESP8266 membaca sensor dan API cuaca secara berkala, lalu menampilkan data di Web Dashboard dan mengontrol perangkat sesuai mode yang dipilih.

## 🔐 Keamanan
Aman untuk pembelajaran dan prototype.  
Belum disarankan untuk penggunaan produksi.

## 📄 Lisensi
Open-source untuk tujuan edukasi.
