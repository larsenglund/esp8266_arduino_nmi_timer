#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"

typedef enum {
    FRC1_SOURCE = 0,
    NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;

volatile bool state;

//#define ICACHE_RAM_ATTR     __attribute__((section(".iram.text")))
//void ICACHE_RAM_ATTR
void // defaults to RAM? http://bbs.espressif.com/viewtopic.php?t=622
blink_gpio(void)
{
  //if ((T1C & ((1 << TCAR) | (1 << TCIT))) == 0) TEIE &= ~TEIE1;//edge int disable
  //T1I = 0;

  // to make ISR compatible to Arduino AVR model where interrupts are disabled
  // we disable them before we call the client ISR
  //uint32_t savedPS = xt_rsil(15); // stop other interrupts
  
  ///your code here
    state = !state;
    if (state)
    {
        //Set GPIO2 to LOW
        gpio_output_set(0, BIT2, BIT2, 0);
    }
    else
    {
        //Set GPIO2 to HIGH
        gpio_output_set(BIT2, 0, BIT2, 0);
    }

  //xt_wsr_ps(savedPS);
}

//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
    // Initialize the GPIO subsystem.
    gpio_init();

    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

    //Set GPIO2 low
    gpio_output_set(0, BIT2, BIT2, 0);

    //hw_timer_init(FRC1_SOURCE, 1); // Works here and in Arduino version
    hw_timer_init(NMI_SOURCE, 1);
    hw_timer_set_func(blink_gpio);
    hw_timer_arm(50);
}
