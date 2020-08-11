#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include "BattleBoats.h"


/** According to the NMEA standard, messages cannot be longer than 82,
 * including the delimiters $, *, and \n.
 */
#define MESSAGE_MAX_LEN 82

/** The maximum payload length is the maximum message length,
 * -1 start delimiter ($)
 * -1 checksum delimiter (*),
 * -2 checksum characters, 
 * -1 end delimiter (\n) 
 */
#define MESSAGE_MAX_PAYLOAD_LEN (MESSAGE_MAX_LEN - 1 - 1 - 2 - 1)

/*NMEA also defines a specific  checksum length*/
#define MESSAGE_CHECKSUM_LEN 2

/** 
 * The types of messages that can be sent or received:
 */
typedef enum {
    MESSAGE_NONE, //used if no message is to be sent
    MESSAGE_CHA,
    MESSAGE_ACC,
    MESSAGE_REV,
    MESSAGE_SHO,
    MESSAGE_RES,
            
    //while not required, an error message can be a useful debugging tool:
    MESSAGE_ERROR = -1, 
} MessageType;

/**
 * Messages follow this struct:
 */
typedef struct {
    MessageType type;
    unsigned int param0;
    unsigned int param1;
    unsigned int param2;
} Message;



/** Message payloads will have the following syntax. 
 * Each (almost*) follows the NMEA0183 syntax for message payloads:  
 *		The first three characters describe the message type
 * 		Zero or more comma-separated fields follow, containing various kinds of data
 *
 * (* true NEMA0183 payloads begin with two characters that describe the "talker", which we 
 *    omit from the BattleBoats protocol)
 */
#define PAYLOAD_TEMPLATE_CHA "CHA,%u" 		// Challenge message:  		hash_a (see protocol)
#define PAYLOAD_TEMPLATE_ACC "ACC,%u" 		// Accept message:	 		B (see protocol)
#define PAYLOAD_TEMPLATE_REV "REV,%u"       // Reveal message: 			A (see protocol)
#define PAYLOAD_TEMPLATE_SHO "SHO,%d,%d"    // Shot (guess) message: 	row, col
#define PAYLOAD_TEMPLATE_RES "RES,%u,%u,%u" // Result message: 			row, col, GuessResult


/** 
 * NEMA0183 messages wrap the payload with a start delimiter, 
 * a checksum to verify the contents of   
 * the message in case of transmission errors, and an end delimiter.
 * This template defines the wrapper.
 * Note that it uses printf-style tokens so that it can be used with sprintf().
 * 
 * Here is an example message:
 *                 
 * 1 start delimiter    (a literal $)
 * 2 payload            (any string, represented by %s in the template)
 * 3 checksum delimiter (a literal *)
 * 4 checksum			(two ascii characters representing hex digits, %02x in the template)
 * 5 end delimiter      (a literal \n)
 * 
 * example message:      1       3  5      
 *						 v       v  vv
 *                       $SHO,2,9*5F\n
 *                        ^^^^^^^ ^^      
 *                          2     4      
 * 
 * Note that 2 and 4 correspond to %s and %02x in the template.
 * 
 * Also note that valid BattleBoats messages use 
 * strictly upper-case letters, so $SHO,2,3*5f\n is an invalid message.
 */
#define MESSAGE_TEMPLATE "$%s*%02X\n"

/**
 * Given a payload string, calculate its checksum
 * 
 * @param payload       //the string whose checksum we wish to calculate
 * @return   //The resulting 8-bit checksum 
 */
uint8_t Message_CalculateChecksum(const char* payload);

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
        const char* checksum_string, BB_Event * message_event);

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
int Message_Encode(char *message_string, Message message_to_encode);


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
int Message_Decode(unsigned char char_in, BB_Event * decoded_message_event);


#endif // MESSAGE_H
