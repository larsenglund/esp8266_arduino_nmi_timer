extern "C" {
  #include "ets_sys.h"
  #include "os_type.h"
  #include "osapi.h"
  #include "gpio.h"
  #include "user_interface.h"
  #include "mem.h"
  #include "hw_timer.h"
}

void setup() {
  Serial.begin(115200);
  Serial.println("setup()");

  hw_timer_init(FRC1_SOURCE, 1);
  //hw_timer_init(NMI_SOURCE, 1);
  hw_timer_set_func(blink_gpio);
  hw_timer_arm(25);
}

void loop() {
  Serial.print("*");
  delay(1000);
}
