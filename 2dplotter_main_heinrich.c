
STEP_SIZE_X = 1;
STEP_SIZE_Y = 1;

typedef struct {
    float x;
    float y;
} Coord;

Coord startPos;
Coord endPos;

float sqrt2 = 1.41421356;

void moveAbs(Coord co) {
    endPos = co;
    
    float Dx = co.x - startPos.x;
    float Dy = co.y - startPos.y;

    Coord motorMove;
    motorMove.x = (Dx - Dy) / sqrt2;
    motorMove.y = (Dx + Dy) / sqrt2;

    moveMotor(motorMove);
};

void moveMotor(Coord co) {
    int steps_x = round(co.x / STEP_SIZE_X);
    int steps_y = round(co.y / STEP_SIZE_Y);


};



void TIM2_IRQHandler() {

};