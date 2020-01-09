//Initialise I2C module 0
//Slightly modified version of TI's example code
void InitI2C0(void)
{
    //enable I2C module 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    //reset module
    //SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    //enable GPIO peripheral that contains I2C 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);

    //clear I2C FIFOs
    //HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
}

void initI2C0_dir(void)
{
    SYSCTL_RCGCI2C_R|= 0x01;
    SYSCTL_RCGCGPIO_R |= 0x02;

    GPIO_PORTB_AFSEL_R|= 0x0C;
    GPIO_PORTB_PCTL_R &= ~0x0000FF00;
    GPIO_PORTB_PCTL_R |= 0x00003300;
    GPIO_PORTB_DEN_R|=0x0C;
    GPIO_PORTB_ODR_R|= 0x08;


    //Skipped GPIOPCTL
    I2C0_MCR_R = 0x10;
    I2C0_MTPR_R = (SysCtlClockGet()/(2*(6 + 4)*100000))-1;
    //I2C0_MTPR_R = 9;


}

//read specified register on slave device
uint32_t I2CReceive(uint32_t slave_addr, int8_t reg)
{
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);

    //specify register to be read
    I2CMasterDataPut(I2C0_BASE, reg);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND); //I2C_MASTER_CMD_BURST_SEND_START

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, true);

    //send control byte and read from the register we
    //specified
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));

    //return data pulled from the specified register
    return I2CMasterDataGet(I2C0_BASE);
    //return 0;
}

void writeI2C0(int16_t device_address, int16_t device_register, int8_t device_data)
{
   //This works because the MPU6050 can't have a S condition between its sends
   //And the Single Send does exactly that
   //Burst avoids this
   I2CMasterSlaveAddrSet(I2C0_BASE, 0x68, false);
   I2CMasterDataPut(I2C0_BASE, 0x6B);
   I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
   while(I2CMasterBusy(I2C0_BASE));

   I2CMasterDataPut(I2C0_BASE, device_data);
   I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
   while(I2CMasterBusy(I2C0_BASE));
}

uint8_t readI2C0_dir(uint16_t slave_addr, uint16_t mem_addr)
{
    //Get stuff from 0x45
    volatile int error = 0;
    //I2C0_MSA_R = 0xD0;
    I2C0_MSA_R = slave_addr;
    I2C0_MSA_R = (I2C0_MSA_R << 1);
    //I2C0_MDR_R = 0x45;
    I2C0_MDR_R = mem_addr;
    I2C0_MCS_R = 0x07;
    //I2C0_MCS_R = 3;
    //while((I2C0_MCS_R & 0x40)!=0);
    while((I2C0_MCS_R & 0x01)!=0);

    //I2C0_MSA_R = 0xD1;
    I2C0_MSA_R = slave_addr;
    I2C0_MSA_R = (I2C0_MSA_R << 1) + 1;
    I2C0_MCS_R = 0x07;
    while((I2C0_MCS_R & 0x01)!=0);
    if((I2C0_MCS_R & 0x02)==1)
    {
        error = 1;
        return 0;
    }
    else
    {
        return I2C0_MDR_R;
    }
}

//This function does burst write to MPU6050
//This is because that is how MPU6050 works, i.e. it does not work with single writes
void writeI2C0_dir(int16_t slave_addr, int16_t mem_addr, int16_t msg)
{
    volatile int error = 0;
    I2C0_MSA_R = slave_addr;
    I2C0_MSA_R = (I2C0_MSA_R << 1);
    I2C0_MDR_R = mem_addr;
    I2C0_MCS_R = 0x03;
    while((I2C0_MCS_R & 0x01)!=0);

    I2C0_MDR_R = msg;
    I2C0_MCS_R = 0x05;
    while((I2C0_MCS_R & 0x01)!=0);

}


//Single I2C write, not used but just in case.
void writeI2C0_dir_single(int16_t slave_addr, int16_t msg)
{
    volatile int error = 0;
    I2C0_MSA_R = slave_addr;
    I2C0_MSA_R = (I2C0_MSA_R << 1);
    I2C0_MDR_R = msg;
    I2C0_MCS_R = 0x07;
    while((I2C0_MCS_R & 0x01)!=0);
}

void ConfigureUART(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    UARTEnable(UART0_BASE);
}

void print_text(char arr[], int size)
{
    int i = 0;
    for(i = 0; i < size; i++)
        UARTCharPut(UART0_BASE, arr[i]);
}

void print_textln(char arr[], int size)
{
    int i = 0;
    for(i = 0; i < size; i++)
        UARTCharPut(UART0_BASE, arr[i]);

    UARTCharPut(UART0_BASE, '\n');
    UARTCharPut(UART0_BASE, '\r');
}

void print_num(char arr[], int size) //, int size
{
    int i = 0;
    //int size = sizeof(arr)/sizeof(arr[0]);
    for(i = size-1; i > -1; i--)
    {
        UARTCharPut(UART0_BASE, arr[i]);
    }

}

void print_numln(char arr[], int size) //, int size
{
    int i = 0;
    //int size = sizeof(arr)/sizeof(arr[0]);
    for(i = size-1; i > -1; i--)
    {
        UARTCharPut(UART0_BASE, arr[i]);
    }
    UARTCharPut(UART0_BASE, '\n');
    UARTCharPut(UART0_BASE, '\r');

}

char * int_to_array(int num, char arr[], int size)
{
    int i;
    for(i = 0; i < size; i++)
    {
        arr[i] = (num % 10) + 48;
        num = num / 10;
    }

    return arr;

}

char * decimal_to_array(float dec_num, char arr[4])
{
    int i;
    int num = dec_num * 10000;
    for(i = 0; i < 4; i++)
    {
        arr[i] = (num % 10) + 48;
        num = num / 10;
    }

    return arr;
}
