/* 
 * File:   NegotiationTest.c
 * Author: jwang456
 * 
 * Purpose: Test harness for Negotiation.c
 *
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "Negotiation.h"
#include "Uart1.h"
#include "BOARD.h"

int main() {
    
    int iter = 0;
    
    printf("\nTesting negotiation:\n");
    
    printf("\nTesting NegotiationHash()\n");
    
    // testing normal sized hashes
    NegotiationData secret = 11;
    NegotiationData correctHash = (secret * secret) % 0xBEEF;
    if (NegotiationHash(secret) == correctHash) {
        printf("\tPassed basic NegotiationHash() \n");
        iter++;
    } else {
        printf("\tFailed NegotiationHash()\n");
    }
    
    // testing hashes with overflow
    if (NegotiationHash(65535) == 34011) {
        printf("\tPassed overflow value for NegotiationHash()\n");
        iter++;
    } else {
        printf("\tfailed\n");
    }
    
    if (NegotiationHash(0) == 0) {
        printf("\tPassed zero value for NegotiationHash()\n");
        iter++;
    } else {
        printf("\tfailed\n");
    }
    
    // making sure it can detect cheating and correctness
    printf("\nTesting NegotiationVerify()\n");
    
    NegotiationData cheat = 20;
    if (NegotiationVerify(cheat, correctHash) == FALSE &&
            NegotiationVerify(secret, correctHash) == TRUE) {
        printf("\tPassed basic NegotiationVerify()\n");
        iter++;
    } else {
        printf("\tFailed NegotiationVerify()\n");
    }
    
    if (NegotiationVerify(65535, 34011) == TRUE) {
        printf("\tPassed overflow in NegotiationVerify()\n");
        iter++;
    } else {
        printf("\tfailed\n");
    }
    
    // making sure both heads and tails work
    printf("\nTesting NegotiationVerify()\n");
    
    if (NegotiateCoinFlip(secret, cheat) == HEADS) {
        printf("\tPassed NegotiateCoinFlip()\n");
        iter++;
    } else {
        printf("\tFailed NegotiateCoinFlip\n");
    }
    
    if (NegotiateCoinFlip(10, 10) == TAILS) {
        printf("\tPassed NegotiateCoinFlip()\n");
        iter++;
    } else {
        printf("\tFailed NegotiateCoinFlip\n");
    }
    
    printf("\nDone testing!\n");
    printf("\nFinal result: %d/7 tests passed!\n", iter);
    

    while(1);
}
