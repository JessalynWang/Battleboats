#include "Field.h"
#include <stdio.h>
#include <stdlib.h>
#include "BOARD.h"

#define dos 2

// This simply prints the representation of both fields
void FieldPrint_UART(Field *own_field, Field * opp_field)
{
    // first field
    int x, y;
    for (x = 0; x < FIELD_ROWS; x++) {
        for (y = 0; y < FIELD_COLS; y++) {
            printf(" %d", own_field->grid[x][y]);
        }
        printf("\n");
    }
    printf("\n");
    // second field
    for (x = 0; x < FIELD_ROWS; x++) {
        for (y = 0; y < FIELD_COLS; y++) {
            printf(" %d", opp_field->grid[x][y]);
        }
        printf("\n");
    }
}

// This basically initializes two passed field structs for the beginning 
// of play
void FieldInit(Field *own_field, Field * opp_field)
{
    int i, j;
    for (i = 0; i < FIELD_ROWS; i++) {
        for (j = 0; j < FIELD_COLS; j++) {
            own_field->grid[i][j] = FIELD_SQUARE_EMPTY;
            opp_field->grid[i][j] = FIELD_SQUARE_UNKNOWN;
        }
    }
    opp_field->smallBoatLives = FIELD_BOAT_SIZE_SMALL;
    opp_field->mediumBoatLives = FIELD_BOAT_SIZE_MEDIUM;
    opp_field->largeBoatLives = FIELD_BOAT_SIZE_LARGE;
    opp_field->hugeBoatLives = FIELD_BOAT_SIZE_HUGE;

    own_field->smallBoatLives = 0;
    own_field->mediumBoatLives = 0;
    own_field->largeBoatLives = 0;
    own_field->hugeBoatLives = 0;
}

SquareStatus FieldGetSquareStatus(const Field *f, uint8_t row, uint8_t col)
{
    if (row > FIELD_ROWS || row < 0 || col > FIELD_COLS || col < 0) {
        return FIELD_SQUARE_INVALID;
    } else {
        return f->grid[row][col];
    }
}

SquareStatus FieldSetSquareStatus(Field *f, uint8_t row, uint8_t col, SquareStatus p)
{
    SquareStatus prevstat = f->grid[row][col];
    f->grid[row][col] = p;
    return prevstat;
}

uint8_t FieldAddBoat(Field *own_field, uint8_t row, uint8_t col, BoatDirection dir, BoatType boat_type)
{

    int i, length = 0;
    SquareStatus type = FIELD_SQUARE_INVALID;
    switch (boat_type) {
    case FIELD_BOAT_TYPE_SMALL:
        length = FIELD_BOAT_SIZE_SMALL;
        type = FIELD_SQUARE_SMALL_BOAT;
        break;
    case FIELD_BOAT_TYPE_MEDIUM:
        length = FIELD_BOAT_SIZE_MEDIUM;
        type = FIELD_SQUARE_MEDIUM_BOAT;
        break;
    case FIELD_BOAT_TYPE_LARGE:
        length = FIELD_BOAT_SIZE_LARGE;
        type = FIELD_SQUARE_LARGE_BOAT;
        break;
    case FIELD_BOAT_TYPE_HUGE:
        length = FIELD_BOAT_SIZE_HUGE;
        type = FIELD_SQUARE_HUGE_BOAT;
        break;
    default:
        return STANDARD_ERROR;
    }

    if (row >= FIELD_ROWS || col >= FIELD_COLS || row < 0 || col < 0) {
        return STANDARD_ERROR;
    }

    if (dir == FIELD_DIR_EAST) {

        if (col + length - 1 >= FIELD_COLS) {
            return STANDARD_ERROR;
        }

        for (i = 0; i < (length); i++) {
            if (own_field->grid[row][col + i] != FIELD_SQUARE_EMPTY) {
                return STANDARD_ERROR;
            }
        }

        for (i = 0; i < (length); i++) {
            own_field->grid[row][col + i] = type;
        }

        if (boat_type == FIELD_BOAT_TYPE_SMALL) {
            own_field->smallBoatLives = FIELD_BOAT_SIZE_SMALL;
        } else if (boat_type == FIELD_BOAT_TYPE_MEDIUM) {
            own_field->mediumBoatLives = FIELD_BOAT_SIZE_MEDIUM;
        } else if (boat_type == FIELD_BOAT_TYPE_LARGE) {
            own_field->largeBoatLives = FIELD_BOAT_SIZE_LARGE;
        } else if (boat_type == FIELD_BOAT_TYPE_HUGE) {
            own_field->hugeBoatLives = FIELD_BOAT_SIZE_HUGE;
        } else {
            return STANDARD_ERROR;
        }
        return SUCCESS;
    } else if (dir == FIELD_DIR_SOUTH) {

        if (row + length - 1 >= FIELD_ROWS) {
            return STANDARD_ERROR;
        }

        for (i = 0; i < (length); i++) {
            if (own_field->grid[row + i][col] != FIELD_SQUARE_EMPTY) {
                return STANDARD_ERROR;
            }
        }

        for (i = 0; i < (length); i++) {
            own_field->grid[row + i][col] = type;
        }

        if (boat_type == FIELD_BOAT_TYPE_SMALL) {
            own_field->smallBoatLives = FIELD_BOAT_SIZE_SMALL;
        } else if (boat_type == FIELD_BOAT_TYPE_MEDIUM) {
            own_field->mediumBoatLives = FIELD_BOAT_SIZE_MEDIUM;
        } else if (boat_type == FIELD_BOAT_TYPE_LARGE) {
            own_field->largeBoatLives = FIELD_BOAT_SIZE_LARGE;
        } else if (boat_type == FIELD_BOAT_TYPE_HUGE) {
            own_field->hugeBoatLives = FIELD_BOAT_SIZE_HUGE;
        } else {
            return STANDARD_ERROR;
        }
        return SUCCESS;
    } else {
        return STANDARD_ERROR;
    }
}

SquareStatus FieldRegisterEnemyAttack(Field *own_field, GuessData *opp_guess)
{
    SquareStatus prevstst = own_field->grid[opp_guess->row][opp_guess->col];

    switch (own_field->grid[opp_guess->row][opp_guess->col]) {
    case FIELD_BOAT_SIZE_SMALL:
        if ((own_field->smallBoatLives) > 0) {
            opp_guess->result = RESULT_HIT;
        } else {
            opp_guess->result = RESULT_SMALL_BOAT_SUNK;
        }
        own_field->grid[opp_guess->row][opp_guess->col] = RESULT_HIT;

    case FIELD_BOAT_SIZE_MEDIUM:
        if ((own_field->mediumBoatLives) > 0) {
            opp_guess->result = RESULT_HIT;
        } else {
            opp_guess->result = RESULT_MEDIUM_BOAT_SUNK;
        }
        own_field->grid[opp_guess->row][opp_guess->col] = RESULT_HIT;

    case FIELD_BOAT_SIZE_LARGE:
        if ((own_field->largeBoatLives) > 0) {
            opp_guess->result = RESULT_HIT;
        } else {
            opp_guess->result = RESULT_LARGE_BOAT_SUNK;
        }
        own_field->grid[opp_guess->row][opp_guess->col] = RESULT_HIT;

    case FIELD_BOAT_SIZE_HUGE:
        if ((own_field->hugeBoatLives) > 0) {
            opp_guess->result = RESULT_HIT;
        } else {
            opp_guess->result = RESULT_HUGE_BOAT_SUNK;
        }
        own_field->grid[opp_guess->row][opp_guess->col] = RESULT_HIT;
    }
    return prevstst;
}

SquareStatus FieldUpdateKnowledge(Field *opp_field, const GuessData *own_guess)
{
    SquareStatus prevalue = opp_field->grid[own_guess->row][own_guess->col];

    if (own_guess->result == RESULT_HIT) {
        opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
    }

    if (own_guess->result == RESULT_MISS) {
        opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_EMPTY;
    }

    if (own_guess->result == RESULT_SMALL_BOAT_SUNK) {
        opp_field->smallBoatLives = 0;
        opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
    }

    if (own_guess->result == RESULT_MEDIUM_BOAT_SUNK) {
        opp_field->mediumBoatLives = 0;
        opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
    }

    if (own_guess->result == RESULT_LARGE_BOAT_SUNK) {
        opp_field->largeBoatLives = 0;
        opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
    }

    if (own_guess->result == RESULT_HUGE_BOAT_SUNK) {
        opp_field->hugeBoatLives = 0;
        opp_field->grid[own_guess->row][own_guess->col] = FIELD_SQUARE_HIT;
    }
    return prevalue;
}

uint8_t FieldGetBoatStates(const Field *f)
{
    uint8_t shipaon = 0;

    if (f->smallBoatLives >= 0)
        shipaon |= FIELD_BOAT_STATUS_SMALL;

    if (f->mediumBoatLives >= 0)
        shipaon |= FIELD_BOAT_STATUS_MEDIUM;

    if (f->largeBoatLives >= 0)
        shipaon |= FIELD_BOAT_STATUS_LARGE;

    if (f->hugeBoatLives >= 0)
        shipaon |= FIELD_BOAT_STATUS_HUGE;

    return shipaon;
}

uint8_t FieldAIPlaceAllBoats(Field *own_field)
{
    int smallflag = FALSE;
    int mediumflag = FALSE;
    int largeflag = FALSE;
    int hugeflag = FALSE;

    uint8_t row;
    uint8_t col;
    uint8_t dire;

    while (smallflag == FALSE || smallflag == SUCCESS) {
        dire = rand() % dos;
        col = rand() % FIELD_COLS;
        row = rand() % FIELD_ROWS;

        if (hugeflag == FALSE) {
            hugeflag = FieldAddBoat(own_field, row, col, dire, FIELD_BOAT_TYPE_HUGE);
        }
        if (hugeflag == SUCCESS && largeflag == FALSE && mediumflag == FALSE && smallflag == FALSE) {
            largeflag = FieldAddBoat(own_field, row, col, dire, FIELD_BOAT_TYPE_LARGE);
        } else if (largeflag == SUCCESS && mediumflag == FALSE && smallflag == FALSE) {
            mediumflag = FieldAddBoat(own_field, row, col, dire, FIELD_BOAT_TYPE_MEDIUM);
        } else if (mediumflag == SUCCESS && smallflag == FALSE) {
            smallflag = FieldAddBoat(own_field, row, col, dire, FIELD_BOAT_TYPE_SMALL);
        } else if (smallflag == SUCCESS) {
            return SUCCESS;
        }
    }
    return STANDARD_ERROR;
}

GuessData FieldAIDecideGuess(const Field *opp_field)
{
    while (1) {
        int row = rand() % FIELD_ROWS;
        int col = rand() % FIELD_COLS;
        if (opp_field->grid[row][col] == FIELD_SQUARE_UNKNOWN) {
            GuessData gData = {row, col, RESULT_MISS};
            return gData;
        }
    }
}