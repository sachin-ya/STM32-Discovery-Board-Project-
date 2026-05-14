#include <stdint.h>

/* ================= BASE ADDRESSES ================= */
#define RCC_BASE        0x40023800U
#define GPIOA_BASE      0x40020000U
#define GPIOB_BASE      0x40020400U
#define USART2_BASE     0x40004400U
#define I2C1_BASE       0x40005400U

/* ================= RCC ================= */
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR    (*(volatile uint32_t *)(RCC_BASE + 0x40))

/* ================= GPIO ================= */
#define GPIOA_MODER    (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL     (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

#define GPIOB_MODER    (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER   (*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_AFRL     (*(volatile uint32_t *)(GPIOB_BASE + 0x20))

/* ================= USART ================= */
#define USART2_SR      (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR      (*(volatile uint32_t *)(USART2_BASE + 0x04))
#define USART2_BRR     (*(volatile uint32_t *)(USART2_BASE + 0x08))
#define USART2_CR1     (*(volatile uint32_t *)(USART2_BASE + 0x0C))

/* ================= I2C ================= */
#define I2C1_CR1       (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2       (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_SR1       (*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C1_SR2       (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_DR        (*(volatile uint32_t *)(I2C1_BASE + 0x10))
#define I2C1_CCR       (*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C1_TRISE     (*(volatile uint32_t *)(I2C1_BASE + 0x20))

#define EEPROM_ADDR    0x50

/* ================= UART FUNCTIONS ================= */
void uart2_init(void)
{
    RCC_AHB1ENR |= (1 << 0);      // GPIOA
    RCC_APB1ENR |= (1 << 17);     // USART2

    GPIOA_MODER &= ~(0xF << 4);
    GPIOA_MODER |=  (0xA << 4);   // PA2, PA3 AF
    GPIOA_AFRL  |= (7 << 8) | (7 << 12);

    USART2_BRR = 0x683;           // 9600 baud @16MHz
    USART2_CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
}

void uart_tx(char c)
{
    while (!(USART2_SR & (1 << 7)));
    USART2_DR = c;
}

char uart_rx(void)
{
    while (!(USART2_SR & (1 << 5)));
    return USART2_DR;
}

void uart_print(const char *s)
{
    while (*s)
        uart_tx(*s++);
}

/* ================= I2C FUNCTIONS ================= */
void i2c1_init(void)
{
    RCC_AHB1ENR |= (1 << 1);
    RCC_APB1ENR |= (1 << 21);

    GPIOB_MODER |= (2 << 12) | (2 << 14);
    GPIOB_OTYPER |= (1 << 6) | (1 << 7);
    GPIOB_AFRL |= (4 << 24) | (4 << 28);

    I2C1_CR2 = 16;
    I2C1_CCR = 80;
    I2C1_TRISE = 17;

    I2C1_CR1 |= (1 << 0);
}

/* ================= EEPROM FUNCTIONS ================= */
void eeprom_write(uint16_t addr, char data)
{
    I2C1_CR1 |= (1 << 8);
    while (!(I2C1_SR1 & 1));

    I2C1_DR = (EEPROM_ADDR << 1);
    while (!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    I2C1_DR = addr >> 8;
    while (!(I2C1_SR1 & (1 << 7)));

    I2C1_DR = addr & 0xFF;
    while (!(I2C1_SR1 & (1 << 7)));

    I2C1_DR = data;
    while (!(I2C1_SR1 & (1 << 7)));

    I2C1_CR1 |= (1 << 9);
}

char eeprom_read(uint16_t addr)
{
    char data;

    I2C1_CR1 |= (1 << 8);
    while (!(I2C1_SR1 & 1));

    I2C1_DR = (EEPROM_ADDR << 1);
    while (!(I2C1_SR1 & (1 << 1)));
    (void)I2C1_SR2;

    I2C1_DR = addr >> 8;
    while (!(I2C1_SR1 & (1 << 7)));

    I2C1_DR = addr & 0xFF;
    while (!(I2C1_SR1 & (1 << 7)));

    I2C1_CR1 |= (1 << 8);
    I2C1_DR = (EEPROM_ADDR << 1) | 1;

    while (!(I2C1_SR1 & (1 << 6)));
    data = I2C1_DR;

    I2C1_CR1 |= (1 << 9);
    return data;
}

/* ================= MAIN ================= */
int main(void)
{
    uint16_t addr = 0;
    char c;

    uart2_init();
    i2c1_init();

    uart_print("\r\nType text and press ENTER:\r\n");

    /* WRITE TO EEPROM */
    while (1)
    {
        c = uart_rx();

        if (c == '\n') continue;   // ignore LF

        uart_tx(c);                // echo

        if (c == '\r') break;      // ENTER

        eeprom_write(addr++, c);
        for (volatile int i = 0; i < 80000; i++);
    }

    uart_print("\r\n\r\nEEPROM DATA:\r\n");

    /* READ FROM EEPROM */
    for (uint16_t i = 0; i < addr; i++)
    {
        uart_tx(eeprom_read(i));
    }

    while (1);
}
