# esp8266 NMI timer
Example of using the NMI (non interruptable) timer source on esp8266 in Arduino

Works just fine with FRC1_SOURCE but gives watchdog resets with NMI_SOURCE.. any thoughts?
It prints `setup()*` before watchdog-resetting.
Using FRC1_SOURCE i can even remove the stuff around the indented `///your code here` block and it still works fine
