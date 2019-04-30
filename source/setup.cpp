#include "setup.h"

#include "mbed.h"
#include "storage.h"

DigitalOut enableDebugPortRX(PB_15, 0); //enable RX on debug port
DigitalOut debugPortForceOff(PD_10, 1); // uses inverse logic!
DigitalOut debugPortForceOn(PD_11, 1);
DigitalOut disableVc(PC_6, 0);  //enable vcontrolled
DigitalOut enable3v3(PG_1, 1);
PwmOut buzzer(PF_7); 

#ifdef BINBEAT_V10
DigitalOut enable5v(PB_1, 1);
#endif // BINBEAT_V10
#ifdef BINBEAT_V12
DigitalOut enable5v(PA_1, 1);
#endif // BINBEAT_V12


InterruptIn irq(PA_0);


void powerup() {
    wait_ms(10);
    printf("Initializing board\r\n");
    // Setup the erase event on button press, use the event queue
    // to avoid running in interrupt context
    irq.fall(mbed_event_queue()->event(erase));
    wait_ms(1000);
    printf("Board ready\r\n");
}

void buzzzz()
{
    buzzer.period(1/(float)2050); //xx seconds period
	buzzer = 0.1; //duty cycle, relative to period
}

void mute()
{
    buzzer = 0;
}

void beep()
{
    buzzzz();
    wait_ms(100);
    mute();
}
