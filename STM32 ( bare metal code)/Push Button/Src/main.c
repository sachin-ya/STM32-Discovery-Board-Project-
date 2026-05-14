#include <stdint.h>

//// ================= RCC ================= ////
// WHY: Enable clock for GPIO, TIMER, SYSCFG

#define RCC_BASE        0x40023800
#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30)) // GPIO clock
#define RCC_APB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x40)) // TIM2 clock
#define RCC_APB2ENR     (*(volatile uint32_t*)(RCC_BASE + 0x44)) // SYSCFG clock


//// ================= GPIOD (LEDs) ================= ////
// WHY: PD12–PD15 are onboard LEDs

#define GPIOD_BASE      0x40020C00
#define GPIOD_MODER     (*(volatile uint32_t*)(GPIOD_BASE + 0x00)) // Mode
#define GPIOD_ODR       (*(volatile uint32_t*)(GPIOD_BASE + 0x14)) // Output


//// ================= GPIOA (BUTTON) ================= ////
// WHY: PA0 = User button

#define GPIOA_BASE      0x40020000
#define GPIOA_MODER     (*(volatile uint32_t*)(GPIOA_BASE + 0x00))


//// ================= SYSCFG ================= ////
// WHY: Connect PA0 to EXTI0

#define SYSCFG_BASE     0x40013800
#define SYSCFG_EXTICR1  (*(volatile uint32_t*)(SYSCFG_BASE + 0x08))


//// ================= EXTI ================= ////
// WHY: Detect button press and generate interrupt

#define EXTI_BASE       0x40013C00
#define EXTI_IMR        (*(volatile uint32_t*)(EXTI_BASE + 0x00))
#define EXTI_RTSR       (*(volatile uint32_t*)(EXTI_BASE + 0x08))
#define EXTI_PR         (*(volatile uint32_t*)(EXTI_BASE + 0x14))


//// ================= TIM2 ================= ////
// WHY: Create delay for debounce + visible blink

#define TIM2_BASE       0x40000000
#define TIM2_CR1        (*(volatile uint32_t*)(TIM2_BASE + 0x00))
#define TIM2_SR         (*(volatile uint32_t*)(TIM2_BASE + 0x10))
#define TIM2_CNT        (*(volatile uint32_t*)(TIM2_BASE + 0x24))
#define TIM2_PSC        (*(volatile uint32_t*)(TIM2_BASE + 0x28))
#define TIM2_ARR        (*(volatile uint32_t*)(TIM2_BASE + 0x2C))


//// ================= NVIC ================= ////
// WHY: Allow CPU to execute interrupt

#define NVIC_ISER0      (*(volatile uint32_t*)(0xE000E100))


// Function prototypes
void gpio_init(void);
void tim2_init(void);
void exti_init(void);
void delay(void);



// ================= GPIO INIT =================
void gpio_init(void)
{
    // -------- PD12–PD15 OUTPUT --------
    // WHY: LEDs need output mode

    GPIOD_MODER &= ~(3<<(12*2));
    GPIOD_MODER |=  (1<<(12*2));

    GPIOD_MODER &= ~(3<<(13*2));
    GPIOD_MODER |=  (1<<(13*2));

    GPIOD_MODER &= ~(3<<(14*2));
    GPIOD_MODER |=  (1<<(14*2));

    GPIOD_MODER &= ~(3<<(15*2));
    GPIOD_MODER |=  (1<<(15*2));


    // -------- PA0 INPUT --------
    // WHY: Button gives input signal

    GPIOA_MODER &= ~(3<<(0*2));
}



// ================= TIMER INIT =================
void tim2_init(void)
{
    TIM2_CR1 &= ~(1<<0);   // WHY: Stop timer before setup

    TIM2_PSC = 16000 - 1;
    // WHY: 16 MHz → 1 ms tick

    TIM2_ARR = 300 - 1;
    // WHY: 300 ms delay (visible blink)

    TIM2_CNT = 0;
    // WHY: Start from zero

    TIM2_SR &= ~(1<<0);
    // WHY: Clear overflow flag

    TIM2_CR1 |= (1<<0);
    // WHY: Start timer
}



// ================= DELAY =================
void delay(void)
{
    TIM2_CNT = 0;                     // WHY: Restart timing

    while(!(TIM2_SR & (1<<0)));      // WHY: Wait for overflow

    TIM2_SR &= ~(1<<0);              // WHY: Clear flag
}



// ================= EXTI INIT =================
void exti_init(void)
{
    RCC_APB2ENR |= (1<<14);
    // WHY: Enable SYSCFG

    SYSCFG_EXTICR1 &= ~(0xF<<0);
    // WHY: EXTI0 connected to PA0

    EXTI_IMR |= (1<<0);
    // WHY: Enable interrupt line

    EXTI_RTSR |= (1<<0);
    // WHY: Trigger on button press

    NVIC_ISER0 |= (1<<6);
    // WHY: Enable EXTI0 interrupt in CPU
}



// ================= INTERRUPT =================
void EXTI0_IRQHandler(void)
{
    if(EXTI_PR & (1<<0))   // WHY: Confirm EXTI0 interrupt
    {
        delay();
        // WHY: Debounce + visible delay

        // -------- Toggle ALL LEDs --------
        GPIOD_ODR ^= (1<<12); // WHY: Toggle LED1
        GPIOD_ODR ^= (1<<13); // WHY: Toggle LED2
        GPIOD_ODR ^= (1<<14); // WHY: Toggle LED3
        GPIOD_ODR ^= (1<<15); // WHY: Toggle LED4

        EXTI_PR |= (1<<0);
        // WHY: Clear interrupt flag
    }
}



// ================= MAIN =================
int main(void)
{
    RCC_AHB1ENR |= (1<<0); // WHY: Enable GPIOA
    RCC_AHB1ENR |= (1<<3); // WHY: Enable GPIOD
    RCC_APB1ENR |= (1<<0); // WHY: Enable TIM2

    gpio_init();
    tim2_init();
    exti_init();

    while(1)
    {
        // WHY: System waits for button interrupt
    }
}
