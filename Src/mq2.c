
#include "stm32f4xx.h"
#include "mq2.h"

uint16_t adc_value;

void gpio_pa0_analog_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;               // GPIOA clock'u aktif et
    GPIOA->MODER |= (3 << (0 * 2));                    // PA0 modunu '11' (Analog) yap
    GPIOA->PUPDR &= ~(3 << (0 * 2));                   // PA0 için pull-up/pull-down yok
}

/*

RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	Ne yapar: GPIOA'nın clock'unu etkinleştirir. AHB1ENR içindeki ilgili bit set edilmeden GPIOA register'larına yazmak etkisizdir.
	Neden önemli: Clock kapalı olursa yazdığınız konfigürasyonlar uygulanmaz veya belirsiz sonuç verir.
	Öneri: Bazı tasarımlarda (void)RCC->AHB1ENR; gibi bir read-back eklenir, bus senkronizasyonu için faydalıdır.

GPIOA->MODER |= (3 << (0 * 2));
	Ne yapar: PA0 için MODER[1:0] = 11 → Analog mode. Analog modda pin dijital giriş/çıkış mantığından çıkar ve ADC girişine bağlanır.
	Dikkat: |= kullanımı mevcut üst bitleri bozmayabilir; fakat en güvenli yol önce ilgili iki biti temizleyip (&= ~mask) sonra |= value ile yazmaktır.
	Neden analog: ADC girişleri için pinin dijital input buffer'ı devre dışı bırakılmalı; analog mod bunu sağlar.

GPIOA->PUPDR &= ~(3 << (0 * 2));
	Ne yapar: PA0 için internal pull-up/pull-down devrelerini kapatır (00).
	Neden: Analog ölçümlerde pull-up/pull-down'lar ölçümü bozabilir; bu yüzden genelde kapatılır.
	Dikkat: Eğer devrenizde harici bir bias (pull-down/up) yoksa ve giriş floating oluyorsa ölçüm yanlış olabilir. Analog girişin bağlı olduğu sensör/devre referansını kontrol edin.

Ek notlar:
	Analog moddayken pinin dijital sürücüsü devre dışıdır — dolayısıyla LED gibi dijital yüklemelerden kaçının.
	Yüksek kaynak empedanslı sinyaller (örn. >10kΩ) için örnekleme süresi uzatılmalıdır; aksi halde ADC sample/hold kapasitörü doğru değeri alamaz (bunu SMPRx ile ayarlıyoruz).

*/

void adc1_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;                // ADC1 clock'unu aktif et
    ADC1->CR1 = 0;                                     // CR1 varsayılan (8-bit çözünürlük yok, default 12-bit)
    ADC1->CR2 = ADC_CR2_ADON;                          // ADC1'i aktif et
    ADC1->SMPR2 |= (3 << (3 * 0));                     // Kanal 0 için örnekleme süresi = 56 cycle
}

/*

Amaç: ADC1'e temel konfigürasyonu yapmak (clock, örnekleme zamanı, ADC aktif etme).
Aşağıdaki açıklama önceki örnekteki minimal adımları kapsar ve ardından hangi ekstra ayarların önerildiğini açıklar.

RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	Ne yapar: ADC1 çevresine saat sağlar. ADC register'ları etkin hale gelir.
	Neden: ADC register'larına yazabilmek için APB2'deki ADC1 clock'unun açık olması gerekir.

ADC1->CR1 = 0;
	Ne yapar: CR1 register'ını sıfırlar (örn. çözünürlük, scan modu vb. ayarlarını varsayılan yapar).
	Neden: Basit örneklerde 12-bit, single conversion için default ayarlar yeterlidir. Ancak üretim kodunda = yerine |=/&= kullanıp sadece ihtiyacınız olan bitleri değiştirmek daha güvenlidir.
	Dikkat: CR1 içinde RES (resolution) alanı gibi önemli konfigürasyonlar vardır (12/10/8/6 bit). Eğer farklı çözünürlük istenirse CR1'de RES bitlerini set edin.

ADC1->CR2 = ADC_CR2_ADON;
	Ne yapar: ADC'yi aktif eder (ADON bitini set eder). ADON = 1 ADC'yi uyanık hale getirir.
	Neden: ADC çalışmaya başlamadan önce ADON set edilmelidir.
	Dikkat ve öneri: = yerine |= kullanmak genelde daha güvenli (diğer konfigürasyon bitlerini korumak için).
	Ayrıca bazı referanslarda ADC'yi etkinleştirdikten sonra kısa bir stabilizasyon gecikmesi önerilir (birkaç mikro-saniye).
	Not: Bazı STM32 serilerinde ADON set-up dizisi farklı davranabilir (ilk ADON uyanma, ikinci ADON veya SWSTART dönüşümü başlatma).
	Genel güvenli akış: configure → ADON set → küçük delay → başlatma (SWSTART).

ADC1->SMPR2 |= (3 << (3 * 0));
	Ne yapar: Channel0 (SMP0 alanı) için örnekleme süresini ayarlar. 3 değeri burada 3-bitlik saha içine (011) yazılır; genelde bu kod '56 cycles' karşılığıdır (kod tablosuna göre).
	Neden: Örnekleme süresi ADC sample & hold kapasitörünün kaynak tarafından yeterince doldurulmasını sağlar. Yüksek empedanslı bir kaynaktan (büyük R) ölçülüyorsa daha uzun örnekleme zamanı gerekir.
	Öneri: Kaynak impedansına göre uygun SMP seçin; kısa SMP → hızlı ama doğruluk düşebilir; uzun SMP → daha yavaş ama daha güvenilir. Tipik tabloda kodlar: 000=3 ciklus, 001=15, 010=28, 011=56, 100=84, 101=112, 110=144, 111=480 (bu sıralama STM32F4 referans manualine uygundur).

Ek önemli noktalar (uygulamada göz önünde bulundurulmalı):
	ADC ortak prescaler: ADC saat kaynağı APB2 üzerinden gelir; ADC'nin çalışma frekansı (ADCCLK) datasheette belirtilen maksimum (genelde 36 MHz) üstünde olmamalıdır.
						 ADC prescaler (ADC common control register — CCR) ile ADCCLK'i PCLK2/2, /4, /6 veya /8 yapabilirsiniz.
						 Pratikte PCLK2 = 84 MHz ise ADCCLK = 84/2 = 42 MHz (bu 36MHz sınırını aşar) → bu yüzden ADC_CCR içinde uygun prescaler seçilmeli (ör. /4 → 21 MHz).
	CR2 diğer bitleri: ALIGN (yerleştirme), CONT (continuous conversion), DMA enable gibi ayarlar burada belirlenir; mevcut örnek single conversion (tek ölçüm) için minimal ayarlar yapar.
	Kalibrasyon: STM32F4'te otomatik ADC kalibrasyonu yoktur (bazı eski ve farklı serilerde vardır).
				 Bunun yerine referanslarınızı ve çevreyi kontrol edin, offset/error düzeltmeleri yazılımla yapılabilir.
	Stabilizasyon: ADC aktif edildikten sonra kısa bekleme (ör. birkaç µs) tavsiye edilir; üretim kodunda ADC'nin tam stabil olması beklenir.

*/

uint16_t adc1_read(void) {
    ADC1->SQR3 = 0;                                    // Sadece Kanal 0 seçildi
    ADC1->CR2 |= ADC_CR2_SWSTART;                      // Yazılım ile dönüşümü başlat
    while (!(ADC1->SR & ADC_SR_EOC));                  // Dönüşüm tamamlanana kadar bekle
    return (uint16_t)ADC1->DR;                         // Ölçüm sonucunu döndür
}

/*

Amaç: ADC1 kanal0 için yazılım başlatmalı (polling) tek dönüşüm yapmak ve sonucu döndürmek.

ADC1->SQR3 = 0;
	Ne yapar: Regular sequence register 3 (SQR3) içindeki SQ1 alanına 0 yazar; yani yapılacak ilk (ve tek) dönüşüm kanalı 0 olarak seçilir. SQR3'ün SQ1 alanı bit[4:0]'tadır.
	Dikkat: Eğer birden fazla kanal okunacaksa SQR1/SQR2/SQR3 register'larında SQx sırasına göre kanal numaralarını yerlestirirsiniz; ayrıca L[3:0] alanıyla sıra uzunluğu belirlenir.

ADC1->CR2 |= ADC_CR2_SWSTART;
	Ne yapar: Yazılım komutu ile ADC'yi başlatır (SWSTART). Eğer ADC CR2'de CONT=0 ise tek dönüşüm yapılır.
	Önemli: ADC'nin ADON bitinin zaten set olduğundan ve ADC stabilize olduğundan emin olun; aksi halde SWSTART etkisiz olabilir.

while (!(ADC1->SR & ADC_SR_EOC));
	Ne yapar: Polling ile dönüşümün tamamlanmasını bekler; EOC (End Of Conversion) biti set olunca döngü kırılır.
	Dikkat: Polling uzun sürerse CPU meşgul olur; daha düşük enerji/performans ihtiyacı varsa interrupt veya DMA kullanmayı düşünün.

return (uint16_t)ADC1->DR;
	Ne yapar: ADC veri register'ından (DR) sonucu okur ve döndürür. OKUMA işlemi EOC flag'ini temizler (okuma DR ile EOC temizleme davranışı referans manualine göre).
	Önemli: DR okuması aynı zamanda register'ı "consume" eder; tekrar EOC kontrolü yapılmadan DR okumak yanlış olur.
	Ayrıca DR okuması aynı zamanda Overrun flag'lerini etkileyebilir; eğer eşzamanlı çok hızlı dönüşümler varsa OVR kontrolü düşünülmeli.

Ek tavsiyeler:
	EOC'yi bekledikten sonra DR okumak en güvenli temizleme yöntemidir. Alternatif olarak bazı uygulamalarda ADC1->SR &= ~ADC_SR_EOC; gibi doğrudan temizleme yoktur — EOC write-0 ile temizlenmez, DR okuması gereklidir. (Bu MCU serisinde EOC, DR okuması ile temizlenir.)
	Eğer çok sık örnekleme yapılacaksa CONT modunu ve DMA'yı etkinleştirip continuous + DMA yaklaşımı kullanmak en verimli yöntemdir.
	Yazılımda timeout mekanizması eklemek iyi bir güvenlik pratiğidir (sonsuz döngüden kurtarmak için).

*/

//-------------------------------------------------------------------------------------------------------------------------------------------------

/*

MQ2 Gaz Sensörü Nedir?
MQ2, LPG (Sıvılaştırılmış Petrol Gazı), propan, hidrojen, metan ve hatta duman gibi yanıcı gazların ve dumanın varlığını tespit etmek için tasarlanmış bir sensördür.
Temel olarak, bir hassas direnç sensörüdür ve bu direncin değeri, çevresindeki havadaki gaz konsantrasyonuna göre değişir.

Çalışma Mantığı
	Sensörün ana elemanı, bir ısıtıcı tel ve bir gaz algılayıcı katmandan oluşur.

	Isıtıcı Tel:
		Sensörün içinde, gaz algılayıcı katmanı ısıtmak için bir tel bulunur.
		Gazların doğru bir şekilde tespit edilebilmesi için bu katmanın belirli bir sıcaklıkta tutulması gerekir. Bu nedenle sensör, çalıştırıldıktan sonra kısa bir süre ısınmaya ihtiyaç duyar.
		Isıtma işlemi sürekli devam eder.

	Gaz Algılayıcı Katman (SnO2):
		Bu katman, kalay dioksit (SnO2) gibi bir yarı iletken malzemeden yapılmıştır.
		Temiz havada, sensörün direnci çok yüksektir.
		Ortamda yanıcı gazlar bulunduğunda, bu gaz molekülleri SnO2 katmanına çarpar ve kimyasal reaksiyona girer. Bu reaksiyon, SnO2'nin iç direncini düşürür.

Çıkış Sinyalleri
	MQ2 sensörünün iki ana çıkışı vardır:

	AOUT (Analog Çıkış):
		Sensörün direncindeki değişime bağlı olarak bir analog voltaj sinyali üretir.
		Ortamdaki gaz konsantrasyonu ne kadar yüksekse, sensörün direnci o kadar düşer ve çıkış voltajı o kadar artar.
		Bu çıkış, gaz seviyesini orantılı olarak ölçmek için kullanılır. Örneğin, bir voltaj okuması alarak havadaki gaz miktarının ne kadar olduğunu tahmin edebilirsiniz.
		Bu, daha hassas uygulamalar için idealdir.
		Bizim projemizde, STM32'nin ADC (Analog-to-Digital Converter) birimi ile bu voltajı okuyacağız.

	DOUT (Dijital Çıkış):
		Bu pin, bir karşılaştırma (comparator) devresine bağlıdır.
		Sensörün voltajı, bir potansiyometre ile ayarlanabilen bir eşik değerini aştığında, bu pinin çıkışı HIGH olur (örneğin 3.3V veya 5V).
		Gaz seviyesi eşik değerinin altındaysa, çıkış LOW olur (0V).
		Bu çıkış, basit bir gaz algılama alarmı için kullanılır: "Gaz var" veya "Gaz yok".

Özetle: Biz projemizde, gaz seviyesini bir sayısal değer olarak okuyabilmek için MQ2'nin AOUT (Analog) çıkışını kullanacağız.
Bu analog sinyali, STM32'nin ADC birimi ile dijital bir sayıya dönüştüreceğiz.
Bu sayıyı daha sonra LCD ekranına yazdırabilir ve belirli bir eşik değeri geçtiğinde röleyi tetikleyerek bir alarm (örneğin lamba yakma) verebiliriz.

*/
