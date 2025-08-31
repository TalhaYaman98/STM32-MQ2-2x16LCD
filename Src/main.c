
#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "lcd_config.h"
#include "delay.h"
#include "mq2.h"


void clock_config(void)
{

	RCC->CR |= RCC_CR_HSEON;						   // HSE (Harici osilatör) aktif et
	while(!(RCC->CR & RCC_CR_HSERDY));				   // HSE hazır olana kadar bekle

	FLASH->ACR = FLASH_ACR_DCEN | 	                   // Data Cache'i etkinleştir
	             FLASH_ACR_ICEN | 	                   // Instruction Cache'i etkinleştir
	             FLASH_ACR_LATENCY_2WS;                // 72MHz için 2 bekleme durumu ayarla

    RCC->PLLCFGR = (8 << RCC_PLLCFGR_PLLM_Pos) |       // PLLM = 8
                   (144 << RCC_PLLCFGR_PLLN_Pos) |     // PLLN = 144
                   (0 << RCC_PLLCFGR_PLLP_Pos) |       // PLLP = 2 (0: /2)
                   (RCC_PLLCFGR_PLLSRC_HSE) |          // PLL kaynağı = HSE
                   (7 << RCC_PLLCFGR_PLLQ_Pos);        // PLLQ = 7 (USB, SDIO vs. için)

    RCC->CR |= RCC_CR_PLLON;					// PLL'yi aktif et
    while(!(RCC->CR & RCC_CR_PLLRDY));			// PLL'nin stabil hale gelmesini bekle

    RCC->CFGR |= RCC_CFGR_SW_PLL;                     			 // PLL'yi sistem clock kaynağı olarak seç
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); 	 // PLL geçişi tamamlanana kadar bekle

}

/*

Harici Osilatörü Etkinleştirme:

	RCC->CR |= RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY));
	STM32F4 kartında bulunan 8 MHz'lik harici kristal osilatörü (HSE) etkinleştirir. Bu osilatör, PLL (Phase-Locked Loop) için bir kaynak görevi görür.
	Kod, osilatörün stabil hale gelmesini ve kullanıma hazır olmasını bekler.

Flash Bellek Ayarları:

	FLASH->ACR = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_LATENCY_2WS;
	Bu adım, en kritik noktalardan biridir. Flash bellek, CPU'dan daha yavaş olduğu için, saat hızı arttığında CPU'nun veriyi doğru bir şekilde okuyabilmesi için bekleme süreleri (wait states) eklenir.
	72 MHz için 2 bekleme durumu ayarlanır. Ayrıca, veri ve komut önbellekleri (DCEN ve ICEN) etkinleştirilerek, veri okuma işlemleri hızlandırılır.

PLL Yapılandırması:

	RCC->PLLCFGR = ...
	PLL, giriş frekansını (HSE) bölerek, çarparak ve tekrar bölerek nihai bir frekans oluşturan bir frekans çarpanıdır.
	Bu satırda, 72 MHz'lik sistem saatini elde etmek için gerekli matematiksel ayarlar yapılır:
	Giriş frekansı 8'e bölünür (8MHz / 8 = 1 MHz).
	Bu 1 MHz frekansı 144 ile çarpılır (1 MHz * 144 = 144 MHz).
	Son olarak, elde edilen bu frekans 2'ye bölünerek nihai sistem saati olan 72 MHz elde edilir (144 MHz / 2 = 72 MHz).

PLL'yi Aktif Etme ve Bekleme:

	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY));
	Yapılandırması tamamlanan PLL'yi etkinleştirir ve PLL'nin stabil hale gelmesini bekler.

Sistem Saat Kaynağını Değiştirme:

	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	Son adımda, mikrodenetleyicinin ana saat kaynağı, varsayılan HSI veya HSE yerine, artık güvenli bir şekilde ayarlanan PLL'ye yönlendirilir. Kod, bu geçişin başarıyla tamamlandığını doğrular.

Buradaki parametreler:

	M (RCC_PLLCFGR_PLLM_Pos): Giriş saat frekansını (HSE) bölen ön bölücüdür.
							  Buradaki amaç, VCO'ya giren frekansın belirli bir aralıkta (genellikle 1-2 MHz) olmasını sağlamaktır.

	N (RCC_PLLCFGR_PLLN_Pos): VCO frekansını yükseltmek için kullanılan çarpandır.
							  Frekansı asıl artıran parametre budur.

	P (RCC_PLLCFGR_PLLP_Pos): VCO çıkışını, ana sistem saatini (SYSCLK) elde etmek için bölen son bölücüdür.
							  Bu bölücü 2'nin katlarıdır ve genellikle 2, 4, 6 veya 8 olarak ayarlanır.
							  Sizin kodunuzda 0 yazması, 2'ye böl anlamına gelir.

	Q (RCC_PLLCFGR_PLLQ_Pos): USB OTG FS, SDIO ve RNG gibi çevresel birimlerin saat frekansını elde etmek için kullanılan başka bir bölücüdür.
							  Ana sistem saatini doğrudan etkilemez, ancak bu çevresel birimler için doğru bir frekans sağlamak önemlidir.
*/

void gpioD_config(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

	GPIOD->MODER &= ~( (0x3 << GPIO_MODER_MODER12_Pos) |		// PD12, PD13, PD14, PD15 için bitleri temizle
					   (0x3 << GPIO_MODER_MODER13_Pos) |
					   (0x3 << GPIO_MODER_MODER14_Pos) |
					   (0x3 << GPIO_MODER_MODER15_Pos) );

	GPIOD->MODER |= ( (0x1 << GPIO_MODER_MODER12_Pos) |		// PD12, PD13, PD14, PD15 için çıkış modunu ayarla
			          (0x1 << GPIO_MODER_MODER13_Pos) |
					  (0x1 << GPIO_MODER_MODER14_Pos) |
					  (0x1 << GPIO_MODER_MODER15_Pos) );

	GPIOD->OTYPER &= ~( (0x1 << GPIO_OTYPER_OT12_Pos) |	 	// PD12, PD13, PD14, PD15 için Push-pull çıkış tipi
					    (0x1 << GPIO_OTYPER_OT13_Pos) |
						(0x1 << GPIO_OTYPER_OT14_Pos) |
						(0x1 << GPIO_OTYPER_OT15_Pos) );

	GPIOD->OSPEEDR |= ( (0x3 << GPIO_OSPEEDR_OSPEED12_Pos) |	// PD12, PD13, PD14, PD15 için Çok yüksek hızda çıkış (Very High Speed)
						(0x3 << GPIO_OSPEEDR_OSPEED13_Pos) |
						(0x3 << GPIO_OSPEEDR_OSPEED14_Pos) |
						(0x3 << GPIO_OSPEEDR_OSPEED15_Pos) );

	GPIOD->PUPDR &= ~( (0x3 << GPIO_PUPDR_PUPD12_Pos) |		// PD12, PD13, PD14, PD15 için Dahili dirençleri kapat (No Pull-up/Pull-down)
					   (0x3 << GPIO_PUPDR_PUPD13_Pos) |
					   (0x3 << GPIO_PUPDR_PUPD14_Pos) |
					   (0x3 << GPIO_PUPDR_PUPD15_Pos) );

	GPIOD->BSRR = (1 << (12 + 16));

}



void relay_pd12(int odr)
{
	if(odr == 1)
	{
		// Pini HIGH yapmak için BSRR'nin alt kısmına yaz
		// (1 << 12) = 12. bite 1 yaz
		GPIOD->BSRR = (1 << 12);
	}


	else if(odr == 0)
	{
		// Pini LOW yapmak için BSRR'nin üst kısmına yaz
		// (1 << (12 + 16)) = 28. bite 1 yaz
		GPIOD->BSRR = (1 << (12 + 16));
	}
}
/*

Röle Nedir?
Röle, temelde elektromanyetik bir anahtardır.
Düşük voltajlı bir sinyal (mikrodenetleyiciden gelen 3.3V) ile yüksek voltajlı bir devreyi (oda lambası gibi 220V'luk bir yük) kontrol etmemizi sağlar.
Bu, mikrodenetleyici ile yüksek güçlü cihazlar arasında güvenli bir izolasyon katmanı oluşturur.

Çalışma Mantığı
	Bir rölenin çalışma prensibi 3 ana bölümden oluşur:

	Bobin (Kontrol Tarafı):
		Rölenin içinde, üzerinden akım geçtiğinde manyetik alan oluşturan bir bobin bulunur.
		Bizim durumumuzda, bu bobin STM32'den gelen 3.3V'luk sinyal ile (relay_pd12(1)) enerjilendirilir.

	Elektromanyetik Alan:
		Bobin enerjilendiğinde, güçlü bir manyetik alan oluşur.
		Bu manyetik alan, rölenin içindeki bir metal kolu veya anahtarı kendine doğru çeker.

	Anahtar Kontakları (Yük Tarafı):
		Metal kol, manyetik alanın etkisiyle hareket ederek bir dizi kontağa (pinlere) temas eder.
		Bu kontaklar genellikle üç farklı şekilde etiketlenir:
			COM (Common): Ortak giriş ucu.
			NO (Normally Open - Normalde Açık): Bobin enerjisizken açık olan uç. Yani, anahtar bu uca temas etmez.
			NC (Normally Closed - Normalde Kapalı): Bobin enerjisizken kapalı olan uç. Yani, anahtar bu uca temas eder.

Bu kısım, STM32'nizin röle modülünü kontrol etmesini sağlar. Düşük voltajlı ve güvenli kısımdır.

	Röle Modülü VCC Pini → STM32F4 Discovery Kartı 5V Pini
		Bu bağlantı, röle modülünün çalışması için gerekli olan gücü sağlar.

	Röle Modülü GND Pini → STM32F4 Discovery Kartı GND Pini
		İki cihaz arasında ortak bir toprak referansı oluşturur, bu da sinyal iletişiminin doğru çalışması için hayati önem taşır.

	Röle Modülü IN Pini → STM32F4 Discovery Kartı PD12 Pini
		Bu, STM32'nin röleyi açıp kapatmak için kullandığı sinyal kablosudur. Kodumuzdaki relay_pd12 fonksiyonu bu pini kontrol eder.

Bu kısım, lambayı açıp kapamak için kullanılır ve 220V ile çalıştığı için dikkatle ele alınmalıdır.

	220V Prizden Gelen Faz Kablosu (Genellikle kahverengi veya siyah) → Röle Modülünün COM Pini
		Faz kablosu, röle anahtarının ortak girişine bağlanır.

	Röle Modülünün NO Pini (Normalde Açık) → Oda Lambasının Birinci Kablosu
		Anahtar kapalıyken (röle aktif olduğunda) akımın lambaya gitmesi için bu kablo kullanılır.

	Oda Lambasının İkinci Kablosu → 220V Prizden Gelen Nötr Kablosu (Genellikle mavi)
		Devreyi tamamlamak için nötr kablosu doğrudan lambaya bağlanır.

*/

int main(void)
{
	int sayac = 0;
	char buffer[5];
    uint16_t sensor_value;
    char buffer2[16];

    clock_config();	// Sistem saatini 72 MHz'e ayarla
    systick_config();
    DWT_Delay_Init();
    gpioD_config();
    lcd_init();	// LCD'yi başlat ve 4-bit moda ayarla
    gpio_pa0_analog_init();
    adc1_init();

    lcd_set_cursor(0, 0);
    lcd_print_string("Merhaba Dunya!");	// Ekrana "Merhaba Dunya!" yazdır
    delay_ms(500);
    lcd_clear();

    while(1)
    {

    	lcd_clear();
    	sayac++;

    	sensor_value = adc1_read();

    	sprintf(buffer, "%d", sensor_value);
    	sprintf(buffer2, "%d", sayac);

    	lcd_set_cursor(0, 0);
    	lcd_print_string(buffer);

    	lcd_set_cursor(0, 15);
    	lcd_print_string(buffer2);

        // Belirli bir eşik değeri kontrolü yap
        // Bu eşik değerini kendi denemelerinize göre ayarlayabilirsiniz.
        if (sensor_value > 2300) // Örnek bir eşik değeri
        {
            relay_pd12(0); // Gaz algılandığında lambayı yak
            lcd_set_cursor(1, 13);
            lcd_print_string("ON");
            delay_ms(50);
        }
        else
        {
            relay_pd12(1); // Gaz yoksa lambayı söndür
            lcd_set_cursor(1, 13);
            lcd_print_string("OFF");
            delay_ms(50);
        }

        // Ölçümler arasında kısa bir gecikme ekle
        delay_ms(500);

    	/*
    	delay_ms(100);
    	GPIOD->ODR ^= ( (0x1 << GPIO_ODR_OD12_Pos) |	// PD12, PD13, PD14, PD15 için Toggle
    	                (0x1 << GPIO_ODR_OD13_Pos) |
    	        		(0x1 << GPIO_ODR_OD14_Pos) |
    	               	(0x1 << GPIO_ODR_OD15_Pos) );

		*/
    }

}
