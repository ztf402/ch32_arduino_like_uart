# ch32_arduino_like_uart
It uses register to init ,send or receive message.It has more stablity than directly using offical one
# How to use it?
Just like using arduino library,the only thing different is you should first define the object,just like this UART uart5_my(UART5, GPIOC, GPIO_Pin_12, GPIOD, GPIO_Pin_2); // UART5 TX: PC12, RX: PD2
And than you can do anything you want to do in Arduino serial library.Enjoy!
# Why there are such many Errors when using it outside ch32v30x
Some MCUs have different names of registers,just like the DATAR rename to DR,you need to revise it.
Some MCUs don't have so many USART/UART ,you also need to revise the init function.
