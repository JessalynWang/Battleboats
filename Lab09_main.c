// **** Include libraries here ****
// Standard libraries
#include <stdio.h>
#include <string.h>
#include <stdint.h>

//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>

//CE13 standard libraries:
#include "Buttons.h"
#include "Uart1.h"
#include "Oled.h"

// Battleboats Libraries:
#include "BattleBoats.h"
#include "Agent.h"
#include "Negotiation.h"
#include "Message.h"
#include "Field.h"

//The following Macro switches provide useful debugging tools:

//Trace Mode:  Print a trace of events as they are detected:
//#define TRACE_MODE

//Unseeded Mode:  Do not reseed rand, and seed with switches (useful for creating repeatable tests):
//#define UNSEEDED_MODE

// <editor-fold defaultstate="collapsed" desc="macros for trace mode">
#ifndef TRACE_MODE
#define debug_printf(...)
#else
#define debug_printf printf
#endif
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="macros for unseeded mode">
#ifdef UNSEEDED_MODE
#define seed_rand(x) 
#define AgentInit() {srand(SWITCH_STATES()); AgentInit();}
#else
#define seed_rand(x) srand(x)
#endif
// </editor-fold>


//The amount of time between UART updates (in 100ths of a second)
#define TRANSMIT_PERIOD 10

/**
 *  Static data for BattleBoats top level:
 */

//This is the top-level event flag:
static BB_Event battleboatEvent;

//A freerunning timer is used to inject randomness using external events,
//and to throttle the outgoing transmission speed:
static uint32_t freerunning_timer = 0;

/*
 * The Transmission Outgoing submodule has two states.  It can only send one message at a time,
 * so new outgoing messages can only be started when it is in IDLE mode. 
 * 
 * An indexed buffer stores the message until it is completely sent.
 */
enum {
    SENDING, IDLE
} transmission_state = IDLE;
static char outgoing_message_buffer[MESSAGE_MAX_LEN + 1];
static int outgoing_index = 0;

/**
 * This function copies a message into the Transmission outgoing message buffer and begins
 * the sending process.   Once this function is called, the Transmission module
 * switches into the SENDING state.
 * 
 * This function should not be called if it is already in the SENDING state!
 */
void Transmission_StartSendingMessage(const Message * message_to_send)
{
    //this should only be called if sender is in IDLE.
    switch (transmission_state) {
    case SENDING:
        //The Agent state machine should not allow a new message to be started while
        //another message is in the buffer:
        OledClear(OLED_COLOR_BLACK);
        OledDrawString("Fatal Transmission Error!");
        OledUpdate();
        FATAL_ERROR();
    case IDLE:
        //copy message into sending buffer:
        Message_Encode(outgoing_message_buffer, *message_to_send);
        outgoing_index = 0;
        //switch into sending mode:
        transmission_state = SENDING;
    }
}

/**
 * If an outgoing message is in the buffer, this module sends one character each
 * time it is called.  When the message is sent, the module switches to IDLE mode.
 *  
 */
void Transmission_SendChar(void)
{
    //if in SENDING, send one char.  Otherwise, do nothing.
    if (transmission_state != SENDING) return;

    //First send our current char:
    char to_send = outgoing_message_buffer[outgoing_index];
    if (to_send == '\0') {
        //this means our message is fully transmitted.
        battleboatEvent.type = BB_EVENT_MESSAGE_SENT;
        outgoing_index = 0;
        transmission_state = IDLE;
        return;
    } else {
        Uart1WriteByte(to_send);
        outgoing_index++;
    }
}

/**
 * Check for incoming messages.  This module uses Message_Decode to parse messages
 * in the UART input stream, and generates events if any messages are detected.
 **/
void Transmission_ReceiveChar(void)
{
    unsigned char incoming_char;

    //read from the UART (if there is anything to read:
    if (!Uart1HasData()) return;
    Uart1ReadByte(&incoming_char);

    // the commented line below is very handy for debugging Message_Decode
    debug_printf("%c | %02x\n", incoming_char, incoming_char);

    //react to incoming char:
    if (incoming_char != '\0') {
        Message_Decode(incoming_char, &battleboatEvent);
    }

    //also, re-seed our random number using the time:
    seed_rand(rand() + freerunning_timer);
}

//Functions that stringify state names and event names for display.
// <editor-fold defaultstate="collapsed" desc="Trace Mode Functions">
#ifdef TRACE_MODE
#define printcase(x) case x: strcat(tracestr,#x); break;

void TraceState(void)
{
    char tracestr[100] = "---TRACE:  Current state=";
    switch (AgentGetState()) {
        printcase(AGENT_STATE_START);
        printcase(AGENT_STATE_CHALLENGING);
        printcase(AGENT_STATE_ACCEPTING);
        printcase(AGENT_STATE_ATTACKING);
        printcase(AGENT_STATE_DEFENDING);
        printcase(AGENT_STATE_WAITING_TO_SEND);
        printcase(AGENT_STATE_END_SCREEN);
    default: return;
    }
    strcat(tracestr, "\n");
    Uart1WriteData(tracestr, strlen(tracestr));
}

void TraceEvent(void)
{
    char tracestr[100] = "---TRACE:  Detected Event: ";
    switch (battleboatEvent.type) {
        printcase(BB_EVENT_NO_EVENT);
        printcase(BB_EVENT_START_BUTTON);
        printcase(BB_EVENT_RESET_BUTTON);
        printcase(BB_EVENT_SOUTH_BUTTON);
        printcase(BB_EVENT_EAST_BUTTON);
        printcase(BB_EVENT_CHA_RECEIVED);
        printcase(BB_EVENT_ACC_RECEIVED);
        printcase(BB_EVENT_REV_RECEIVED);
        printcase(BB_EVENT_SHO_RECEIVED);
        printcase(BB_EVENT_RES_RECEIVED);
        printcase(BB_EVENT_MESSAGE_SENT);
        printcase(BB_EVENT_ERROR);
    }
    sprintf(tracestr, "%s - %d,%d,%d\n", tracestr,
            battleboatEvent.param0, battleboatEvent.param1, battleboatEvent.param2);
    Uart1WriteData(tracestr, strlen(tracestr));
}

#else
#define TraceEvent() 
#define TraceState()
#endif
// </editor-fold>

int main()
{
    BOARD_Init();

    // Set up UART1 for output.
    // <editor-fold defaultstate="collapsed" desc="Configure Timers and UART">
    Uart1Init(UART_BAUD_RATE); // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a 10ms timer.
    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a .01s timer.

    T2CON = 0; // everything should be off
    T2CONbits.TCKPS = 0b100; // 1:16 prescaler
    PR2 = BOARD_GetPBClock() / 16 / 100; // interrupt at .5s intervals
    T2CONbits.ON = 1; // turn the timer on

    // Set up the timer interrupt with a priority of 4.
    IFS0bits.T2IF = 0; //clear the interrupt flag before configuring
    IPC2bits.T2IP = 4; // priority of  4
    IPC2bits.T2IS = 0; // subpriority of 0 arbitrarily 
    IEC0bits.T2IE = 1; // turn the interrupt on

    // Disable buffering on stdout
    setbuf(stdout, NULL);
    // </editor-fold>

    //Set up LEDs:
    TRISE = 0;
    LATE = 0;

    //initialize CE13 libraries:
    ButtonsInit();
    OledInit();

    //Print a greeting:
    OledDrawString("This is BattleBoats!\nPress BTN4 to\nchallenge, or wait\nfor opponent.");
    OledUpdate();

    //Initialize Agent module:
    AgentInit();

    TraceState();

    //Main loop:
    while (TRUE) {

        //if there is a top-level event, the Agent module should respond to it:
        if (battleboatEvent.type != BB_EVENT_NO_EVENT) {

            TraceEvent();

            Message message_to_send = AgentRun(battleboatEvent);

            TraceState();

            //send a message, if there is one to send:
            if (message_to_send.type != MESSAGE_NONE) {
                Transmission_StartSendingMessage(&message_to_send);
            }

            //consume the event:
            battleboatEvent.type = BB_EVENT_NO_EVENT;

        }

        //update the LEDs to show the agent's current state:
        LATE = (1 << AgentGetState()); //this is very fast so we can do it directly in while(1) loop
    }
}

/**
 * This is the interrupt for the Timer2 peripheral. It just keeps incrementing a counter used to
 * track the time until the first user input.
 */
void __ISR(_TIMER_2_VECTOR, ipl4auto) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;

    // Increment the timer
    freerunning_timer++;

    // Check for any button events
    uint8_t buttonEvent = ButtonsCheckEvents();

    if (buttonEvent & BUTTON_EVENT_4DOWN) battleboatEvent.type = BB_EVENT_START_BUTTON;
    if (buttonEvent & BUTTON_EVENT_3DOWN) battleboatEvent.type = BB_EVENT_EAST_BUTTON;
    if (buttonEvent & BUTTON_EVENT_2DOWN) battleboatEvent.type = BB_EVENT_SOUTH_BUTTON;
    if (buttonEvent & BUTTON_EVENT_1DOWN) battleboatEvent.type = BB_EVENT_RESET_BUTTON;

    //also, re-seed our random number using the time:
    if (buttonEvent) seed_rand(rand() + freerunning_timer);

    //every TRANSMIT_PERIOD cycles, attempt to run the transmission module.
    if (freerunning_timer % TRANSMIT_PERIOD == 0) {
        Transmission_SendChar();
        if (battleboatEvent.type == BB_EVENT_MESSAGE_SENT) return;
        Transmission_ReceiveChar();
    }

}