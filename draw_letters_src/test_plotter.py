import math
import turtle
from patterns import *

STEP_PATTERN = [
    [1, 0],
    [1, 1],
    [0, 1],
    [0, 0]
]

STEP_FACTOR = 1  # 1 ... full step, 0.5 ... half step, 0.25 ... quarter step, ...

# FULL_STEP_SIZE_U = 10 * math.pi * 7.5 / 360
# FULL_STEP_SIZE_V = 10 * math.pi * 7.5 / 360

FULL_STEP_SIZE_U = 1
FULL_STEP_SIZE_V = 1

STEP_SIZE_U = FULL_STEP_SIZE_U * STEP_FACTOR
STEP_SIZE_V = FULL_STEP_SIZE_V * STEP_FACTOR

SQRT2 = math.sqrt(2)

LETTER_SCALE = 0.1

# ---------------------------------
# Turtle setup
# ---------------------------------

screen = turtle.Screen()
screen.title("Motor Movement Simulation")

pen = turtle.Turtle()
pen.speed(0)

# ---------------------------------
# Data structures
# ---------------------------------

class Coord:
    def __init__(self, x=0.0, y=0.0):
        self.x = x
        self.y = y


class MotorCommand:
    def __init__(self):
        self.steps_u = 0
        self.steps_v = 0
        self.k = 0
        self.direction_u = 1
        self.direction_v = 1


# ---------------------------------
# Global state
# ---------------------------------

currentPos = Coord(0, 0)

MOTOR_COMMAND = MotorCommand()

STEP_INDEX_U = 0
STEP_INDEX_V = 0

CURRENT_U = 0.0
CURRENT_V = 0.0

# ---------------------------------
# Movement functions
# ---------------------------------

def moveAbs(co):
    global currentPos

    Dx = co.x - currentPos.x
    Dy = co.y - currentPos.y

    motorMove = Coord()
    motorMove.x = Dx
    motorMove.y = Dy
    # motorMove.x = (Dx - Dy) * SQRT2 / 2
    # motorMove.y = (Dx + Dy) * SQRT2 / 2

    currentPos = co

    moveMotor(motorMove)


def moveMotor(co):
    cmd = MotorCommand()

    cmd.steps_u = abs(round(co.x / STEP_SIZE_U))
    cmd.steps_v = abs(round(co.y / STEP_SIZE_V))

    if cmd.steps_u == 0 and cmd.steps_v == 0:
        cmd.k = 0
    else:
        cmd.k = cmd.steps_v / cmd.steps_u if cmd.steps_u > cmd.steps_v else cmd.steps_u / cmd.steps_v

    cmd.direction_u = 1 if co.x > 0 else -1
    cmd.direction_v = 1 if co.y > 0 else -1

    actMotor(cmd)


def actMotor(cmd):
    global MOTOR_COMMAND
    global stepCounter
    global stepRemainder

    MOTOR_COMMAND = cmd
    
    stepCounter = 1
    stepRemainder = 0

# ---------------------------------
# Pen up / down
# ---------------------------------

def penUp():
    pen.penup()

def penDown():
    pen.pendown()

# ---------------------------------
# Step functions
# ---------------------------------

def sendStepU(direction):
    global STEP_INDEX_U
    global CURRENT_U
    global CURRENT_V

    STEP_INDEX_U = (STEP_INDEX_U + direction) % len(STEP_PATTERN)

    # Convert U motor step back into XY movement
    du = direction * STEP_SIZE_U

    CURRENT_U += du

    pen.goto(CURRENT_U, CURRENT_V)


def sendStepV(direction):
    global STEP_INDEX_V
    global CURRENT_U
    global CURRENT_V

    STEP_INDEX_V = (STEP_INDEX_V + direction) % len(STEP_PATTERN)

    # Convert V motor step back into XY movement
    dv = direction * STEP_SIZE_V

    CURRENT_V += dv

    pen.goto(CURRENT_U, CURRENT_V)


# ---------------------------------
# Simulated interrupt handler
# ---------------------------------

stepCounter = 1
stepRemainder = 0

def TIM2_IRQHandler():
    global stepCounter
    global stepRemainder

    while MOTOR_COMMAND.steps_u > 0 or MOTOR_COMMAND.steps_v > 0:
        other_step = stepCounter * MOTOR_COMMAND.k + stepRemainder

        if MOTOR_COMMAND.steps_u > MOTOR_COMMAND.steps_v:
            sendStepU(MOTOR_COMMAND.direction_u)
            MOTOR_COMMAND.steps_u -= 1

            if other_step >= 1:
                sendStepV(MOTOR_COMMAND.direction_v)
                MOTOR_COMMAND.steps_v -= 1
                stepRemainder = other_step - 1
                stepCounter = 1
            else:
                stepCounter += 1

        else:
            sendStepV(MOTOR_COMMAND.direction_v)
            MOTOR_COMMAND.steps_v -= 1

            if other_step >= 1:
                sendStepU(MOTOR_COMMAND.direction_u)
                MOTOR_COMMAND.steps_u -= 1
                stepRemainder = other_step - 1
                stepCounter = 1
            else:
                stepCounter += 1

# -----------------------------
# Utility movements
# -----------------------------

def travel(co):
    penUp()
    moveAbs(co)
    TIM2_IRQHandler()


def draw(co):
    penDown()
    moveAbs(co)
    TIM2_IRQHandler()

# -------------------------------------
# Draw Letters
# -------------------------------------

def drawLetter(letter, pos):
    if ord(letter) >= 97:
        letterIndex = ord(letter) - 97
        coordinates = lowercase_letters_coordinates
        endPts = lowercase_letters_endPts
    else:
        letterIndex = ord(letter) - 65
        coordinates = uppercase_letters_coordinates
        endPts = uppercase_letters_endPts

    endpt_i = 0
    endpt = endPts[letterIndex][endpt_i]

    for i in range(400):
        if len(coordinates[letterIndex]) <= i:
            break
        # Stop if coordinate is (0,0)
        if coordinates[letterIndex][i][0] == 0 and coordinates[letterIndex][i][1] == 0:
            break

        co = Coord(
            coordinates[letterIndex][i][0] * LETTER_SCALE + pos.x,
            coordinates[letterIndex][i][1] * LETTER_SCALE + pos.y
        )
        
        if i == 0 or i-1 == endpt:
            travel(co)
        else:
            draw(co)
        
        if i-2 == endpt:
            if endpt_i < len(endPts[letterIndex]):
                endpt_i += 1
                endpt = endPts[letterIndex][endpt_i]


def letterRect(letter):
    if ord(letter) >= 97:
        letterIndex = ord(letter) - 97
        coordinates = lowercase_letters_coordinates
        endPts = lowercase_letters_endPts
    else:
        letterIndex = ord(letter) - 65
        coordinates = uppercase_letters_coordinates
        endPts = uppercase_letters_endPts
    
    max_x = coordinates[letterIndex][0][0]
    max_y = coordinates[letterIndex][0][1]
    min_x = coordinates[letterIndex][0][0]
    min_y = coordinates[letterIndex][0][1]

    for co in coordinates[letterIndex]:
        if co[0] < min_x:
            min_x = co[0]
        if co[1] < min_y:
            min_y = co[1]
        if co[0] > max_x:
            max_x = co[0]
        if co[1] > max_y:
            max_y = co[1]
    
    max_x *= LETTER_SCALE
    max_y *= LETTER_SCALE
    min_x *= LETTER_SCALE
    min_y *= LETTER_SCALE

    x, y = min_x, min_y
    w = max_x - min_x
    h = max_y - min_y

    print(min_x, min_y, max_x, max_y)
    return x, y, w, h


def drawText(chars, pos):
    start = pos
    for char in chars:
        if char == "\n":
            start = Coord(pos.x, pos.y - 100)
        elif char == " ":
            start = Coord(start.x + 20, start.y)
        else:
            drawLetter(char, start)
            x, y, w, h = letterRect(char)
            start = Coord(start.x + w, start.y)


# ---------------------------------
# Example usage
# ---------------------------------

travel(Coord(0, 0))
drawText("Hello World\nThe quick brown fox jumps\nover the lazy dog", Coord(-350, 0))

turtle.done()