#ifndef __DELAY__     // Header dosyasının birden fazla kez include edilmesini engellemek için koruma tanımı başlatılır
#define __DELAY__

void systick_config(void);     // SysTick yapılandırma fonksiyonu (1ms tabanlı delay için)
void SysTick_Handler(void);    // SysTick kesme fonksiyonu prototipi
void delay_ms(uint32_t ms);    // Milisaniye cinsinden gecikme fonksiyonu

uint32_t DWT_Delay_Init(void); // DWT modülünü başlatan fonksiyon (mikrosaniye delay için kullanılacak)


__STATIC_INLINE void DWT_Delay_us(volatile uint32_t microseconds)   // Inline fonksiyon: mikro saniye gecikme yapar
{
  uint32_t clk_cycle_start = DWT->CYCCNT;                          // Başlangıç cycle değerini al
  microseconds *= (72000000 / 1000000);                            // 72 MHz -> 1 µs = 72 clock cycle, çevrim sayısına çevir
  while ((DWT->CYCCNT - clk_cycle_start) < microseconds);          // İstenen süre dolana kadar bekle (busy-wait döngüsü)
}

#endif  // __DELAY__   // Header guard bitişi
