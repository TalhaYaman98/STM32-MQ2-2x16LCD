#include "stm32f4xx.h"      // STM32F4 serisi için temel CMSIS tanımları
#include "delay.h"          // delay fonksiyonlarının başlık dosyası

#define SYSTICK_FREQ_HZ 1000  // SysTick kesme frekansı (1000 Hz = 1 ms periyot)
uint32_t SystemCoreClock = 72000000;  // Sistem saat frekansı 72 MHz
static volatile uint32_t systick_counter;  // 1 ms kesmelerde azalacak sayaç

void systick_config(void)
{
    // Reload değeri = (Saat Frekansı / Kesme Frekansı) - 1
    SysTick->LOAD = (SystemCoreClock / SYSTICK_FREQ_HZ) - 1;   // 72000000 / 1000 - 1 = 71999

    SysTick->VAL = 0;   // Sayaç başlangıç değerini sıfırla

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |   // İşlemci saatini (AHB clock = 72 MHz) kullan
                    SysTick_CTRL_TICKINT_Msk   |   // SysTick kesmesini etkinleştir
                    SysTick_CTRL_ENABLE_Msk;       // SysTick sayacını başlat
}

void SysTick_Handler(void)
{
    // SysTick kesmesi her 1 ms’de bir çalışır
    if (systick_counter > 0)     // Sayaç 0’dan büyükse azalt
    {
        systick_counter--;       // ms bazlı bekleme için sayaç bir azalır
    }
}

void delay_ms(uint32_t ms)
{
    systick_counter = ms;                 // Bekleme süresini global sayaç değişkenine yükle
    while (systick_counter != 0)          // Sayaç 0 olana kadar döngüde kal
    {
        // Döngü içinde bekleme yapılır, kesmeler sayaç değerini azaltır
    }
}

// Mikro-saniye gecikme için DWT (Data Watchpoint and Trace) yapılandırması
uint32_t DWT_Delay_Init(void)
{
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;  // Trace özelliğini önce temizle
    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk;  // Trace özelliğini etkinleştir

    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;  // Cycle counter’ı kapat
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk;  // Cycle counter’ı aç

    DWT->CYCCNT = 0;   // Cycle counter başlangıç değerini sıfırla

    __ASM volatile ("NOP");   // Pipeline boşaltmak için 3 NOP
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");

    if (DWT->CYCCNT)   // Sayaç çalışmaya başladıysa
    {
        return 0;      // Başarılı
    }
    else
    {
        return 1;      // Başarısız
    }
}
