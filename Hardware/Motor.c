#include "stm32f10x.h"
#include "PWM.h"

void Motor_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //使能GPIOB时钟
    GPIOB->CRH &= ~GPIO_CRH_MODE12; //PB12清除原有设置
    GPIOB->CRH |= GPIO_CRH_MODE12; //PB12输出速度50MHz
    GPIOB->CRH &= ~GPIO_CRH_CNF12; //PB12通用推挽输出

    GPIOB->CRH &= ~GPIO_CRH_MODE13; //PB13清除原有设置
    GPIOB->CRH |= GPIO_CRH_MODE13; //PB13输出速度50MHz
    GPIOB->CRH &= ~GPIO_CRH_CNF13; //PB13通用推挽输出
    
    
    
    PWM_Init(); //初始化PWM
}


void Motor_SetPWM(int8_t PWM)
{
	  static int8_t Last_PWM = 0;
      // 当请求和上一次方向不同且值很小时，加补偿
      if ((PWM > 0 && Last_PWM < 0) || (PWM < 0 && Last_PWM
   > 0)) {
          PWM += (PWM > 0) ? 8 : -8;
      }
      Last_PWM = PWM;
    if (PWM > 0) //限制速度在0-100%
    {
        GPIOB->ODR &= ~GPIO_ODR_ODR12; //PB12置1，正转
        GPIOB->ODR |= GPIO_ODR_ODR13; //PB13置0
        PWM_SetDutyCycle(PWM); //设置占空比
    }
    else
    {
        GPIOB->ODR |= GPIO_ODR_ODR12; //PB12置0
        GPIOB->ODR &= ~GPIO_ODR_ODR13; //PB13置1，反转
        PWM_SetDutyCycle(-PWM); //设置占空比
    }
}

