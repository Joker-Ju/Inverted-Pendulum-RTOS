#include "stm32f10x.h"
#include "PID.h"


void PID_Init(PID_TypeDef *p)
{
	p->Actual = 0;
	p->Error0 = 0;
	p->Error1 = 0;
	p->ErrorInt = 0;
	p->Out = 0;
	p->History = 0;
}


void PID_Update(PID_TypeDef *p)
{
    p->Error1 = p->Error0;
    p->Error0 = p->Target - p->Actual;

    if (p->Ki != 0){
        p->ErrorInt += p->Error0;
    }
    else{
        p->ErrorInt = 0;
    }
	
	float derivative_raw = p->Error0 - p->Error1;
	p->History = (1 - p->Alpha) * p->History + p->Alpha * derivative_raw;
	p->Out = p->Kp * p->Error0 + p->Ki * p->ErrorInt + p->Kd * p->History;

   
    if (p->Out > p->OutMax)p->Out = p->OutMax;
    else if (p->Out < p->OutMin)p->Out = p->OutMin;
}


