# STM32F4 – MQ2 Gaz Sensörü ve Röle Kontrollü LCD Uygulaması

Bu proje, **STM32F4 Discovery** geliştirme kartı kullanılarak hazırlanmıştır. Amaç, **MQ2 gaz sensöründen alınan analog veriyi ölçmek**, bu değeri **16x2 LCD ekranda göstermek** ve belirli bir eşik değer aşıldığında **röle aracılığıyla bir yükü (ör. lamba)** kontrol etmektir.  

Proje, yalnızca **CMSIS kütüphanesi** kullanılarak yazılmıştır (HAL kütüphanesi kullanılmamıştır). Bu sayede donanım register’larına doğrudan erişim sağlanmıştır.

---

## 🚀 Projenin Özellikleri

- Sistem saati **72 MHz PLL** ile yapılandırılmıştır.  
- **SysTick Timer** ve **DWT** kullanılarak **ms ve µs cinsinden gecikme fonksiyonları** yazılmıştır.  
- **GPIO konfigürasyonu**: Röle kontrolü için **PD12 pini** çıkış olarak ayarlanmıştır.  
- **ADC konfigürasyonu**: MQ2 gaz sensörü için **PA0 pini analog giriş** olarak kullanılmıştır.  
- **LCD (16x2) kontrolü**: 4-bit modda çalışacak şekilde konfigüre edilmiştir.  
- Sensör verisi **ekranda görüntülenir** ve sayaç değeri ile birlikte yazdırılır.  
- Belirlenen eşik değer üzerinde gaz algılandığında:
  - Röle aktif edilerek bağlı yük **açılır**.  
  - LCD’de **ON** yazısı gösterilir.  
- Gaz yoksa:
  - Röle kapatılır, yük kapanır.  
  - LCD’de **OFF** yazısı gösterilir.  

---

## 🛠 Kullanılan Donanımlar

- STM32F4 Discovery Board  
- MQ2 Gaz Sensörü  
- Röle Modülü (5V)  
- 16x2 LCD Ekran  
- Bağlantı kabloları ve breadboard  
- Harici güç kaynağı (gerekirse)  

---

## ⚡ Çalışma Mantığı

1. **clock_config()** ile sistem saat frekansı 72 MHz’e ayarlanır.  
2. **gpioD_config()** ile PD12 pini röle çıkışı olarak hazırlanır.  
3. **lcd_init()** çağrılarak LCD 4-bit modda başlatılır.  
4. **gpio_pa0_analog_init()** ve **adc1_init()** ile MQ2 sensörünün bağlı olduğu **PA0 pini** ADC girişine hazırlanır.  
5. **adc1_read()** ile sensör verisi alınır.  
6. Sensör değeri ekranda gösterilir, sayaç ile birlikte yazdırılır.  
7. Eğer **ADC değeri > 2300** ise röle aktif edilir (**relay_pd12(0)** → lamba ON).  
8. Aksi durumda röle kapatılır (**relay_pd12(1)** → lamba OFF).  

---

## 📊 LCD Ekran Çıktısı Örneği

- Sol kısım: MQ2 sensöründen okunan değer  
- Sağ üst köşe: Sayaç  
- Sağ alt köşe: Röle durumu (ON / OFF)  

---

## 🔌 Röle Bağlantısı

- **VCC → 5V**  
- **GND → GND**  
- **IN → PD12**  
- Röle Çıkışı:
  - **COM → 220V faz hattı**  
  - **NO → Lamba (faz girişi)**  
  - **Lamba diğer ucu → 220V nötr hattı**  

⚠️ **Dikkat:** Röle ile 220V çalışırken güvenlik önlemleri alınız.  

---

## 📝 Derleme ve Yükleme

- Derleyici: **ARM Keil / STM32CubeIDE / vs.**  
- Board: **STM32F407 Discovery**  
- Program yükleme: **ST-LINK**  

---

## 📌 Donanım Bağlantıları

Bileşen	      Pinler	Bağlantı Yeri

MQ2 Sensörü  	VCC    	5V

              GND    	GND
              
              AOUT	  STM32 PA0
              
LCD Ekran	    RS	    STM32 PA1

              E	      STM32 PA3
              
              D4-D7	  STM32 PB4-PB7
              
Röle Modülü  	VCC	    5V

              GND	    GND
              
              IN	    STM32 PD12

---
