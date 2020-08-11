#include <stdio.h>
#include <stdlib.h>
#include "Agent.h"
#include "BattleBoats.h"
int main() {    
    printf("Testing AgentSetSate and AgentGetState:\n");    
    
    int testercount = 0;
    
    AgentSetState(AGENT_STATE_START);
    if(AgentGetState() == AGENT_STATE_START) testercount++;
    AgentSetState(AGENT_STATE_CHALLENGING);
    if(AgentGetState() == AGENT_STATE_CHALLENGING) testercount++;
    AgentSetState(AGENT_STATE_ACCEPTING);
    if(AgentGetState() == AGENT_STATE_ACCEPTING) testercount++;
    AgentSetState(AGENT_STATE_ATTACKING);
    if(AgentGetState() == AGENT_STATE_ATTACKING) testercount++;
    AgentSetState(AGENT_STATE_DEFENDING);
    if(AgentGetState() == AGENT_STATE_DEFENDING) testercount++;
    AgentSetState(AGENT_STATE_WAITING_TO_SEND);
    if(AgentGetState() == AGENT_STATE_WAITING_TO_SEND) testercount++;
    AgentSetState(AGENT_STATE_END_SCREEN);
    if(AgentGetState() == AGENT_STATE_END_SCREEN) testercount++;
    if(testercount == 7) printf("SUCCESS\n");
  
    printf("Testing AgentInIt:\n");
    AgentInit();
    if(AgentGetState() == AGENT_STATE_START) printf("SUCCESS\n");    
    
    
    printf("Testing AgentRun:\n");
    testercount = 0;
    BB_Event event;
    event.type = BB_EVENT_START_BUTTON;
    event.param0 = 0;
    event.param1 = 0;
    event.param2 = 0;
    AgentRun(event);    
    if(AgentGetState() == AGENT_STATE_CHALLENGING) testercount++;        
    if(testercount == 1) printf("\nSUCCESS\n");    
    
    while(1);    
}