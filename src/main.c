#include "main.h"

uint32_t core_clock_hz;

int main(void) {
    // need 1 wait state at 32MHz
    FLASH->ACR &= ~(0xF);
    FLASH->ACR |=  (FLASH_ACR_LATENCY | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_1WS);

    // select pll clk source (16MHz HSI)
    RCC->PLLCFGR &= ~(0x3);
    RCC->PLLCFGR |= 0x2;
    
    // select division factor (PLLM)
    RCC->PLLCFGR &= ~(0xf << 4);
    RCC->PLLCFGR |= (0x1 << 4);

    // select multiplication factor (PLLN)
    RCC->PLLCFGR &= ~(0x7f << 8);
    RCC->PLLCFGR |= (0x8 << 8);

    // set PLL R clock (system clock) division factor
    RCC->PLLCFGR &= ~(0x3 << 25);
    RCC->PLLCFGR |= (0x1 << 24);

    // turn PLL on
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {};

    // select sysclk source as pll
    RCC->CFGR |= 0x3;

    // read which source is used
    volatile uint8_t cfgr = RCC->CFGR & 0xc;
    core_clock_hz = 32000000;

    // Enable the GPIOB peripheral in 'RCC_AHB2ENR'.
    volatile uint32_t tmp2 __attribute__((unused)) = RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    tmp2 = RCC->AHB2ENR & RCC_AHB2ENR_GPIOBEN;

    // Enable the TIM2 clock.
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    // Enable the NVIC interrupt for TIM2.
    NVIC_SetPriority(TIM2_IRQn, 0x03);
    NVIC_EnableIRQ(TIM2_IRQn);

    // reset LD2 pin functions
    GPIOB->MODER &= ~(0x3 << (8*2));
    GPIOB->PUPDR &= ~(0x3 << (8*2));
    GPIOB->OSPEEDR &= ~(0x3 << (8*2));
    GPIOB->OTYPER &= ~(0x1 << 8);

    // set LD2 activation to pull up
    GPIOB->MODER |= (0x1 << (8*2)); 

    // set LD2 on
    GPIOB->ODR |= (0x1 << 8);

    start_timer(TIM2, 1000);
    while (1) {

    }
}

void start_timer(TIM_TypeDef *TIMx, uint16_t ms) {
    // Start by making sure the timer's 'counter' is off.
    TIMx->CR1 &= ~(TIM_CR1_CEN);
    // Next, reset the peripheral. (This is where a HAL can help)
    if (TIMx == TIM2) {
        RCC->APB1RSTR1 |=  (RCC_APB1RSTR1_TIM2RST);
        RCC->APB1RSTR1 &= ~(RCC_APB1RSTR1_TIM2RST);
    }
    
    // Set the timer prescaler/autoreload timing registers.
    // (These are 16-bit timers, so this won't work with >65MHz.)
    TIMx->PSC   = core_clock_hz / 1000;
    TIMx->ARR   = ms;
    // Send an update event to reset the timer and apply settings.
    TIMx->EGR  |= TIM_EGR_UG;
    // Enable the hardware interrupt.
    TIMx->DIER |= TIM_DIER_UIE;
    // Enable the timer.
    TIMx->CR1  |= TIM_CR1_CEN;
}

void TIM2_IRQHandler(void) {
    // Handle a timer 'update' interrupt event
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~(TIM_SR_UIF);
        // Toggle the LED output pin.
        GPIOB->ODR ^= (1 << 8);
    }
}