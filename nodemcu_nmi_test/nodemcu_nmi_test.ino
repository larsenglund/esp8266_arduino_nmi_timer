extern "C" {
  #include "ets_sys.h"
  #include "os_type.h"
  #include "osapi.h"
  #include "gpio.h"
  #include "user_interface.h"
  #include "mem.h"
  #include "hw_timer.h"
}

volatile bool state = false;
void ICACHE_RAM_ATTR blink_gpio(void)
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

void setup() {
  Serial.begin(115200);
  Serial.println("setup()");

  hw_timer_init(FRC1_SOURCE, 1);
  //hw_timer_init(NMI_SOURCE, 1);
  hw_timer_set_func(blink_gpio);
  hw_timer_arm(50);
}

void loop() {
  Serial.print("*");
  delay(1000);
}
