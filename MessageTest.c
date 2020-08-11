/* 
 * File:   MessageTest.c
 * Author: jwang456
 * 
 * Purpose: Test harness for Message.c
 *
 * 
 */
#include "xc.h"
#include <stdio.h>
#include "Message.h"
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include "OledDriver.h"
#include "Oled.h"
#include "Ascii.h"

static BB_Event testEvent;
static Message testMessage;
static uint8_t checksum;
static char *payload;
static char *checkString;
static char message[MESSAGE_MAX_LEN];
static char *message2;
static int result;
static int correct = 1;

int main() {
    
    
    
    printf("Beginning the test harness for Message.c\n\n");
    
    // tests for calculate checksum
    printf("Testing Message_CalculateChecksum():\n\n");
    
    // each test:
    // gets the checksum and compares it with the bitwise XOR that is calculated by hand
    payload = "CHA,1";
    checksum = Message_CalculateChecksum(payload);
    if (checksum == 0x57) {
        printf("\tTest 1: passed!\n");
    } else {
        printf("\tTest 1: failed!\n");
        correct = 0;
    }
    
    payload = "SHO,1,2";
    checksum = Message_CalculateChecksum(payload);
    if (checksum == 0x57) {
        printf("\tTest 2: passed!\n");
    } else {
        printf("\tTest 2: failed!\n");
        correct = 0;
    }
    
    payload = "RES,1,2,3";
    checksum = Message_CalculateChecksum(payload);
    if (checksum == 0x58) {
        printf("\tTest 3: passed!\n\n");
    } else {
        printf("\tTest 3: failed! \n\n");
        correct = 0;
    }
    
    // tests for Message_ParseMessage()
    printf("Testing Message_ParseMessage():\n\n");
    
    // each test checks for the return in the bb event
    payload = "CHA,1";
    checkString = "57";
    result = Message_ParseMessage(payload, checkString, &testEvent);
    if (result && testEvent.type == BB_EVENT_CHA_RECEIVED && testEvent.param0 == 1 ) {
        printf("\tTest 1: passed!\n");
    } else {
        printf("\tTest 1: failed!\n");
        correct = 0;
    }
    
    payload = "SHO,1,2";
    checkString = "57";
    result = Message_ParseMessage(payload, checkString, &testEvent);
    if (result && testEvent.type == BB_EVENT_SHO_RECEIVED && testEvent.param0 == 1 && testEvent.param1 == 2) {
        printf("\tTest 2: passed!\n");
    } else {
        printf("\tTest 2: failed!\n");
        correct = 0;
    }
    
    payload = "RES,1,2,3";
    checkString = "58";
    result = Message_ParseMessage(payload, checkString, &testEvent);
    if (result && testEvent.type == BB_EVENT_RES_RECEIVED && testEvent.param0 == 1 && testEvent.param1 == 2
            && testEvent.param2 == 3) {
        printf("\tTest 3: passed!\n");
    } else {
        printf("\tTest 3: failed!\n");
        correct = 0;
    }
    
    // below is error checking for thge same function by putting in invalid things
    checkString = "46";
    result = Message_ParseMessage(payload, checkString, &testEvent);
    if (!result && testEvent.type == BB_EVENT_ERROR) {
        printf("\tTest 4: passed!\n");
    } else {
        printf("\tTest 4: failed!\n");
        correct = 0;
    }
    
    payload = "CHA,1,2";
    checkString = "49";
    result = Message_ParseMessage(payload, checkString, &testEvent);
    if (!result && testEvent.type == BB_EVENT_ERROR) {
        printf("\tTest 5: passed!\n");
    } else {
        printf("\tTest 5: failed!\n");
        correct = 0;
    }
    
    payload = "1";
    checkString = "31";
    result = Message_ParseMessage(payload, checkString, &testEvent);
    if (!result && testEvent.type == BB_EVENT_ERROR) {
        printf("\tTest 6: passed!\n\n");
    } else {
        printf("\tTest 6: failed!\n\n");
        correct = 0;
    }
    
    // testing message encode
    printf("Testing Message_Encode():\n\n");
    
    testMessage.type = MESSAGE_CHA;
    testMessage.param0 = 1;
    Message_Encode(message, testMessage);
    if (strcmp(message, "$CHA,1*57\n") == 0) {
        printf("\tTest 1: passed!\n");
    } else {
        printf("\tTest 1: failed!\n");
        correct = 0;
    }
    
    testMessage.type = MESSAGE_SHO;
    testMessage.param0 = 1;
    testMessage.param1 = 2;
    Message_Encode(message, testMessage);
    if (strcmp(message, "$SHO,1,2*57\n") == 0) {
        printf("\tTest 2: passed!\n");
    } else {
        printf("\tTest 2: failed!\n");
        correct = 0;
    }
    
    testMessage.type = MESSAGE_RES;
    testMessage.param0 = 1;
    testMessage.param1 = 2;
    testMessage.param2 = 3;
    Message_Encode(message, testMessage);
    if (strcmp(message, "$RES,1,2,3*58\n") == 0) {
        printf("\tTest 3: passed!\n\n");
    } else {
        printf("\tTest 3: failed!\n\n");
        correct = 0;
    }
    
    // testing message decode
    printf("Testing Message_Decode():\n\n");
    
    // each test will send the char one by one until its done
    int iter = 0;
    
    message2 = "$CHA,1*57\n";
    for (iter = 0; iter < strlen(message2); iter++) {
        Message_Decode(message2[iter], &testEvent);
    } 
    if (testEvent.type == BB_EVENT_CHA_RECEIVED && testEvent.param0 == 1) {
        printf("\tTest 1: passed!\n");
    } else {
        printf("\tTest 1: failed! %d\n", testEvent.param0);
        correct = 0;
    }
    
    message2 = "$SHO,1,2*57\n";
    for (iter = 0; iter < strlen(message2); iter++) {
        Message_Decode(message2[iter], &testEvent);
    } 
    if (testEvent.type == BB_EVENT_SHO_RECEIVED && testEvent.param0 == 1 && testEvent.param1 == 2) {
        printf("\tTest 2: passed!\n");
    } else {
        printf("\tTest 2: failed! %d\n", testEvent.param0);
        correct = 0;
    }
    
    message2 = "$RES,1,2,3*58\n";
    for (iter = 0; iter < strlen(message2); iter++) {
        Message_Decode(message2[iter], &testEvent);
    } 
    if (testEvent.type == BB_EVENT_RES_RECEIVED && testEvent.param0 == 1 && testEvent.param1 == 2 && testEvent.param2 == 3) {
        printf("\tTest 3: passed!\n");
    } else {
        printf("\tTest 3: failed! %d\n", testEvent.param0);
        correct = 0;
    }
    
    // below are tests for error, such as invalid message
    message2 = "RES,1,2,3*58\n";
    for (iter = 0; iter < strlen(message2); iter++) {
        Message_Decode(message2[iter], &testEvent);
    } 
    if (testEvent.type == BB_EVENT_ERROR) {
        printf("\tTest 4: passed!\n");
    } else {
        printf("\tTest 4: failed! \n");
        correct = 0;
    }
    
    message2 = "RES";
    for (iter = 0; iter < strlen(message2); iter++) {
        Message_Decode(message2[iter], &testEvent);
    } 
    if (testEvent.type == BB_EVENT_ERROR) {
        printf("\tTest 5: passed!\n\n");
    } else {
        printf("\tTest 5: failed!\n\n");
        correct = 0;
    }
    
    if (correct) {
        printf("Final result: all tests passed!\n");
    } else {
        printf("Final result: some tests failed!\n");
    }
    
    while (1);
    
}