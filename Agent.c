/* 
 * File:   Agent.c
 * Author: jwang456
 * 
 * Purpose: File for the main state machine of battle boats
 *
 * 
 */
#include "Agent.h"
#include "Ascii.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "Message.h"
#include "BattleBoats.h"
#include "BOARD.h"
#include "Buttons.h"
#include "CircularBuffer.h"
#include "FieldOled.h"
#include "Oled.h"
#include "OledDriver.h"
#include "Uart1.h"
#include "Negotiation.h"
#include "Field.h"

struct Agent {
    AgentState state;
    NegotiationData secret;
    NegotiationData hash;
    Field own;
    Field other;
    Message message;
};

static struct Agent agent;
static int turnCount = 0;;
static char *errorMSG;
static FieldOledTurn turn;

#define RAND_SIZE 0xFFFF
#define ALL_SUNK 0b00000000

/**
 * The Init() function for an Agent sets up everything necessary for an agent before the game
 * starts.  At a minimum, this requires:
 *   -setting the start state of the Agent SM.
 *   -setting turn counter to 0
 * If you are using any other persistent data in Agent SM, that should be reset as well.
 * 
 * It is not advised to call srand() inside of AgentInit.  
 *  */
void AgentInit(void) {
    
    // set the state to start and turn count to 0
    agent.state = AGENT_STATE_START;
    turnCount = 0;
    turn = FIELD_OLED_TURN_NONE;
    

}

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
Message AgentRun(BB_Event event) {
    switch (event.type) {
        case BB_EVENT_START_BUTTON:
            // if the state is start, we set the fields up for playing, generate the hash,
            // and go to the challenge mode
            if (agent.state == AGENT_STATE_START) {
                agent.secret = rand() & RAND_SIZE;
                agent.message.param0 = NegotiationHash(agent.secret);
                agent.message.type = MESSAGE_CHA;
                FieldInit(&agent.own, &agent.other);
                
                FieldAIPlaceAllBoats(&agent.own);
                
                agent.state = AGENT_STATE_CHALLENGING;
            } else {
                agent.message.type = MESSAGE_NONE;
            }
            break;
        case BB_EVENT_RESET_BUTTON:
            // TODO:
            //  make sure this is okay stylistically
            // here we reset all the data that needs resetting, and output a new screen
            agent.message.type = MESSAGE_NONE;
            char *tempWelcome = "Press BTN4 to start \nor wait for challenge\n";
            OledClear(OLED_COLOR_BLACK);
            OledDrawString(tempWelcome);
            OledUpdate();
            AgentInit();
            return agent.message;
            break;
        case BB_EVENT_CHA_RECEIVED:
            // in this mode we received a challenge, we generate the random number and send
            // it to the challenger
            if (agent.state == AGENT_STATE_START) {
                agent.secret = rand() & RAND_SIZE; // see agent.secret for EVENT_START_BUTTON
                // TODO:
                //  send ACC
                agent.hash = event.param0;
                agent.message.type = MESSAGE_ACC;
                agent.message.param0 = agent.secret;
                
                FieldInit(&agent.own, &agent.other);
                FieldAIPlaceAllBoats(&agent.own);
                agent.state = AGENT_STATE_ACCEPTING;
                
            } else {
                agent.message.type = MESSAGE_NONE;
            }
            break;
        case BB_EVENT_ACC_RECEIVED:
            // if we are challenger and we receive the other random number,
            // we run the coin flip to see who is attack and go to a state
            // dependant on attack/defending
            if (agent.state == AGENT_STATE_CHALLENGING) {
                // send REV
                agent.message.type = MESSAGE_REV;
                agent.message.param0 = agent.secret;
                // fix this
                NegotiationOutcome outcome = NegotiateCoinFlip(agent.secret, event.param0);
                if (outcome == HEADS) {
                    turn = FIELD_OLED_TURN_MINE;
                    agent.state = AGENT_STATE_WAITING_TO_SEND;
                } else {
                    // else if tails
                    turn = FIELD_OLED_TURN_THEIRS;
                    agent.state = AGENT_STATE_DEFENDING;
                }
                
            } else {
                agent.message.type = MESSAGE_NONE;
            }
            break;
        case BB_EVENT_REV_RECEIVED:
            // if we are accepter and we receive the other random number,
            // we run the coin flip to see who is attack and go to a state
            // dependant on attack/defending
            // we also check to make sure the challenger hasnt cheated
            if (agent.state == AGENT_STATE_ACCEPTING) {
                NegotiationOutcome outcome = NegotiateCoinFlip(agent.secret, event.param0);
                if (NegotiationVerify(event.param0, agent.hash) == FALSE) {
                    char *cheat = "cheating message here, press reset button to start again\n";
                    OledDrawString(cheat);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    agent.message.type = MESSAGE_NONE;
                    return agent.message;
                }
                if (outcome == TAILS) {
                    // determine and send shot here
                    turn = FIELD_OLED_TURN_MINE;
                    GuessData guess = FieldAIDecideGuess(&agent.other);
                    agent.message.type = MESSAGE_SHO;
                    agent.message.param0 = guess.row;
                    agent.message.param1 = guess.col;
                    agent.state = AGENT_STATE_ATTACKING;
                } else {
                    // else if tails
                    agent.message.type = MESSAGE_NONE;
                    turn = FIELD_OLED_TURN_THEIRS;
                    agent.state = AGENT_STATE_DEFENDING;
                }
                // detect cheating
            } else {
                agent.message.type = MESSAGE_NONE;
            }
            break;
        case BB_EVENT_SHO_RECEIVED:
            // if we are defending and we just received the enemy attack
            // we register the attack and send the result back to the other boat
            // if we lost from this attack we display defeat
            if (agent.state == AGENT_STATE_DEFENDING) {
                GuessData opGuess;
                opGuess.row = event.param0;
                opGuess.col = event.param1;
                FieldRegisterEnemyAttack(&agent.own, &opGuess);
                
                agent.message.type = MESSAGE_RES;
                agent.message.param0 = event.param0;
                agent.message.param1 = event.param1;
                agent.message.param2 = opGuess.result;
                
                if (FieldGetBoatStates(&agent.own) == ALL_SUNK) {
                    agent.message.type = MESSAGE_NONE;
                    char *defeat = "defeat :(\n";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(defeat);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    return agent.message;
                } else {
                    turn = FIELD_OLED_TURN_MINE;
                    agent.state = AGENT_STATE_WAITING_TO_SEND;
                }
                
            } else {
                agent.message.type = MESSAGE_NONE;
            }
            break;
        case BB_EVENT_RES_RECEIVED:
            // if we are attacking
            // we just got the result of our guess and we add the data to what we know already
            // if we won we display victory
            // otherwise we go to defending and wait for the enemy attack
            if (agent.state == AGENT_STATE_ATTACKING) {
                // check for victory
                // otherwise go to defending
                GuessData ownGuess;
                ownGuess.row = event.param0;
                ownGuess.col = event.param1;
                ownGuess.result = event.param2;
                FieldUpdateKnowledge(&agent.other, &ownGuess);
                if (FieldGetBoatStates(&agent.other) == ALL_SUNK) {
                    agent.message.type = MESSAGE_NONE;
                    char *victory = "victory :)\n";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(victory);
                    OledUpdate();
                    agent.state = AGENT_STATE_END_SCREEN;
                    return agent.message;
                } else {
                    agent.message.type = MESSAGE_NONE;
                    turn = FIELD_OLED_TURN_THEIRS;
                    agent.state = AGENT_STATE_DEFENDING;
                }
            } else {
                agent.message.type = MESSAGE_NONE;
            }
            break;
        case BB_EVENT_MESSAGE_SENT:
            // if we sent a message just now and are about to attack,
            // we decide out guess and send it to the other player
            if (agent.state == AGENT_STATE_WAITING_TO_SEND) {
                turnCount++;
                GuessData guess = FieldAIDecideGuess(&agent.other);
                
                agent.message.type = MESSAGE_SHO;
                agent.message.param0 = guess.row;
                agent.message.param1 = guess.col;
                
                agent.state = AGENT_STATE_ATTACKING;
                
            } else {
                agent.message.type = MESSAGE_NONE;
            }
            break;
        case BB_EVENT_ERROR:
            // error messages for debugging
            switch (event.param0) {
                case BB_ERROR_BAD_CHECKSUM:
                    errorMSG = "Bad checksum";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMSG);
                    OledUpdate();
                    break;
                case BB_ERROR_PAYLOAD_LEN_EXCEEDED:
                    errorMSG = "Payload len exceeded";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMSG);
                    OledUpdate();
                    break;
                case BB_ERROR_CHECKSUM_LEN_EXCEEDED:
                    errorMSG = "checksum len exceeded";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMSG);
                    OledUpdate();
                    break;
                case BB_ERROR_CHECKSUM_LEN_INSUFFICIENT:
                    errorMSG = "checksum len insufficient";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMSG);
                    OledUpdate();
                    break;
                case BB_ERROR_INVALID_MESSAGE_TYPE:
                    errorMSG = "invalid msg type";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMSG);
                    OledUpdate();
                    break;
                case BB_ERROR_MESSAGE_PARSE_FAILURE:
                    errorMSG = "message parse failure";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMSG);
                    OledUpdate();
                    break;
                default:
                    errorMSG = "message parse failure";
                    OledClear(OLED_COLOR_BLACK);
                    OledDrawString(errorMSG);
                    OledUpdate();
                    break;
            }
            
            agent.state = AGENT_STATE_END_SCREEN;
            agent.message.type = MESSAGE_ERROR;
            return agent.message;
            
            break;
        case BB_EVENT_NO_EVENT: case BB_EVENT_SOUTH_BUTTON: case BB_EVENT_EAST_BUTTON:
            // chose not to do the extra credit, so in these cases nothing happens
            agent.message.type = MESSAGE_NONE;
            break;
    }
    
    // if everything goes smoothly, we update the screen as it is
    OledClear(OLED_COLOR_BLACK);
    FieldOledDrawScreen(&agent.own, &agent.other, turn, turnCount);
    OledUpdate();
    return agent.message;
}

/** * 
 * @return Returns the current state that AgentGetState is in.  
 * 
 * This function is very useful for testing AgentRun.
 */
AgentState AgentGetState(void) {
    return agent.state;
}

/** * 
 * @param Force the agent into the state given by AgentState
 * 
 * This function is very useful for testing AgentRun.
 */
void AgentSetState(AgentState newState) {
    agent.state = newState;
}


