#ifndef __PID_H
#define __PID_H

typedef struct
{
    float Kp;
    float Ki;
    float Kd;
    float Target;
    float Actual;
    float Out;
    float Error0;
    float Error1;
    float ErrorInt;
    float OutMax;
    float OutMin;
	float History;   // ← 放结构体里，每次调用都保留
	float Alpha;      // ← 滤波系数，0.3 左右
}PID_TypeDef;

void PID_Init(PID_TypeDef *p);

void PID_Update(PID_TypeDef *p);


#endif
