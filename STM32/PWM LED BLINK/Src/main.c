#include <stdint.h>

//// ================= RCC (CLOCK ENABLE) ================= ////
// WHY: Without clock, peripherals (GPIO, TIMER) will not work

#define RCC_BASE        0x40023800
#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30)) // GPIO clock enable
#define RCC_APB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x40)) // TIM4 clock enable


//// ================= GPIOD (LED PINS) ================= ////
// WHY: PD12–PD15 are connected to LEDs and TIM4 PWM channels

#define GPIOD_BASE      0x40020C00
#define GPIOD_MODER     (*(volatile uint32_t*)(GPIOD_BASE + 0x00)) // Mode register
#define GPIOD_AFRH      (*(volatile uint32_t*)(GPIOD_BASE + 0x24)) // Alternate function register


//// ================= TIM4 (PWM TIMER) ================= ////
// WHY: TIM4 generates PWM signals

#define TIM4_BASE       0x40000800
#define TIM4_CR1        (*(volatile uint32_t*)(TIM4_BASE + 0x00)) // Control register
#define TIM4_CCMR1      (*(volatile uint32_t*)(TIM4_BASE + 0x18)) // CH1, CH2 config
#define TIM4_CCMR2      (*(volatile uint32_t*)(TIM4_BASE + 0x1C)) // CH3, CH4 config
#define TIM4_CCER       (*(volatile uint32_t*)(TIM4_BASE + 0x20)) // Enable outputs
#define TIM4_PSC        (*(volatile uint32_t*)(TIM4_BASE + 0x28)) // Prescaler
#define TIM4_ARR        (*(volatile uint32_t*)(TIM4_BASE + 0x2C)) // Period
#define TIM4_CCR1       (*(volatile uint32_t*)(TIM4_BASE + 0x34)) // Duty CH1
#define TIM4_CCR2       (*(volatile uint32_t*)(TIM4_BASE + 0x38)) // Duty CH2
#define TIM4_CCR3       (*(volatile uint32_t*)(TIM4_BASE + 0x3C)) // Duty CH3
#define TIM4_CCR4       (*(volatile uint32_t*)(TIM4_BASE + 0x40)) // Duty CH4


// Function prototypes
void gpio_init(void);
void tim4_pwm_init(void);



// ================= GPIO INITIALIZATION =================
void gpio_init(void)
{
    // -------- Set PD12–PD15 as ALTERNATE FUNCTION --------
    // WHY: PWM signal comes from TIMER, not normal GPIO

    GPIOD_MODER &= ~(0xFF << (12 * 2));
    // WHY: Clear mode bits of PD12–PD15

    GPIOD_MODER |=  (0xAA << (12 * 2));
    // WHY: Set mode = 10 (Alternate Function) for all 4 pins


    // -------- Select AF2 (TIM4) --------
    // WHY: Connect PD12–PD15 to TIM4 channels internally

    GPIOD_AFRH &= ~(0xFFFF << 16);
    // WHY: Clear previous alternate function

    GPIOD_AFRH |=  (0x2222 << 16);
    // WHY: AF2 = TIM4 for pins PD12–PD15
}



// ================= TIM4 PWM INITIALIZATION =================
void tim4_pwm_init(void)
{
    TIM4_CR1 = 0;
    // WHY: Reset timer settings (start clean)


    // -------- TIMER SPEED CONFIG --------
    TIM4_PSC = 16 - 1;
    // WHY: 16 MHz / 16 = 1 MHz → 1 µs per count


    // -------- PWM PERIOD --------
    TIM4_ARR = 1000 - 1;
    // WHY: 1000 counts → 1000 µs → 1 ms period → 1 kHz PWM


    // -------- PWM MODE SETTING --------
    // WHY: Convert timer into PWM generator (Mode 1)

    TIM4_CCMR1 |= (6 << 4);   // CH1 PWM mode
    TIM4_CCMR1 |= (6 << 12);  // CH2 PWM mode

    TIM4_CCMR2 |= (6 << 4);   // CH3 PWM mode
    TIM4_CCMR2 |= (6 << 12);  // CH4 PWM mode


    // -------- ENABLE OUTPUT CHANNELS --------
    // WHY: Without this, PWM will not appear on pins

    TIM4_CCER |= (1<<0);   // CH1 enable (bit 0)
    TIM4_CCER |= (1<<4);   // CH2 enable (bit 4)
    TIM4_CCER |= (1<<8);   // CH3 enable (bit 8)
    TIM4_CCER |= (1<<12);  // CH4 enable (bit 12)


    // -------- SET DUTY CYCLE --------
    // WHY: Controls LED brightness

    TIM4_CCR1 = 250;   // 25% duty → dim LED
    TIM4_CCR2 = 500;   // 50% duty → medium brightness
    TIM4_CCR3 = 750;   // 75% duty → bright
    TIM4_CCR4 = 1000;  // 100% duty → fully ON


    TIM4_CR1 |= (1<<0);
    // WHY: Start timer → PWM starts running
}



// ================= MAIN FUNCTION =================
int main(void)
{
    RCC_AHB1ENR |= (1<<3); // WHY: Enable GPIOD clock
    RCC_APB1ENR |= (1<<2); // WHY: Enable TIM4 clock

    gpio_init();       // WHY: Configure pins for PWM
    tim4_pwm_init();   // WHY: Setup PWM

    while(1)
    {
        // WHY: Nothing required
        // PWM runs automatically in hardware
    }
}
