#include <xc.h>

volatile unsigned int led_state = 0;
#define LED_PIN (1 << 17);



//delay function from previous lab. It just stalls
void delay_ms(int ms) {
    for (volatile int i = 0; i < ms * 2; i++);
}
void TC5_Handler(void)
{
    // this will toggle the LED state on and off
    led_state = !led_state;
    
    // remmeber to reset the INT flag or else the handler will run again immeditly
    TC5_REGS->COUNT16.TC_INTFLAG = TC_INTFLAG_MC0_Msk;
}


static void Timer5_Init(void) {
    // the setup is long and is an absolute pain to do with bit shifting (1 << n)
    // the pm, gclk, and tc header files already have many useful definition you can just plug in
    
    // they basically all follow the same naming convention
    // EX GCLK_CLKCTRL_ID_TC4_TC5
    // GCLK tells you the group, CLKCTRL tells you the register, ID refers to the group of bits its targetting, and lastly TC4_TC5 is an option
    // this definiton specifically is used to set the ID in the CLKCTRL register to TC4_TC5
    
    
    // Enable the APBC clock for TC5
    PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC5_Msk; //see 16.7 for more information
    //TLDR TC5 is part of the APBC group
   
    // Here Im making a few assumptions theres no need to select the clock src
    // by default the src should be OSC8M and DIV factor should be 8
    // you can confirm using debug and checking the registers
    
    // what we will need to do is route the clock to the TC5 peripheral
    // select one of 9 generic clock generators, i choose 0
    // we need to pass it the ID of the TC5 peripheral
    // then set CLK EN to 1 to activate the generic clock you choose
    // see 15.7 for more information
    GCLK_REGS->GCLK_CLKCTRL = 
            GCLK_CLKCTRL_ID_TC4_TC5 
            | GCLK_CLKCTRL_GEN_GCLK0 
            | GCLK_CLKCTRL_CLKEN_Msk;
    while(GCLK_REGS->GCLK_STATUS); // youll see these while loops scattered in the setup
    // most of the groups have status registers, the while loop gives some time to make changes
    // will continue once its not busy anymore
    
    // reset TC5 //lowkey might be uneccesary but just ensures a clean slate 
    TC5_REGS->COUNT16.TC_CTRLA = TC_CTRLA_SWRST_Msk;
    while (TC5_REGS->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk);
    
    // now we need to configure the TC peripheral
    // TC5 has 3 modes and the one we are using is 16 bits
    // so it can only count to like 30k, so youll have to divide it
    // i choose 64 as the prescaler (div) but i think he expects 1024, itll matter in the next step
    // make sure the mode is count 16
    // also make sure to select MFRQ or match frequency. This will compare it to CC0
    // refer to 30.9 for TC5 register information 
    TC5_REGS->COUNT16.TC_CTRLA = TC_CTRLA_WAVEGEN_MFRQ 
        | TC_CTRLA_MODE_COUNT16
        | TC_CTRLA_PRESCALER_DIV64;
    while (TC5_REGS->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk); 

    // Set 1Mhz/64 = 15625, so theoretically 1 second (if you choose a different prescaler divide by a diff num)
    // SETS CC0 to 15625
    TC5_REGS->COUNT16.TC_CC[0] = 15625; // after 1 million operations, that would be 1s
    while (TC5_REGS->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk); 

    // Next is to toggle MC0 in the INTENSET 
    // this will enable the match interrupt on CC0
    TC5_REGS->COUNT16.TC_INTENSET = TC_INTENSET_MC0_Msk;
    
    // Now the microcontroller should be listening to INT from TC5
    NVIC_ClearPendingIRQ(TC5_IRQn); //ensures no old TC5 interrupts are waiting
    NVIC_EnableIRQ(TC5_IRQn); //CPU will accept TC5 interrupts at the priority we assigned.
    __enable_irq(); // global interrupt-enable call (if interrupts were disabled, this turns it on)
    
    // Finally Enable TC5
    TC5_REGS->COUNT16.TC_CTRLA |= TC_CTRLA_ENABLE_Msk;
    while (TC5_REGS->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk);
    
}

int main(void)
{
    // Configure PA17 as output for the LED
    PORT_REGS->GROUP[0].PORT_DIRSET = (1 << 17);

    // Initialize our timer + interrupt
    Timer5_Init();
    
    while (1){
        // you need some delay since the clock moves faster than the registers can update
        // but also not really because 1Mhz is not that fast but he might still grill you if you dont
        delay_ms(10);
        if (led_state) {
            // sets it 1
            PORT_REGS->GROUP[0].PORT_OUTSET = LED_PIN;
        }
        else {
            // sets it 0
            PORT_REGS->GROUP[0].PORT_OUTCLR = LED_PIN;
        }
    }
    return 0; 
}