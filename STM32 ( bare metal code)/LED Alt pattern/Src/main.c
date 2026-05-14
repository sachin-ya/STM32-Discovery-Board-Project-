#include <stdint.h>

//// RCC (Reset and Clock Control) ////
#define RCC_BASE        0x40023800
#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30)) // Enables clock for GPIO ports
#define RCC_APB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x40)) // Enables clock for TIM2

//// GPIOD ////
#define GPIOD_BASE      0x40020C00
#define GPIOD_MODER     (*(volatile uint32_t*)(GPIOD_BASE + 0x00)) // Set pin mode (input/output)
#define GPIOD_OTYPER    (*(volatile uint32_t*)(GPIOD_BASE + 0x04)) // Output type (push-pull/open-drain)
#define GPIOD_OSPEEDR   (*(volatile uint32_t*)(GPIOD_BASE + 0x08)) // Output speed
#define GPIOD_ODR       (*(volatile uint32_t*)(GPIOD_BASE + 0x14)) // Output data register (write HIGH/LOW)

//// TIM2 ////
#define TIM2_BASE       0x40000000
#define TIM2_CR1        (*(volatile uint32_t*)(TIM2_BASE + 0x00)) // Control register (start/stop timer)
#define TIM2_SR         (*(volatile uint32_t*)(TIM2_BASE + 0x10)) // Status register (UIF flag)
#define TIM2_CNT        (*(volatile uint32_t*)(TIM2_BASE + 0x24)) // Counter value
#define TIM2_PSC        (*(volatile uint32_t*)(TIM2_BASE + 0x28)) // Prescaler (controls speed)
#define TIM2_ARR        (*(volatile uint32_t*)(TIM2_BASE + 0x2C)) // Auto-reload (max count value)


// Function declarations
void gpio_init(void);
void tim2_init(void);
void delay(void);


// GPIO initialization
void gpio_init(void)
{
    GPIOD_MODER &= ~(3 << (12*2));  // Clear mode bits for pin 12
    GPIOD_MODER |=  (1 << (12*2));  // Set pin 12 as OUTPUT mode

    GPIOD_MODER &= ~(3 << (13*2));  // Clear mode bits for pin 13
    GPIOD_MODER |=  (1 << (13*2));  // Set pin 13 as OUTPUT mode

    GPIOD_MODER &= ~(3 << (14*2));  // Clear mode bits for pin 14
    GPIOD_MODER |=  (1 << (14*2));  // Set pin 14 as OUTPUT mode

    GPIOD_MODER &= ~(3 << (15*2));  // Clear mode bits for pin 15
    GPIOD_MODER |=  (1 << (15*2));  // Set pin 15 as OUTPUT mode

    GPIOD_OTYPER &= ~(1 << 12);     // Set output type as push-pull (strong drive)
    GPIOD_OTYPER &= ~(1 << 13);     // Set output type as push-pull (strong drive)
    GPIOD_OTYPER &= ~(1 << 14);     // Set output type as push-pull (strong drive)
    GPIOD_OTYPER &= ~(1 << 15);     // Set output type as push-pull (strong drive)

    GPIOD_OSPEEDR |= (3 << (12*2)); // Set high speed for better switching
    GPIOD_OSPEEDR |= (3 << (13*2)); // Set high speed for better switching
    GPIOD_OSPEEDR |= (3 << (14*2)); // Set high speed for better switching
    GPIOD_OSPEEDR |= (3 << (15*2)); // Set high speed for better switching
}


// Timer initialization
void tim2_init(void)
{
    TIM2_CR1 &= ~(1 << 0);  // Stop timer before configuration (safe practice)

    TIM2_PSC = 16000 - 1;   // Divide 16 MHz clock → 1 kHz (1 ms per count)
    // WHY: Makes timer easy to understand (1 count = 1 ms)

    TIM2_ARR = 500 - 1;     // Timer overflows after 500 counts → 500 ms delay
    // WHY: Defines total delay time

    TIM2_CNT = 0;           // Reset counter to start from 0
    // WHY: Ensures correct timing from beginning

    TIM2_SR &= ~(1 << 0);   // Clear update flag (UIF)
    // WHY: Prevents false trigger from previous state

    TIM2_CR1 |= (1 << 0);   // Start timer
    // WHY: Enables counting
}


// Delay function using timer
void delay(void)
{
    TIM2_CNT = 0;                     // Reset counter for fresh timing
    // WHY: Ensures same delay every loop

    while (!(TIM2_SR & (1 << 0)));   // Wait until UIF flag is set
    // WHY: Timer sets UIF when it reaches ARR (time completed)

    TIM2_SR &= ~(1 << 0);            // Clear UIF flag
    // WHY: Prepare for next delay cycle
}


// Main function
int main(void)
{
    RCC_AHB1ENR |= 1<<3; // Enable clock for GPIOD
    // WHY: GPIO won't work without clock

    RCC_APB1ENR |= 1<<0; // Enable clock for TIM2
    // WHY: Timer won't run without clock

    gpio_init(); // Configure LED pin
    tim2_init(); // Configure timer

    while (1)
    {
    	GPIOD_ODR ^= ((1<<12) | (1<<13) | (1<<14) | (1<<15));
    	// Toggle multiple LEDs (PD12–PD15) in a single operation
    	// WHY: XOR flips each bit → if LED was ON it becomes OFF, if OFF it becomes ON
    	// WHY: Using OR (|) combines multiple pins into one mask
    	// WHY: Single register access → faster and more efficient than multiple lines
    	// WHY: All LEDs appear to change simultaneously (minimal delay between operations)

    	delay();
    	// WHY: Wait 500 ms (as configured in TIM2)
    	// WHY: Without delay, LEDs will toggle too fast and appear always ON/OFF
    	// WHY: Creates visible blinking effect for human eyes
    }
}
