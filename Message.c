/* 
 * File:   Message.c
 * Author: jwang456
 * 
 * Purpose: File for encoding and decoding messages for battle boats
 *
 * 
 */
#include <stdint.h>
#include "BattleBoats.h"
#include <string.h>
#include "Message.h"
#include <stdio.h>
#include "BOARD.h"


typedef enum {
    WAITING,
    RECORDING_PAYLOAD,
    RECORDING_CHECKSUM,
} DecodingState;

#define Hex2Bin "0123456789ABCDEF"
#define START_DELIM '$'
#define CHECKSUM_DELIM '*'
#define LAST_DELIM '\n'
#define BOUND1 65
#define BOUND2 57
#define BOUND3 48
#define BOUND4 70
#define checklength 2

static DecodingState decodeState = WAITING;
static char decPayload[MESSAGE_MAX_PAYLOAD_LEN];
static char checkSumString[3];
static int counter = 0;

/**
 * Given a payload string, calculate its checksum
 * 
 * @param payload       //the string whose checksum we wish to calculate
 * @return   //The resulting 8-bit checksum 
 */
uint8_t Message_CalculateChecksum(const char* payload) {
    uint8_t result = 0;
    int x = 0;
    // does a bitwise XOR of everything in the string
    while (x < strlen(payload)) {
        result ^= payload[x];
        x++;
    }
    return result;
}

/**
 * ParseMessage() converts a message string into a BB_Event.  The payload and
 * checksum of a message are passed into ParseMessage(), and it modifies a
 * BB_Event struct in place to reflect the contents of the message.
 * 
 * @param payload       //the payload of a message
 * @param checksum      //the checksum (in string form) of  a message,
 *                          should be exactly 2 chars long, plus a null char
 * @param message_event //A BB_Event which will be modified by this function.
 *                      //If the message could be parsed successfully,
 *                          message_event's type will correspond to the message type and 
 *                          its parameters will match the message's data fields.
 *                      //If the message could not be parsed,
 *                          message_events type will be BB_EVENT_ERROR
 * 
 * @return STANDARD_ERROR if:
 *              the payload does not match the checksum
 *              the checksum string is not two characters long
 *              the message does not match any message template
 *          SUCCESS otherwise
 * 
 * Please note!  sscanf() has a couple compiler bugs that make it a very
 * unreliable tool for implementing this function. * 
 */
int Message_ParseMessage(const char* payload,
        const char* checksum_string, BB_Event * message_event) {
    
    // resets the params
    
    message_event->param0 = 0;
    message_event->param1 = 0;
    message_event->param2 = 0;
    
    char payCopy[MESSAGE_MAX_PAYLOAD_LEN];
    strcpy(payCopy, payload);
    
    // the checksum string has to be length 2, if it is not we have an error
    if (strlen(checksum_string) != 2) {
        message_event->type = BB_EVENT_ERROR;
        return STANDARD_ERROR;
    }
    
    // converts the checksum string to its correct values
    uint8_t totconv = strtoul(checksum_string, NULL, 16);
    
    if (Message_CalculateChecksum(payload) != totconv) {
        message_event->type = BB_EVENT_ERROR;
        return STANDARD_ERROR;
    }
    
    //takes the string at each comma
    char *token = strtok(payCopy, ",");
    
    // we check to see what the first string taken was if it is incorrect we return error
    // expected tokens counts for how many tokens are expected depending on the first string
    int expected_tokens;
    if (strcmp(token, "CHA") == 0) {
        expected_tokens = 1;
        message_event->type = BB_EVENT_CHA_RECEIVED;
    } else if (strcmp(token, "ACC") == 0) {
        expected_tokens = 1;
        message_event->type = BB_EVENT_ACC_RECEIVED;
    } else if (strcmp(token, "SHO") == 0) {
        expected_tokens = 2;
        message_event->type = BB_EVENT_SHO_RECEIVED;
    } else if (strcmp(token, "REV") == 0) {
        expected_tokens = 1;
        message_event->type = BB_EVENT_REV_RECEIVED;
    } else if (strcmp(token, "RES") == 0) {
        expected_tokens = 3;
        message_event->type = BB_EVENT_RES_RECEIVED;
    } else {
        message_event->type = BB_EVENT_ERROR;
        return STANDARD_ERROR;
    }
    
    // we take the next token for as many expected tokens there are
    int iter;
    for (iter = 0; iter < expected_tokens; iter++) {
        token = strtok(NULL, ",");
        
        if (token == NULL) {
            message_event->type = BB_EVENT_ERROR;
            return STANDARD_ERROR;
        }
        
        //change the token to an int and put it in the correct param
        uint16_t p = atoi(token);
        if (iter == 0) {
            message_event->param0 = p;
        } else if (iter == 1) {
            message_event->param1 = p;
        } else if (iter == 2) {
            message_event->param2 = p;
        }
    }
    
    // if the next token is not null, the length was too long and we return error
    token = strtok(NULL, ",");
    if (token) {
        message_event->type = BB_EVENT_ERROR;
        return STANDARD_ERROR;
    }
    
    // everything worked well, success
    return SUCCESS;
    
    
}

/**
 * Encodes the coordinate data for a guess into the string `message`. This string must be big
 * enough to contain all of the necessary data. The format is specified in PAYLOAD_TEMPLATE_COO,
 * which is then wrapped within the message as defined by MESSAGE_TEMPLATE. 
 * 
 * The final length of this
 * message is then returned. There is no failure mode for this function as there is no checking
 * for NULL pointers.
 * 
 * @param message            The character array used for storing the output. 
 *                              Must be long enough to store the entire string,
 *                              see MESSAGE_MAX_LEN.
 * @param message_to_encode  A message to encode
 * @return                   The length of the string stored into 'message_string'.
                             Return 0 if message type is MESSAGE_NONE.
 */
int Message_Encode(char *message_string, Message message_to_encode) {
    char toMessageTemplate[MESSAGE_MAX_PAYLOAD_LEN];
    char finalMessage[MESSAGE_MAX_LEN];
    uint8_t checksum;
    
    // we check for the type of the message
    // based on what message it is, we format the string with the correct number of params
    // we also calculate the checksum and add it in
    switch (message_to_encode.type) {
        case MESSAGE_NONE:
            return 0;
            break;
        case MESSAGE_ACC:
            sprintf(toMessageTemplate, PAYLOAD_TEMPLATE_ACC, message_to_encode.param0);
            checksum = Message_CalculateChecksum(toMessageTemplate);
            sprintf(finalMessage, MESSAGE_TEMPLATE, toMessageTemplate, checksum);
            strcpy(message_string, finalMessage);
            break;
        case MESSAGE_CHA:
            sprintf(toMessageTemplate, PAYLOAD_TEMPLATE_CHA, message_to_encode.param0);
            checksum = Message_CalculateChecksum(toMessageTemplate);
            sprintf(finalMessage, MESSAGE_TEMPLATE, toMessageTemplate, checksum);
            strcpy(message_string, finalMessage);
            break;
        case MESSAGE_SHO:
            sprintf(toMessageTemplate, PAYLOAD_TEMPLATE_SHO, message_to_encode.param0, message_to_encode.param1);
            checksum = Message_CalculateChecksum(toMessageTemplate);
            sprintf(finalMessage, MESSAGE_TEMPLATE, toMessageTemplate, checksum);
            strcpy(message_string, finalMessage);
            break;
        case MESSAGE_REV:
            sprintf(toMessageTemplate, PAYLOAD_TEMPLATE_REV, message_to_encode.param0);
            checksum = Message_CalculateChecksum(toMessageTemplate);
            sprintf(finalMessage, MESSAGE_TEMPLATE, toMessageTemplate, checksum);
            strcpy(message_string, finalMessage);
            break;
        case MESSAGE_RES:
            sprintf(toMessageTemplate, PAYLOAD_TEMPLATE_RES, message_to_encode.param0, message_to_encode.param1, message_to_encode.param2);
            checksum = Message_CalculateChecksum(toMessageTemplate);
            sprintf(finalMessage, MESSAGE_TEMPLATE, toMessageTemplate, checksum);
            strcpy(message_string, finalMessage);
            break;
        case MESSAGE_ERROR:
            break;
    }
    
    // returns the length of the string
    return strlen(message_string);
}


/**
 * Message_Decode reads one character at a time.  If it detects a full NMEA message,
 * it translates that message into a BB_Event struct, which can be passed to other 
 * services.
 * 
 * @param char_in - The next character in the NMEA0183 message to be decoded.
 * @param decoded_message - a pointer to a message struct, used to "return" a message
 *                          if char_in is the last character of a valid message, 
 *                              then decoded_message
 *                              should have the appropriate message type.
 *                          if char_in is the last character of an invalid message,
 *                              then decoded_message should have an ERROR type.
 *                          otherwise, it should have type NO_EVENT.
 * @return SUCCESS if no error was detected
 *         STANDARD_ERROR if an error was detected
 * 
 * note that ANY call to Message_Decode may modify decoded_message.
 */
int Message_Decode(unsigned char char_in, BB_Event * decoded_message_event) {
    
    // we check how the decoding is working
    switch (decodeState) {
        case WAITING:
            // in the first state we wait for a $, if it doesnt arrive or arrives late
            // we return error
            if (char_in == START_DELIM) {
                decoded_message_event->type = BB_EVENT_NO_EVENT;
                decodeState = RECORDING_PAYLOAD;
            } else {
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_INVALID_MESSAGE_TYPE;
                return STANDARD_ERROR;
            }
            break;
        case RECORDING_PAYLOAD:
            // in the second state we add the input to a string until we receive a *
            // if the input is too long we return error
            // if there is a character that is not a number we return error
            if (counter > MESSAGE_MAX_PAYLOAD_LEN) {
                counter = 0;
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_PAYLOAD_LEN_EXCEEDED;
                decodeState = WAITING;
                
                return STANDARD_ERROR;
            } else if (char_in == LAST_DELIM || char_in == START_DELIM) {
                counter = 0;
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_INVALID_MESSAGE_TYPE;
                decodeState = WAITING;
                
                return STANDARD_ERROR;
            } else if (char_in == CHECKSUM_DELIM) {
                // might need to add null here
                decoded_message_event->type = BB_EVENT_NO_EVENT;
                decPayload[counter] = '\0';
                decodeState = RECORDING_CHECKSUM;
                counter = 0;
            } else {
                decoded_message_event->type = BB_EVENT_NO_EVENT;
                decPayload[counter] = char_in;
                counter++;
            }
            break;
        case RECORDING_CHECKSUM:
            // we record this into a string until \n
            // if it is too long we return error
            // if there is an invalid character we return error
            if (counter > checklength) {
                counter = 0;
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_CHECKSUM_LEN_EXCEEDED;
                decodeState = WAITING;
                return STANDARD_ERROR;
            } else if (char_in == LAST_DELIM) {
                if (counter < checklength) {
                    counter = 0;
                    decoded_message_event->type = BB_EVENT_ERROR;
                    decoded_message_event->param0 = BB_ERROR_CHECKSUM_LEN_INSUFFICIENT;
                    decodeState = WAITING;
                    return STANDARD_ERROR;
                } else {
                    checkSumString[counter] = '\0';
                    counter = 0;
                    int result = Message_ParseMessage(decPayload, checkSumString, decoded_message_event);
                    if (result == STANDARD_ERROR) {
                        decoded_message_event->type = BB_EVENT_ERROR;
                        decoded_message_event->param0 = BB_ERROR_MESSAGE_PARSE_FAILURE;
                        decodeState = WAITING;
                        return STANDARD_ERROR;
                    }
                    decodeState = WAITING;
                }
            } else if ((char_in < BOUND1 && char_in > BOUND2) || char_in < BOUND3 || char_in > BOUND4) {
                counter = 0;
                decoded_message_event->type = BB_EVENT_ERROR;
                decoded_message_event->param0 = BB_ERROR_BAD_CHECKSUM;
                decodeState = WAITING;
                return STANDARD_ERROR;
            } else {
                decoded_message_event->type = BB_EVENT_NO_EVENT;
                checkSumString[counter] = char_in;
                counter++;
            }
            break;
    }
    
    return SUCCESS;
}
