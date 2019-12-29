#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"

#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#include "comms_func.h"
#include "misc_funcs.h"
#include "matrices.h"

#define SAMP_SIZE 200


volatile int mags = 0;
volatile int dis = 0;


volatile int16_t angle_arr[SAMP_SIZE];
volatile int16_t mem_count = 0;
volatile int16_t state = 0;
volatile int8_t angle1 = 0;
volatile int8_t angle2 = 0;
volatile int8_t angleT = 0;
volatile int8_t angle_fin = 0;
volatile int8_t angle_print = 0;
//volatile int test_count = 0;
void Timer0IntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //int16_t avg_angle = 0;
    int i;

    //test_count++;
    /*if (test_count == 10000)
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 1);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, ui32Load*0.05);
        //test_count = 0;
    }*/

    switch(state)
    {
    case 0:
        angle1 = I2CReceive(0x68, 0x45);
        angle2 = I2CReceive(0x68, 0x46);
        angleT = (((angle1 << 8) | angle2)/ 131);
        //angle_arr[mem_count] = angleT; //I think this thing is to have an array that gets then averaged
        /*mem_count++;
        if (mem_count >= SAMP_SIZE)
        {
            mem_count = 0;
            x1[0] = dis;
            x1[3] =
            state = 1;
        }*/
        x1[0] = dis;
        x1[3] = angleT;
        //I am missing accelerations
        state = 1;
        break;
    case 1:
        //This would be remade to instead to adjust motors based on the calcs done here?
        //Either stops the motors or starts them in either direction

        x2[0] = A[0][0]*x1[0] + A[0][1]*x1[1];
        x2[1] = A[1][0]*x1[0] + A[1][1]*x1[0] + A[1][2]*x1[0] + A[1][3]*x1[0] + A[1][4]*x1[0] + A[1][5]*x1[0] + A[1][6]*x1[0] + A[1][7]*x1[0];// + A[1][8]*x1[0];
        x2[2] = A[2][2]*x1[2] + A[2][3]*x1[3]+ A[2][6]*x1[6];
        x2[3] = A[3][0]*x1[0] + A[3][1]*x1[0] + A[3][2]*x1[0] + A[3][3]*x1[0] + A[3][4]*x1[0] + A[3][5]*x1[0] + A[3][6]*x1[0] + A[3][7]*x1[0];// + A[3][8]*x1[0];
        x2[4] = 0.0;  //These things turn out to be zeroes at all times in Matlab
        x2[5] = 0.0;  //If the pendulum doesn't work, this is one of the possible culprits but I doubt it
        x2[6] = 0.0;
        x2[7] = 0.0;

        y[0] = x2[0];
        y[1] = x2[2];

        for(i = 0; i < 8; i++)
            x1[i] = x2[i];

        //Is the distance the same?
            //If so, do nothing
        //If there's a significant difference, assess direction
            //How to turn PWM on and off?
                //Set the PWM to 1, so that the countdown is tiny
                //Or just setup both output and PWM on the same pin and then just flip between PWM and ground?
                //There's also the invert function
            //Change direction
            //Start the motor
        if (y[0] - (float) dis/1000 > 20.0) //20 millimeters. y[0] will have to be adjusted ADJUST
            motor_dir_change(0);
        else if (dis/1000 - (float) y[0] > 20.0)
            motor_dir_change(1);
        else
            motor_stop();

        state = 0;
        break;
    }
}


void PortDIntHandler()
{
    GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_2);

    //char send_arr[6];
    //int dis = 0;
    //int i = 0;

    mags++;
    dis = mags*6; //6 millimeters

}

int main(void)
{


    // Set the clocking to run directly from the external crystal/oscillator.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    int clock_check = SysCtlClockGet();

    //Initialise I2C module 0
    InitI2C0();

    uint32_t pwr_check = I2CReceive(0x68, 0x6B);
    writeI2C0(0x68, 0x6B, 0x03);
    pwr_check = I2CReceive(0x68, 0x6B);

    //Timer setup start
    timer_setup();

    //ConfigureUART();

    //Setup of PWM
    pwm_setup();

    //Setup of pin interrupt
    pin_int_setup();

    //UARTCharPut(UART0_BASE, 'H');
    //UARTCharPut(UART0_BASE, '\n');
    //UARTCharPut(UART0_BASE, '\r');




    while(1)
    {
    };
}
