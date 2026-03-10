# ATSAMD21-Event-Driven-Timer-Interrupts
This project demonstrates the transition from "Busy-Wait" programming to Event-Driven Architecture on the ATSAMD21E18A. I utilize the hardware timers and interrupts so that the system can achieve precise timing without blocking the main execution loop.
### Technical Features
- **Hardware Timer Configuration:** Configured TC5 in 16-bit mode with a 64x prescaler to divide the 1MHz clock, creating a precise 1-second interval
- **Interrupt Service Routine (ISR):** Added `TC5_Handler` that manages the `led_state` toggle and clears the hardware interrupt flags
- **Volatile Memory Management:** Utilized the `volatile` keyword for shared global variables to ensure thread-safety between the asynchronous interrupt and the main program loop
- **Register-Level Clock Routing:** Manually routed the GCLK (Generic Clock) to the TC5 peripheral
### Hardware
  - **Microcontroller:** ATSAMD21E18A
  - **Programmer:** MPLAB Snap In-Circuit Programmer/Debugger
  - **Indicators**: LED with a 330Ω current-limiting resistor
### Software
- **IDE:** MPLAB X IDE
- **Compiler:** XC32 Compiler
### Challenge & Solution
In an interrupt-driven environment, the main loop and timer handler must run at the same time. So, the problem is that the compiler tries to cache variables in registers and if `led_state` was cached in `main()`, it would never see the change made by the timer. So, I resolved this problem by declaring the variable as `volatile`, which forced the CPU to check the RAM address every time which also prevents a Race Condition where the software misses the hardware event.
### Project Structure
- `main.c`: The complete source code that includes the `Timer5_Init` peripheral setup and the `TC5_Handler` interrupt logic
- `GCLK` Setup: Handles the clock synchronization using the `SYNCBUSY` status bit

