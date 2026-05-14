#include <stdint.h>

// ---------------- BASE ADDRESSES ----------------
#define RCC_BASE        0x40023800UL
#define GPIOA_BASE      0x40020000UL
#define GPIOC_BASE      0x40020800UL
#define GPIOD_BASE      0x40020C00UL
#define ADC1_BASE       0x40012000UL

// ---------------- RCC REGISTERS ----------------
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x44))

// ---------------- GPIO REGISTERS ----------------
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))

#define GPIOC_MODER     (*(volatile uint32_t *)(GPIOC_BASE + 0x00))
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE + 0x14))

#define GPIOD_MODER     (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR       (*(volatile uint32_t *)(GPIOD_BASE + 0x14))

// ---------------- ADC1 REGISTERS ----------------
#define ADC1_SR         (*(volatile uint32_t *)(ADC1_BASE + 0x00))
#define ADC1_CR2        (*(volatile uint32_t *)(ADC1_BASE + 0x08))
#define ADC1_SQR3       (*(volatile uint32_t *)(ADC1_BASE + 0x34))
#define ADC1_DR         (*(volatile uint32_t *)(ADC1_BASE + 0x4C))

// ---------------- SEGMENT TABLE ----------------
// Common Cathode, Active HIGH
uint8_t seg_table[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

char display_buf[4];

// ---------------- INIT FUNCTIONS ----------------
void GPIO_Init(void) {
    // Enable GPIOC, GPIOD clock
    RCC_AHB1ENR |= (1<<2) | (1<<3);

    // PD0-PD7 output (segments)
    GPIOD_MODER &= ~(0xFFFF);   // clear PD0–PD7 (bits 0–15)
    GPIOD_MODER |=  0x5555;     // set output mode (01 per pin)

    // PC0-PC3 output (digit select)
    GPIOC_MODER &= ~(0xFF);     // clear PC0–PC3 (bits 0–7)
    GPIOC_MODER |=  0x55;       // set output mode (01 per pin)
}

void ADC1_Init(void) {
    // Enable ADC1 + GPIOA clock
    RCC_APB2ENR |= (1<<8);   // ADC1EN
    RCC_AHB1ENR |= (1<<0);   // GPIOAEN

    // PA0 analog mode
    GPIOA_MODER &= ~(0x3 << 0);   // clear bits for PA0
    GPIOA_MODER |=  (0x3 << 0);   // set to 11 (analog)

    // ADC1 config
    ADC1_CR2   = 0x00000000;      // reset control register
    ADC1_SQR3  = 0x00000000;      // channel 0 selected
    ADC1_CR2  |= (1<<0);          // ADON = enable
}

uint16_t ADC1_Read(void) {
    ADC1_CR2 |= (1<<30);                 // Start conversion (SWSTART)
    while(!(ADC1_SR & (1<<1)));          // Wait for EOC (bit1)
    return (uint16_t)ADC1_DR;            // Read result
}

// ---------------- DISPLAY ----------------
void Display_Digit(int digit, int pos) {
    GPIOC_ODR = (1<<pos);          // ✅ enable selected digit (active HIGH via BC547)
    GPIOD_ODR = seg_table[digit];  // send segment data
    for(volatile int d=0; d<3000; d++); // small delay
    GPIOC_ODR = 0x00;              // ✅ disable all digits
}

void Update_DisplayBuffer(uint16_t adc) {
    display_buf[3] = (adc/1000)%10;  // D4 (MSB, leftmost)
    display_buf[2] = (adc/100)%10;   // D3
    display_buf[1] = (adc/10)%10;    // D2
    display_buf[0] = adc%10;         // D1 (LSB, rightmost)
}

// ---------------- MAIN ----------------
int main(void) {
    GPIO_Init();
    ADC1_Init();

    uint16_t adc_value;

    while(1) {
        adc_value = ADC1_Read();           // Read ADC
        Update_DisplayBuffer(adc_value);   // Convert to 4 digits

        // Multiplex 4 digits
        Display_Digit(display_buf[0],0);   // D4
        Display_Digit(display_buf[1],1);   // D3
        Display_Digit(display_buf[2],2);   // D2
        Display_Digit(display_buf[3],3);   // D1
    }
}
