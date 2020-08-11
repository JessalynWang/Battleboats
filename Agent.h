    #ifndef AGENT_H
#define AGENT_H

#include <stdint.h>
#include "Message.h"
#include "BattleBoats.h"

/**
 * Defines the various states used within the agent state machines. All states should be used
 * within a valid agent implementation. Additionally there is no need for states outside of
 * those defined here.
 */
typedef enum {
    AGENT_STATE_START, //0
    AGENT_STATE_CHALLENGING, //1
    AGENT_STATE_ACCEPTING, //2
    AGENT_STATE_ATTACKING, //3
    AGENT_STATE_DEFENDING, //4
    AGENT_STATE_WAITING_TO_SEND, //5
    AGENT_STATE_END_SCREEN, //6

    //If implementing a human agent, you will need a state to setup boats:
    AGENT_STATE_SETUP_BOATS, //7
} AgentState;

/**
 * The Init() function for an Agent sets up everything necessary for an agent before the game
 * starts.  At a minimum, this requires:
 *   -setting the start state of the Agent SM.
 *   -setting turn counter to 0
 * If you are using any other persistent data in Agent SM, that should be reset as well.
 * 
 * It is not advised to call srand() inside of AgentInit.  
 *  */
void AgentInit(void);

/**
 * AgentRun evolves the Agent state machine in response to an event.
 * 
 * @param  The most recently detected event
 * @return Message, a Message struct to send to the opponent. 
 * 
 * If the returned Message struct is a valid message
 * (that is, not of type MESSAGE_NONE), then it will be
 * passed to the transmission module and sent via UART.
 * This is handled at the top level! AgentRun is ONLY responsible 
 * for generating the Message struct, not for encoding or sending it.
 */
Message AgentRun(BB_Event event);

/** * 
 * @return Returns the current state that AgentGetState is in.  
 * 
 * This function is very useful for testing AgentRun.
 */
AgentState AgentGetState(void);

/** * 
 * @param Force the agent into the state given by AgentState
 * 
 * This function is very useful for testing AgentRun.
 */
void AgentSetState(AgentState newState);

#endif // AGENT_H
