#include <stdint.h>

//-------------------------
// 1. Peripheral Base Addresses
//-------------------------
#define PERIPH_BASE           0x40000000UL
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000UL)
#define RCC_BASE              (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOD_BASE            (AHB1PERIPH_BASE + 0x0C00UL)

//-------------------------
// 2. Register Addresses
//-------------------------
#define RCC_AHB1ENR           (*((volatile uint32_t *)(RCC_BASE + 0x30)))
#define GPIOD_MODER           (*((volatile uint32_t *)(GPIOD_BASE + 0x00)))
#define GPIOD_ODR             (*((volatile uint32_t *)(GPIOD_BASE + 0x14)))

//-------------------------
// 3. Bit Masks
//-------------------------
#define RCC_GPIOD_EN          (1 << 3)  // Enable GPIOD clock

//-------------------------
// 4. Segment codes for 0–9 (Common Cathode)
//-------------------------
uint8_t digit_code[10] = {
    0x3F,  // 0 = A+B+C+D+E+F
    0x06,  // 1 = B+C
    0x5B,  // 2 = A+B+D+E+G
    0x4F,  // 3 = A+B+C+D+G
    0x66,  // 4 = B+C+F+G
    0x6D,  // 5 = A+C+D+F+G
    0x7D,  // 6 = A+C+D+E+F+G
    0x07,  // 7 = A+B+C
    0x7F,  // 8 = A+B+C+D+E+F+G
    0x6F   // 9 = A+B+C+D+F+G
};

//-------------------------
// 5. Delay Function
//-------------------------
void delay(volatile uint32_t time) {
    while (time--);
}

//-------------------------
// 6. Main Function
//-------------------------
int main(void) {
    // Enable GPIOD peripheral clock
    RCC_AHB1ENR |= RCC_GPIOD_EN;

    // Set PD0–PD7 as general purpose output mode (MODER = 01)
    GPIOD_MODER &= ~(0xFFFF);   // Clear MODER bits for PD0–PD7 (2 bits per pin)
    GPIOD_MODER |=  (0x5555);   // Set MODER = 01 (output) for PD0–PD7

    while (1) {
        for (int i = 0; i < 10; i++) {
            GPIOD_ODR &= ~(0xFF);         // Clear lower 8 bits (PD0–PD7)
            GPIOD_ODR |= digit_code[i];   // Send digit pattern
            delay(1000000);               // Simple delay
        }
    }
}
