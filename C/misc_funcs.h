/*
 * misc_funcs.h
 *
 *  Created on: 19 Dec 2019
 *      Author: goyf
 */


#ifndef MISC_FUNCS_H_
#define MISC_FUNCS_H_

#include "driverlib/pwm.h"

#define PWM_FREQUENCY 60

void PortDIntHandler(void);

void timer_setup(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    uint32_t ui32Period;
    ui32Period = (SysCtlClockGet() / 10000) / 2; // 100Hz to get 20ms
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);
}
volatile uint32_t ui32Load;
void pwm_setup(void)
{
    //volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;

    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPinConfigure(GPIO_PD0_M1PWM0);

    GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_1);
    GPIOPinConfigure(GPIO_PD1_M1PWM1);

    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 1);
    PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, 1);
    PWMOutputState(PWM1_BASE, PWM_OUT_1_BIT, true);

    PWMGenEnable(PWM1_BASE, PWM_GEN_0);
}

void motor_dir_change(int dir)
{
    if (dir == 0)
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 1);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, ui32Load*0.2);
    }

    else if (dir == 1)
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui32Load*0.2);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, 1);
    }
}

void motor_stop(void)
{
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 1);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, 1);
}

void pin_int_setup(void)
{
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);
    GPIOPadConfigSet(GPIO_PORTD_BASE,GPIO_PIN_2,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
    GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_2);

    GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_INT_PIN_2, GPIO_FALLING_EDGE);
    IntPrioritySet(INT_GPIOD, 0);
    IntRegister(INT_GPIOD, PortDIntHandler);
    IntEnable(INT_GPIOD);
    IntMasterEnable();
}




#endif /* MISC_FUNCS_H_ */
