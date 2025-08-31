#ifndef __MQ2__     // Header dosyasının birden fazla kez include edilmesini engellemek için koruma tanımı başlatılır
#define __MQ2__

void gpio_pa0_analog_init(void);
void adc1_init(void);
uint16_t adc1_read(void);

#endif  // __MQ2__   // Header guard bitişi


