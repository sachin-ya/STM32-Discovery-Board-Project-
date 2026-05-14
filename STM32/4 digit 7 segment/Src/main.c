#include <stdint.h>

// ------------------ Base Addresses ------------------
#define RCC_BASE        0x40023800
#define GPIOA_BASE      0x40020000
#define GPIOD_BASE      0x40020C00

// ------------------ RCC Registers -------------------
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))

// ------------------ GPIOA Registers -----------------
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

// ------------------ GPIOD Registers -----------------
#define GPIOD_MODER     (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR       (*(volatile uint32_t *)(GPIOD_BASE + 0x14))

// ------------------ Segment Pattern -----------------
const uint8_t segment_map[10] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

// ------------------ Delay Function ------------------
void delay_ms(volatile uint32_t ms) {
    for (; ms > 0; ms--) {
        for (volatile uint32_t i = 0; i < 1600; i++); // Approximate
    }
}

// ------------------ Display Function ----------------
void display_number(uint16_t number) {
    uint8_t digits[4];
    digits[0] = number % 10;              // Units
    digits[1] = (number / 10) % 10;       // Tens
    digits[2] = (number / 100) % 10;      // Hundreds
    digits[3] = (number / 1000) % 10;     // Thousands

    for (uint8_t i = 0; i < 4; i++) {
        // Output segment pattern on PA0-PA7
        GPIOA_ODR = (GPIOA_ODR & ~0xFF) | segment_map[digits[i]];

        // Activate one digit at a time via PD0-PD3
        GPIOD_ODR = (1 << i);  // Activate current digit
        delay_ms(2);           // Small delay for multiplexing
        GPIOD_ODR = 0x00;      // Turn off digit
    }
}

// ------------------ Main Function -------------------
int main(void) {
    // 1. Enable GPIOA and GPIOD clock
    RCC_AHB1ENR |= (1 << 0);  // GPIOA
    RCC_AHB1ENR |= (1 << 3);  // GPIOD

    // 2. Set GPIOA (PA0-PA7) to output
    GPIOA_MODER &= ~(0xFFFF);       // Clear bits
    GPIOA_MODER |=  (0x5555);       // Set PA0-PA7 as output (01)

    // 3. Set GPIOD (PD0-PD3) to output
    GPIOD_MODER &= ~(0xFF);         // Clear bits
    GPIOD_MODER |=  (0x55);         // Set PD0-PD3 as output (01)

    // 4. Main loop
    uint16_t count = 0;
    while (1) {
        for (uint16_t i = 0; i <= 9999; i++) {
            for (uint16_t j = 0; j < 100; j++) {  // Refresh display ~100 times per number
                display_number(i);
            }
        }
    }
}
