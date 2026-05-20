#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "letters.h"

int STEP_PATTERN[4][2] = {
    {1, 0},
    {1, 1},
    {0, 1},
    {0, 0}
};

/*
    CONSTANTS
*/

float STEP_FACTOR = 1; // 1 ... full step, 0.5 ... half step, 0.25 ... quarter step, ...
float FULL_STEP_SIZE_U = 10 * 3.14159265 * 7.5 / 360;
float FULL_STEP_SIZE_V = 10 * 3.14159265 * 7.5 / 360;
float STEP_SIZE_U;
float STEP_SIZE_V;
float SQRT2 = 1.41421356;
float LETTER_SCALE = 0.1f;

/*
    STRUCTS
*/

typedef struct {
    float x;
    float y;
} Coord;

typedef struct {
    int steps_u;
    int steps_v;
    float k; // ratio of steps_v to steps_u (always <= 1)
    int direction_u; // 1 for positive, -1 for negative
    int direction_v; // 1 for positive, -1 for negative
} MotorCommand;


/*
    STATE
*/

Coord currentPos = {0, 0};
MotorCommand MOTOR_COMMAND;

int STEP_INDEX_U = 0;
int STEP_INDEX_V = 0;

int stepCounter = 1;
float remainder = 0.0f;


/* ---------------------------------
   Function declarations
--------------------------------- */

void moveAbs(Coord co);
void moveMotor(Coord co);
void actMotor(MotorCommand cmd);

void sendStepU(int direction);
void sendStepV(int direction);

void TIM2_IRQHandler(void);

void penUp(void);
void penDown(void);

void travel(Coord co);
void draw(Coord co);

void drawLetter(char letter, Coord pos);
void letterRect(char letter, float* x, float* y, float* w, float* h);
void drawText(char* chars, Coord pos);


/*
    MOVEMENT
*/

void moveAbs(Coord co) {
    float Dx = co.x - currentPos.x;
    float Dy = co.y - currentPos.y;

    Coord motorMove;
    motorMove.x = (Dx - Dy) / SQRT2;
    motorMove.y = (Dx + Dy) / SQRT2;

    currentPos = co;

    moveMotor(motorMove);

};

void moveMotor(Coord co) {
    MotorCommand cmd;
    cmd.steps_u = abs((int)roundf(co.x / STEP_SIZE_U));
    cmd.steps_v = abs((int)roundf(co.y / STEP_SIZE_V));

    if (cmd.steps_u == 0 && cmd.steps_v == 0) {
        cmd.k = 0;
    } else {
        cmd.k = (cmd.steps_u > cmd.steps_v) ? (float)cmd.steps_v / cmd.steps_u : (float)cmd.steps_u / cmd.steps_v;
    }

    cmd.direction_u = (co.x > 0) ? 1 : -1;
    cmd.direction_v = (co.y > 0) ? 1 : -1;


    actMotor(cmd);
};


void actMotor(MotorCommand cmd) {
    stepCounter = 1;
    remainder = 0;
    MOTOR_COMMAND = cmd;
};

/*
    MOTOR STEP CONTROL
*/

void sendStepU(int direction) {
    // Code to send step signal to motor U
    STEP_INDEX_U = (STEP_INDEX_U + direction) % len(STEP_PATTERN);
};

void sendStepV(int direction) {
    // Code to send step signal to motor U
    STEP_INDEX_V = (STEP_INDEX_V + direction) % len(STEP_PATTERN);
};

/*
    PEN CONTROL
*/

void penUp() {
    // Code to lift the pen
};

void penDown() {
    // Code to lower the pen
};

/*
    TIMER INTERRUPT
*/

void TIM2_IRQHandler() {
    float smaller_step = stepCounter * MOTOR_COMMAND.k + remainder;

    if (MOTOR_COMMAND.steps_u > MOTOR_COMMAND.steps_v) {
        sendStepU(MOTOR_COMMAND.direction_u);
        MOTOR_COMMAND.steps_u--;

        if (smaller_step > 1) {
            sendStepV(MOTOR_COMMAND.direction_v);
            MOTOR_COMMAND.steps_v--;
            remainder = smaller_step - 1;
            stepCounter = 1;
        } else {
            stepCounter++;
        }
    } else {
        sendStepV(MOTOR_COMMAND.direction_v);
        MOTOR_COMMAND.steps_v--;

        if (smaller_step > 1) {
            sendStepU(MOTOR_COMMAND.direction_u);
            MOTOR_COMMAND.steps_u--;
            remainder = smaller_step - 1;
            stepCounter = 1;
        } else {
            stepCounter++;
        }
    }
};

/* -----------------------------
   Utility movements
----------------------------- */

void travel(Coord co) {
    penUp();
    moveAbs(co);
}

void draw(Coord co) {
    penDown();
    moveAbs(co);
}



/* -------------------------------------
   Draw Letters
------------------------------------- */

void drawLetter(char letter, Coord pos) {
    int letterIndex;

    const int (*coordinates)[2];
    const int* endPts;

    int endpt_i = 0;
    int endpt;

    int i;

    if (letter >= 'a') {
        letterIndex = letter - 'a';
        coordinates = lowercase_letters_coordinates[letterIndex];
        endPts = lowercase_letters_endPts[letterIndex];

    } else {
        letterIndex = letter - 'A';
        coordinates = uppercase_letters_coordinates[letterIndex];
        endPts = uppercase_letters_endPts[letterIndex];
    }

    endpt = endPts[endpt_i];

    for (i = 0; i < 400; i++) {

        if (coordinates[i][0] == 0 && coordinates[i][1] == 0) {
            break;
        }

        Coord co;
        co.x = coordinates[i][0] * LETTER_SCALE + pos.x;
        co.y = coordinates[i][1] * LETTER_SCALE + pos.y;

        if (i == 0 || i - 1 == endpt) {
            travel(co);
        } else {
            draw(co);
        }

        if (i - 2 == endpt) {
            endpt_i++;
            endpt = endPts[endpt_i];
        }
    }
}


void drawText(char* chars, Coord pos) {
    Coord start = pos;
    int i;

    for (i = 0; chars[i] != '\0'; i++) {
        char c = chars[i];

        if (c == '\n') {
            start.x = pos.x;
            start.y -= 100;

        } else if (c == ' ') {
            start.x += 20;

        } else {
            float x, y, w, h;

            drawLetter(c, start);
            letterRect(c, &x, &y, &w, &h);

            start.x += w;
        }
    }
}


void letterRect(
    char letter,
    float* x,
    float* y,
    float* w,
    float* h
) {
    int letterIndex;

    const int (*coordinates)[2];

    int i;

    int min_x;
    int min_y;
    int max_x;
    int max_y;

    if (letter >= 'a') {
        letterIndex = letter - 'a';
        coordinates = lowercase_letters_coordinates[letterIndex];
    } else {

        letterIndex = letter - 'A';
        coordinates = uppercase_letters_coordinates[letterIndex];
    }

    min_x = coordinates[0][0];
    min_y = coordinates[0][1];

    max_x = coordinates[0][0];
    max_y = coordinates[0][1];

    for (i = 0; i < 400; i++) {
        int px = coordinates[i][0];
        int py = coordinates[i][1];

        if (px == 0 && py == 0) {
            break;
        }

        if (px < min_x) min_x = px;
        if (py < min_y) min_y = py;

        if (px > max_x) max_x = px;
        if (py > max_y) max_y = py;
    }

    *x = min_x * LETTER_SCALE;
    *y = min_y * LETTER_SCALE;

    *w = (max_x - min_x) * LETTER_SCALE;
    *h = (max_y - min_y) * LETTER_SCALE;
}


/* ---------------------------------
   Main
--------------------------------- */

int main(void) {

    STEP_SIZE_U = FULL_STEP_SIZE_U * STEP_FACTOR;
    STEP_SIZE_V = FULL_STEP_SIZE_V * STEP_FACTOR;

    travel((Coord){0, 0});

    drawText(
        "Hello World\n"
        "The quick brown fox jumps\n"
        "over the lazy dog",

        (Coord){-350, 0}
    );

    return 0;
}
