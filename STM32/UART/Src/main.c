#include <stdint.h>

/* ================= RCC Registers ================= */
#define RCC_AHB1ENR     (*(volatile uint32_t*)0x40023830)
#define RCC_APB1ENR     (*(volatile uint32_t*)0x40023840)

/* ================= GPIOB Registers ================= */
#define GPIOB_MODER     (*(volatile uint32_t*)0x40020400)
#define GPIOB_OTYPER    (*(volatile uint32_t*)0x40020404)
#define GPIOB_OSPEEDR   (*(volatile uint32_t*)0x40020408)
#define GPIOB_PUPDR     (*(volatile uint32_t*)0x4002040C)
#define GPIOB_AFRH      (*(volatile uint32_t*)0x40020424)

/* ================= I2C1 Registers ================= */
#define I2C1_CR1        (*(volatile uint32_t*)0x40005400)
#define I2C1_CR2        (*(volatile uint32_t*)0x40005404)
#define I2C1_DR         (*(volatile uint32_t*)0x40005410)
#define I2C1_SR1        (*(volatile uint32_t*)0x40005414)
#define I2C1_SR2        (*(volatile uint32_t*)0x40005418)
#define I2C1_CCR        (*(volatile uint32_t*)0x4000541C)
#define I2C1_TRISE      (*(volatile uint32_t*)0x40005420)

/* ================= TIM2 Registers ================= */
#define TIM2_CR1        (*(volatile uint32_t*)0x40000000)
#define TIM2_SR         (*(volatile uint32_t*)0x40000010)
#define TIM2_CNT        (*(volatile uint32_t*)0x40000024)
#define TIM2_PSC        (*(volatile uint32_t*)0x40000028)
#define TIM2_ARR        (*(volatile uint32_t*)0x4000002C)

/* ================= Function Prototypes ================= */
void I2C1_Init(void);
void TIM2_Init(void);
void delay_ms(uint32_t ms);

void I2C1_Start(void);
void I2C1_Stop(void);
void I2C1_Write(uint8_t data);
uint8_t I2C1_Read_NACK(void);

void EEPROM_Write(uint16_t mem_addr, uint8_t data);
uint8_t EEPROM_Read(uint16_t mem_addr);

/* =========================================================
   TIM2 Initialization
   1 ms timer tick
========================================================= */
void TIM2_Init(void)
{
    RCC_APB1ENR |= (1 << 0);   // Enable TIM2 clock

    TIM2_PSC = 16000 - 1;      // 16MHz /16000 = 1kHz
    TIM2_ARR = 1 - 1;          // 1ms overflow

    TIM2_CNT = 0;
    TIM2_CR1 |= (1 << 0);      // Enable timer
}

/* =========================================================
   Delay using TIM2
========================================================= */
void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms; i++)
    {
        TIM2_CNT = 0;

        while(!(TIM2_SR & (1 << 0)));

        TIM2_SR &= ~(1 << 0);
    }
}

/* =========================================================
   I2C1 Initialization
========================================================= */
void I2C1_Init(void)
{
    /* Enable GPIOB clock */
    RCC_AHB1ENR |= (1 << 1);

    /* Enable I2C1 clock */
    RCC_APB1ENR |= (1 << 21);

    /* PB8, PB9 Alternate Function */
    GPIOB_MODER &= ~((3 << 16) | (3 << 18));
    GPIOB_MODER |=  ((2 << 16) | (2 << 18));

    /* Open Drain */
    GPIOB_OTYPER |= (1 << 8) | (1 << 9);

    /* High Speed */
    GPIOB_OSPEEDR |= (3 << 16) | (3 << 18);

    /* Pull-up */
    GPIOB_PUPDR &= ~((3 << 16) | (3 << 18));
    GPIOB_PUPDR |=  ((1 << 16) | (1 << 18));

    /* AF4 for I2C */
    GPIOB_AFRH &= ~((0xF << 0) | (0xF << 4));
    GPIOB_AFRH |=  ((4 << 0) | (4 << 4));

    /* Reset I2C */
    I2C1_CR1 = 0x8000;
    I2C1_CR1 = 0x0000;

    /* APB1 = 16MHz */
    I2C1_CR2 = 16;

    /* 100kHz I2C */
    I2C1_CCR = 80;
    I2C1_TRISE = 17;

    /* Enable I2C */
    I2C1_CR1 |= (1 << 0);
}

/* =========================================================
   Generate START
========================================================= */
void I2C1_Start(void)
{
    I2C1_CR1 |= (1 << 8);

    while(!(I2C1_SR1 & (1 << 0)));
}

/* =========================================================
   Generate STOP
========================================================= */
void I2C1_Stop(void)
{
    I2C1_CR1 |= (1 << 9);
}

/* =========================================================
   Write Byte
========================================================= */
void I2C1_Write(uint8_t data)
{
    while(!(I2C1_SR1 & (1 << 7)));

    I2C1_DR = data;

    while(!(I2C1_SR1 & (1 << 2)));
}

/* =========================================================
   Read Byte (NACK)
========================================================= */
uint8_t I2C1_Read_NACK(void)
{
    I2C1_CR1 &= ~(1 << 10);

    while(!(I2C1_SR1 & (1 << 6)));

    return I2C1_DR;
}

/* =========================================================
   EEPROM Write
========================================================= */
void EEPROM_Write(uint16_t mem_addr, uint8_t data)
{
    I2C1_Start();

    I2C1_DR = 0xA0;
    while(!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    I2C1_Write(mem_addr >> 8);
    I2C1_Write(mem_addr & 0xFF);

    I2C1_Write(data);

    I2C1_Stop();

    delay_ms(5);   // EEPROM internal write time
}

/* =========================================================
   EEPROM Read
========================================================= */
uint8_t EEPROM_Read(uint16_t mem_addr)
{
    uint8_t data;

    I2C1_Start();

    I2C1_DR = 0xA0;
    while(!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    I2C1_Write(mem_addr >> 8);
    I2C1_Write(mem_addr & 0xFF);

    /* Repeated Start */
    I2C1_Start();

    I2C1_DR = 0xA1;
    while(!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    data = I2C1_Read_NACK();

    I2C1_Stop();

    return data;
}

/* =========================================================
   Main
========================================================= */
int main(void)
{
    uint8_t value;

    TIM2_Init();
    I2C1_Init();

    /* Write 0x55 to address 0x0010 */
    EEPROM_Write(0x0010, 0x55);

    /* Read from same address */
    value = EEPROM_Read(0x0010);

    while(1)
    {
        // Put breakpoint here
        // value should be 0x55
    }
}
