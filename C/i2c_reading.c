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
#include "matrices.h"

#define SAMP_SIZE 100


volatile int16_t mem_count = 0;
volatile int state = 0; //0 - collecting, 1 - sending

volatile int8_t angleH = 0;
volatile int8_t angleL = 0;
volatile int8_t angleT = 0;

volatile int16_t angle_arr[SAMP_SIZE];
volatile int8_t angle_fin = 0;
volatile int8_t angle_print = 0;
void Timer0IntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    float avg_angle = 0;
    int i;

    switch(state)
    {
    case 0:
        angleH = I2CReceive(0x68, 0x45);
        angleL = I2CReceive(0x68, 0x46);
        angleT = (((angleH << 8) | angleL)/ 131);
        angle_arr[mem_count] = angleT;


        //Get stuff from 0x45
        volatile int error = 0;
        volatile int8_t angleT = 0;
        I2C0_MSA_R = 0xD0;
        I2C0_MDR_R = 0x45;
        I2C0_MCS_R = 0x07;
        //I2C0_MCS_R = 3;
        while((I2C0_MCS_R & 0x40)!=0);
        while((I2C0_MCS_R & 0x01)!=0);

        I2C0_MSA_R = 0xD1;
        I2C0_MCS_R = 0x07;
        while((I2C0_MCS_R & 0x40)!=0);
        if((I2C0_MCS_R & 0x02)==1)
            error = 1;
        else
        {
            angleT = I2C0_MDR_R;
        }


        mem_count++;
        if (mem_count >= SAMP_SIZE)
        {
            mem_count = 0;
            state = 1;
        }
        //state = 1;
        break;
    case 1:

        for(i = 0; i < SAMP_SIZE; i++)
        {
            avg_angle = avg_angle + angle_arr[i];
        }
        angle_fin = angle_fin + avg_angle/SAMP_SIZE;

        char send_angle_arr[6];
        int_to_array(angle_print, send_angle_arr, 6);


        print_text("Angle: ", sizeof("Angle: "));
        if (angle_fin < 0)
        {
            UARTCharPut(UART0_BASE, '-');
            angle_print = angle_fin*-1;
        }
        else
            angle_print = angle_fin;
        print_numln(send_angle_arr, 6);
        state = 0;
        break;
    }
}

int main(void)
{


    // Set the clocking to run directly from the external crystal/oscillator.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    int clock_check = SysCtlClockGet();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0b100000);

    //Initialise I2C module 0
    InitI2C0();

    uint32_t pwr_check = I2CReceive(0x68, 0x6B);
    writeI2C0(0x68, 0x6B, 0x03);
    pwr_check = I2CReceive(0x68, 0x6B);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    uint32_t ui32Period;
    ui32Period = (SysCtlClockGet() / 1000) / 2; // 100Hz to get 20ms
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);

    ConfigureUART();

    UARTCharPut(UART0_BASE, 'X');
    UARTCharPut(UART0_BASE, '\n');
    UARTCharPut(UART0_BASE, '\r');


    while(1)
    {
    };
}
