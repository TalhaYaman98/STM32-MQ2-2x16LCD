
#ifndef __LCD__               // LCD kütüphanesi için header guard başlangıcı
#define __LCD__               // Tekrar eklenmeyi önlemek için tanımlama

#include "stm32f4xx.h"        // STM32F4 donanım tanımlarını içeren header

void lcd_gpio_config(void);                     // LCD için gerekli GPIO pinlerini ayarlar
void lcd_send_nibble(uint8_t nibble);           // LCD'ye 4-bit (nibble) veri yollar
void lcd_send_command(uint8_t command);         // LCD'ye komut gönderir (ör. clear, cursor ayarı)
void lcd_send_data(uint8_t data);               // LCD'ye veri (karakter) gönderir
void lcd_init(void);                            // LCD'yi başlatır (4-bit mod ayarı vs.)
void lcd_send_command_nibble_only(uint8_t nibble); // LCD’ye sadece nibble komutu gönderir (özel init için)
void lcd_print_string(const char* str);         // Stringi LCD’ye karakter karakter yazar
void lcd_clear(void);                           // LCD ekranını temizler ve imleci başa alır
void lcd_set_cursor(uint8_t row, uint8_t col);  // İmleci istenen satır ve sütuna taşır

#endif  // __LCD__            // Header guard bitişi
