#ifndef _DEA_MAIN_H
#define _DEA_MAIN_H

#include <stdint.h>
#include "stm32g4xx.h"

void start_timer(TIM_TypeDef *TIMx, uint16_t ms);

#endif