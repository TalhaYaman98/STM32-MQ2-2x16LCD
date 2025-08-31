
#include <stdint.h>
#include "stm32f4xx.h"
#include "lcd_config.h"
#include "delay.h"

/*

LCD Pini	Açıklama						STM32F4 Discovery Kartı Pini

VSS			Toprak (Ground)					GND
VDD			Güç Kaynağı						5V
VO			Kontrast Ayarı					- 10kΩ Potansiyometre (Ortadaki bacak, diğer uçlar VDD ve VSS'ye)
RS			Register Select (Komut/Veri)	GPIOA Pin 1
RW			Read/Write (Okuma/Yazma)		GND (Sürekli yazma modunda kalması için)
E			Enable (Etkinleştirme)			GPIOA Pin 3
D0-D3		Veri Pinleri (Kullanılmıyor)	-
D4			Veri Pini 4						GPIOB Pin 4
D5			Veri Pini 5						GPIOB Pin 5
D6			Veri Pini 6						GPIOB Pin 6
D7			Veri Pini 7						GPIOB Pin 7
A (LED+)	Arka Işık (+)					5V (Seri bir 220Ω-330Ω dirençle birlikte)
K (LED-)	Arka Işık (-)					GND

*/

void lcd_gpio_config(void)
{

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN; // GPIOA ve GPIOB'nin clock'larını etkinleştir

	GPIOA->MODER &= ~(GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3); 		// GPIOA pin 1,2,3'ün mod bitlerini temizle
	GPIOA->MODER |= (GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0); // GPIOA pin 1,2,3'ü çıkış moduna ayarla

    GPIOB->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7); 		// GPIOB pin 4,5,6,7'nin mod bitlerini temizle
    GPIOB->MODER |= (GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0); // GPIOB pin 4,5,6,7'yi çıkış moduna ayarla

    GPIOA->ODR &= ~(GPIO_ODR_OD1 | GPIO_ODR_OD3); 								// GPIOA pin 1 ve 3'ü LOW seviyesine ayarla
    GPIOB->ODR &= ~(GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD7); // GPIOB pin 4,5,6,7'yi LOW seviyesine ayarla

}

/*

Bu fonksiyonun amacı: LCD’nin RS, E ve veri pinleri (D4–D7) için gerekli GPIO ayarlarını yapmak.

RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
	GPIOA ve GPIOB portlarının clock’u açılır. Bu yapılmadan pinler kullanılamaz.

GPIOA->MODER &= ~(GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
	Önce PA1, PA2, PA3 pinlerinin mod register’ları temizlenir (resetlenir).
	PA1 = RS pini, PA2 = kullanılmıyor (boş bırakılmış), PA3 = E pini.

GPIOA->MODER |= (GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0);
	Bu satır ile PA1, PA2 ve PA3 çıkış (output) olarak ayarlanır.

GPIOB->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
	LCD’nin veri pinleri olan PB4–PB7 temizlenir.

GPIOB->MODER |= (GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0);
	PB4–PB7 çıkış moda alınır. Bu pinler LCD’ye veri göndermek için kullanılacak.

GPIOA->ODR &= ~(GPIO_ODR_OD1 | GPIO_ODR_OD3);
	RS ve E pinleri başta LOW seviyeye çekilir. Böylece LCD yanlış tetiklenmez.

GPIOB->ODR &= ~(GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD7);
	Veri pinleri başlangıçta LOW yapılır.

Özet: Bu fonksiyon, LCD’nin ihtiyaç duyduğu tüm GPIO pinlerini çıkış moduna getirir ve hepsini LOW yaparak temiz bir başlangıç sağlar.

*/

void lcd_send_nibble(uint8_t nibble)
{

	GPIOA->ODR &= ~GPIO_ODR_OD3; // Enable pinini (PA3) LOW yap

	GPIOB->ODR &= ~(GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD7); // Veri pinlerini temizle

	if (nibble & 0x01) GPIOB->ODR |= GPIO_ODR_OD4; // Nibble'ın 0. biti set ise PB4'ü HIGH yap
	if (nibble & 0x02) GPIOB->ODR |= GPIO_ODR_OD5; // Nibble'ın 1. biti set ise PB5'i HIGH yap
	if (nibble & 0x04) GPIOB->ODR |= GPIO_ODR_OD6; // Nibble'ın 2. biti set ise PB6'yı HIGH yap
    if (nibble & 0x08) GPIOB->ODR |= GPIO_ODR_OD7; // Nibble'ın 3. biti set ise PB7'yi HIGH yap

    DWT_Delay_us(100); // Gecikme

    // E pinine darbe gönder (HIGH-LOW)
    GPIOA->ODR |= GPIO_ODR_OD3;  // E pinini HIGH yap
    DWT_Delay_us(100);
    GPIOA->ODR &= ~GPIO_ODR_OD3; // E pinini LOW yap
    DWT_Delay_us(100); // Gecikme

}

/*

Bu fonksiyonun amacı: LCD’ye 4-bitlik veri (nibble) göndermektir.

GPIOA->ODR &= ~GPIO_ODR_OD3;
	Önce Enable (E) pini LOW yapılır. LCD’ye veri yazmadan önce E pininin kapalı olması gerekir.

GPIOB->ODR &= ~(GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD7);
	Veri pinleri sıfırlanır.

Daha sonra nibble’ın bitleri tek tek kontrol edilir:

	if (nibble & 0x01) GPIOB->ODR |= GPIO_ODR_OD4;
		Eğer nibble’ın 0. biti (LSB) 1 ise PB4 HIGH yapılır.

	if (nibble & 0x02) GPIOB->ODR |= GPIO_ODR_OD5;
		Eğer nibble’ın 1. biti 1 ise PB5 HIGH yapılır.

	if (nibble & 0x04) GPIOB->ODR |= GPIO_ODR_OD6;
		Eğer nibble’ın 2. biti 1 ise PB6 HIGH yapılır.

	if (nibble & 0x08) GPIOB->ODR |= GPIO_ODR_OD7;
		Eğer nibble’ın 3. biti 1 ise PB7 HIGH yapılır.

DWT_Delay_us(100);

	Veri pinleri ayarlandıktan sonra küçük bir gecikme verilir (LCD bu süre içinde girişleri algılar).

Enable darbesi (tetikleme):
	GPIOA->ODR |= GPIO_ODR_OD3; → E pini HIGH yapılır.
	DWT_Delay_us(100); → Kısa süre beklenir.
	GPIOA->ODR &= ~GPIO_ODR_OD3; → E pini tekrar LOW yapılır.
	Bu işlem LCD’nin gönderilen 4-bit veriyi “okumasını” sağlar.

Son bir küçük DWT_Delay_us(100); beklenir.

Özet: lcd_send_nibble fonksiyonu, parametreyle gelen 4 bitlik değeri PB4–PB7 pinlerine yazar ve Enable pinine darbe göndererek LCD’nin bu 4 biti okumasını sağlar.

*/

void lcd_send_command(uint8_t command)
{
    // RS (Register Select) pinini LOW yaparak komut modunu seç
    // GPIOA->ODR &= ~GPIO_ODR_ODR1; // RS pinini LOW yap
    GPIOA->BSRR = (1 << (1 + 16)); // GPIOA Pin 1'i LOW yapmak için BSRR kullanmak daha temiz bir yöntemdir.

    lcd_send_nibble(command >> 4);	// Komutun yüksek 4 bitini gönder

    lcd_send_nibble(command & 0x0F);	// Komutun düşük 4 bitini gönder

    // Komut sonrası bekleme süresi
    // Bazı komutlar (Clear Display gibi) daha uzun bekleme gerektirebilir.
    // Datasheet'e göre 1.52ms (veya daha fazla) bekleme önerilir.
    // Bu yüzden burada yaklaşık 2ms bekleme ekleyelim.
    delay_ms(2);
}

/*

Bu fonksiyon LCD’ye komut göndermek için kullanılır (örneğin ekran temizleme, imleci kaydırma, mod ayarı).

Açıklamalar:
	GPIO_ResetBits(GPIOx, RS_PIN) → RS=0 yapılır, yani LCD’ye “bu bir komut” denir.
	cmd >> 4 → Komutun yüksek 4 bitini çıkarır, LCD’ye gönderir.
	cmd & 0x0F → Komutun düşük 4 bitini çıkarır, ikinci adımda gönderir.
	delay_us(40) → LCD komutu işleyene kadar bekleme süresi (her komut en az 37 µs sürer).
	Özel durum: 0x01 (clear display) veya 0x02 (return home) komutları 1.52 ms sürer, bu durumda daha uzun bekleme yapılır.

*/

void lcd_send_data(uint8_t data)
{
    // RS (Register Select) pinini HIGH yaparak veri modunu seç
    // GPIOA->ODR |= GPIO_ODR_ODR1; // RS pinini HIGH yap
    GPIOA->BSRR = (1 << 1); // BSRR ile Pin 1'i HIGH yapmak daha temiz bir yöntemdir.

    lcd_send_nibble(data >> 4);	// Verinin yüksek 4 bitini gönder

    lcd_send_nibble(data & 0x0F);	// Verinin düşük 4 bitini gönder

    DWT_Delay_us(100);
}

/*

Bu fonksiyon LCD’ye ekrana yazılacak karakteri göndermek için kullanılır (örneğin 'A' karakterini yazdırmak).

Açıklamalar:
	GPIO_SetBits(GPIOx, RS_PIN) → RS=1 yapılır, yani LCD’ye “bu bir karakter” denir.
	data >> 4 → Karakterin ASCII kodunun üst 4 bitini gönder.
	data & 0x0F → Alt 4 bitini gönder.
	delay_us(40) → Karakter işleme süresi (37 µs) için bekleme yapılır.

Örnek Çalışma Senaryosu
	Diyelim ki LCD’ye 'A' karakterini göndermek istiyoruz:
	'A' ASCII’de 0x41 (0100 0001).
	lcd_send_data(0x41) çağrılır.
		RS = 1 yapılır → bu bir veri.
		İlk nibble: 0100 → LCD’ye gönderilir.
		İkinci nibble: 0001 → LCD’ye gönderilir.
	LCD 'A' karakterini ekrana basar.

*/

void lcd_init(void)
{

    lcd_gpio_config(); // GPIO pinlerini yapılandır

    delay_ms(20);	// Güç açılışından sonra stabilizasyon için bekleme

    lcd_send_command_nibble_only(0x03); // Sadece 4 bitlik komut gönderme. 4-bit modda başlatma komut dizisi
    delay_ms(5);

    lcd_send_command_nibble_only(0x03);	// İkinci reset komutu
    DWT_Delay_us(150);

    lcd_send_command_nibble_only(0x03);	// Üçüncü reset komutu
    DWT_Delay_us(150);

    lcd_send_command_nibble_only(0x02); // 4-bit modunu etkinleştir
    DWT_Delay_us(100);

    // Şimdi normal 8-bit komutlar kullanılabilir
    lcd_send_command(0x28); // 4-bit arayüz, 2 satır, 5x8 font
    lcd_send_command(0x0C); // Display ON, Cursor OFF, Blink OFF
    lcd_send_command(0x06); // Entry Mode: Increment, no shift
    lcd_send_command(0x01); // Ekranı temizle
    delay_ms(2);
}

/*

Bu fonksiyon, LCD açıldığında doğru şekilde çalışabilmesi için yapılan ilk ayarlamaları içerir.
Çünkü LCD güç verildiğinde rastgele bir durumda başlar, bizim onu 4-bit mod ve uygun çalışma moduna almamız gerekir.

Açıklamalar:

	delay_ms(40)
	→ LCD açıldığında ilk 30–40 ms boyunca komut kabul etmez, bu yüzden bekleme zorunludur.

	lcd_send_command_nibble_only(0x03) üç kez gönderilir
	→ LCD başlangıçta 8-bit modda olduğunu varsayar. 3 komutu üst üste göndererek LCD’ye "ben 8-bit moddayım" mesajı verilir.

	lcd_send_command_nibble_only(0x02)
	→ LCD’ye "4-bit mod kullanacağım" denir. Artık komut ve veri gönderimi 4-bitlik iki parça halinde yapılacaktır.

	lcd_send_command(0x28)
	→ Function set: 4-bit mod, 2 satır, 5x8 karakter boyutu.

	lcd_send_command(0x0C)
	→ Display control: ekran açık, imleç kapalı, yanıp sönme kapalı.

	lcd_send_command(0x06)
	→ Entry mode: karakter yazıldıkça imleç sağa kayar.

	lcd_send_command(0x01)
	→ Clear display (ekran temizlenir, imleç 0. satıra döner).

Bu dizilim HD44780 LCD datasheet’inde önerilen initialize sequence’tir.

*/

// Yüksek 4 bitlik veriyi gönderen özel bir komut fonksiyonu
// Başlatma sırasında sadece 4 bit gönderilmesi gerektiği için kullanılır.
void lcd_send_command_nibble_only(uint8_t nibble)
{
    // RS pinini LOW yap
    GPIOA->BSRR = (1 << (1 + 16));

    // Sadece yüksek 4 bit gönder
    lcd_send_nibble(nibble);
}

/*

Bu fonksiyon, sadece 4-bit (nibble) komut gönderimi yapar. Yani LCD’yi başlatırken kullanılan özel bir fonksiyondur.

Açıklamalar:
	Burada sadece tek bir nibble gönderilir (çünkü LCD daha 4-bit moda alınmadı).
	Başlangıçta LCD 8-bit modda varsayılır, ama biz sadece 4 pin bağlıyız.
	Bu nedenle ilk komutları yarım göndermemiz gerekir. Datasheet bu yöntemi tanımlar:
		3 gönder → "ben 8-bit moddayım" mesajı
		3 gönder
		3 gönder
		2 gönder → "artık 4-bit moda geçiyorum"

Bu adımlar bittiğinde LCD artık 4-bit modda çalışmaya hazır olur. Sonrasında normal lcd_send_command() ve lcd_send_data() kullanılmaya başlanır.

*/

void lcd_print_string(const char* str)
{
    // Dizinin sonuna (null karakter) ulaşana kadar döngüyü sürdür
    while (*str) {
        // Her bir karakteri LCD'ye gönder
        lcd_send_data(*str++);
    }
}

/*

Bu fonksiyon bir C stringini (\0 ile biten karakter dizisi) ekrana yazar.

Açıklama:
	*str → stringdeki karakteri alır.
	lcd_send_data() → her karakteri LCD’ye yollar.
	Örneğin "HELLO" gönderirseniz, sırasıyla 'H' → 'E' → 'L' → 'L' → 'O' LCD'ye basılır.

👉 Bu fonksiyon olmadan, tek tek lcd_send_data('H'); lcd_send_data('E'); ... yazmanız gerekirdi.

*/

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t address;

    // Satıra göre başlangıç adresini belirle
    if (row == 0) {
        address = 0x80 + col;
    } else {
        address = 0xC0 + col;
    }

    // Komutu gönder
    lcd_send_command(address);
}

/*

Bu fonksiyon LCD imlecini istenilen satır ve sütuna taşır.

Açıklama:
	LCD’nin belleği DDRAM (Display Data RAM) ile adreslenir.
	İlk satırın başlangıç adresi 0x00, ikinci satırın 0x40.
	Komut formatı: 0b1xxxxxxx (0x80 + DDRAM adresi) → cursor’u belirtilen konuma götürür.
	Örnek: lcd_set_cursor(1, 5); → imleci 2. satır, 6. sütuna götürür.

*/

void lcd_clear(void)
{
    // Clear Display komutunu gönder
    lcd_send_command(0x01);
    // Bu komutun tamamlanması için uzun bir gecikme gerekir (datasheet'e göre > 1.52 ms)
    delay_ms(2);
}

/*

Bu fonksiyon ekranı temizler ve imleci en başa (0,0) konumuna getirir.

Açıklama:
	0x01 komutu → Clear Display
	Bütün DDRAM temizlenir, ekran boşalır.
	Cursor (imleç) sol üst köşeye (row=0, col=0) taşınır.
	Bu komut normalden uzun sürer, bu yüzden ekstra gecikme gerekir (datasheet: ~1.64 ms)

*/

//------------------------------------------------------------------------------------------------------------------------------

/*

LCD 16x2’nin Çalışma Mantığı

	Veri Yolu (8-bit veya 4-bit mod)
		LCD aslında 8-bitlik bir veri yolu ile çalışır (D0–D7).
		Fakat donanımı azaltmak için genellikle 4-bit mod kullanılır (D4–D7).
		4-bit modda:
			İlk önce gönderilen 8 bitin üst 4 biti (nibble) yazılır,
			Sonra alt 4 bit yazılır.
		Böylece LCD her komutu veya karakteri iki aşamada alır.

	Kontrol Pinleri
		RS (Register Select):
			RS=0 → Komut (ör. ekran temizle, imleci kaydır vb.)
			RS=1 → Veri (ör. ekrana yazılacak ASCII karakter)
		E (Enable):
			Veri pinlerindeki bilgiyi LCD’nin okuması için darbe (HIGH → LOW) gönderilir.
		R/W (Read/Write):
			Genelde GND’ye bağlanır çünkü sadece yazma (Write) kullanılır.

	Çalışma Sırası (Bir veri gönderme adımları)
		RS pinine komut mu, veri mi gönderileceği belirlenir.
		4-bitlik veri PB4–PB7 pinlerine yazılır.
		E pinine kısa süre HIGH → LOW darbesi uygulanır.
		LCD bu sırada veriyi okur ve işler.

	Zamanlama (Timing)
		LCD çok hızlı değildir. Komut veya veri gönderdikten sonra us seviyesinde bekleme gerekir.
		Örneğin:
			clear display (0x01) komutu ≈ 1.52 ms sürer.
			Normal veri yazımı ≈ 37 µs sürer.

Fonksiyonlarla Bağlantısı

	lcd_gpio_config()
		LCD’nin RS, E ve D4–D7 pinlerini çıkış yapar. Çünkü mikrodenetleyici sürekli LCD’ye yazacak.

	lcd_send_nibble()
		Verilen 4-bit değeri D4–D7 pinlerine koyar.
		Sonra E pinine darbe göndererek LCD’ye “oku artık” der.
		Bu işlem bir komutun veya bir karakterin yarısıdır (çünkü 4 bit gönderildi).

	lcd_send_command() (ileride göreceğiz)
		RS = 0 yapılır → LCD’ye bu bir komut denir.
		Sonra komutun üst nibble’ı gönderilir, ardından alt nibble’ı gönderilir.

	lcd_send_data() (ileride göreceğiz)
		RS = 1 yapılır → LCD’ye bu bir veri (karakter) denir.
		Yine üst nibble ve alt nibble sırayla gönderilir.

Özetle: LCD’nin mantığı RS ile işin komut mu veri mi olduğunu seçmek, D4–D7’den nibble göndermek, E ile darbe verip okutmak üzerine kurulu.
Tüm fonksiyonlar bu temel işleyiş etrafında yazılmış.

*/
