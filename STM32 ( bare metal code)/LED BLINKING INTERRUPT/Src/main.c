#include <stdint.h>

//// RCC (Reset and Clock Control) ////
#define RCC_BASE        0x40023800
#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30)) // Enable clock for GPIO
#define RCC_APB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x40)) // Enable clock for TIM2

//// GPIOD ////
#define GPIOD_BASE      0x40020C00
#define GPIOD_MODER     (*(volatile uint32_t*)(GPIOD_BASE + 0x00)) // Configure pin mode
#define GPIOD_OTYPER    (*(volatile uint32_t*)(GPIOD_BASE + 0x04)) // Output type
#define GPIOD_OSPEEDR   (*(volatile uint32_t*)(GPIOD_BASE + 0x08)) // Output speed
#define GPIOD_ODR       (*(volatile uint32_t*)(GPIOD_BASE + 0x14)) // Write HIGH/LOW to pins

//// TIM2 ////
#define TIM2_BASE       0x40000000
#define TIM2_CR1        (*(volatile uint32_t*)(TIM2_BASE + 0x00)) // Start/stop timer
#define TIM2_DIER       (*(volatile uint32_t*)(TIM2_BASE + 0x0C)) // Interrupt enable
#define TIM2_SR         (*(volatile uint32_t*)(TIM2_BASE + 0x10)) // Status register
#define TIM2_CNT        (*(volatile uint32_t*)(TIM2_BASE + 0x24)) // Counter value
#define TIM2_PSC        (*(volatile uint32_t*)(TIM2_BASE + 0x28)) // Prescaler
#define TIM2_ARR        (*(volatile uint32_t*)(TIM2_BASE + 0x2C)) // Auto reload

//// NVIC ////
#define NVIC_ISER0      (*(volatile uint32_t*)(0xE000E100))

// Function declarations
void gpio_init(void);
void tim2_init(void);

//// Global counters (WHY: used in ISR, so volatile required) ////
volatile uint16_t t1 = 0, t2 = 0, t3 = 0, t4 = 0;


// GPIO initialization
void gpio_init(void)
{
    // WHY: Set PD12–PD15 as OUTPUT (01)
    GPIOD_MODER &= ~((3<<(12*2)) | (3<<(13*2)) | (3<<(14*2)) | (3<<(15*2)));
    GPIOD_MODER |=  ((1<<(12*2)) | (1<<(13*2)) | (1<<(14*2)) | (1<<(15*2)));

    // WHY: Push-pull → strong output drive
    GPIOD_OTYPER &= ~((1<<12) | (1<<13) | (1<<14) | (1<<15));

    // WHY: High speed output
    GPIOD_OSPEEDR |= ((3<<(12*2)) | (3<<(13*2)) | (3<<(14*2)) | (3<<(15*2)));
}


// Timer initialization (Interrupt based)
void tim2_init(void)
{
    TIM2_CR1 &= ~(1<<0);   // WHY: Stop timer before config

    // WHY: 16 MHz / 16000 = 1 kHz → 1 ms tick
    TIM2_PSC = 16000 - 1;

    // WHY: Overflow every 1 ms
    TIM2_ARR = 1;

    TIM2_CNT = 0;          // WHY: Start from 0

    TIM2_SR &= ~(1<<0);    // WHY: Clear UIF flag

    TIM2_DIER |= (1<<0);   // WHY: Enable update interrupt

    NVIC_ISER0 |= (1<<28); // WHY: Enable TIM2 interrupt in NVIC

    TIM2_CR1 |= (1<<0);    // WHY: Start timer
}


// Interrupt Service Routine
void TIM2_IRQHandler(void)
{
    // WHY: Check interrupt flag
    if (TIM2_SR & (1<<0))
    {
        TIM2_SR &= ~(1<<0); // WHY: Clear flag

        // WHY: Increment counters every 1 ms
        t1++;
        t2++;
        t3++;
        t4++;

        // WHY: Toggle LEDs at different delays

        if (t1 >= 500)     // 500 ms
        {
            GPIOD_ODR ^= (1<<12);
            t1 = 0;
        }

        if (t2 >= 1000)    // 1 sec
        {
            GPIOD_ODR ^= (1<<13);
            t2 = 0;
        }

        if (t3 >= 2000)    // 2 sec
        {
            GPIOD_ODR ^= (1<<14);
            t3 = 0;
        }

        if (t4 >= 3000)    // 3 sec
        {
            GPIOD_ODR ^= (1<<15);
            t4 = 0;
        }
    }
}


// Main function
int main(void)
{
    RCC_AHB1ENR |= (1<<3); // WHY: Enable GPIOD clock
    RCC_APB1ENR |= (1<<0); // WHY: Enable TIM2 clock

    gpio_init(); // WHY: Configure LED pins
    tim2_init(); // WHY: Setup timer + interrupt


    while (1)
    {
        // WHY: No delay needed → all handled by interrupt
    }
}
