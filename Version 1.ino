// Pin definitions
#define ENA 10  // Enable/speed motor Right
#define ENB 11  // Enable/speed motor Left
#define IN1 3   // L298N in1 motor Right
#define IN2 5   // L298N in2 motor Right
#define IN3 6   // L298N in3 motor Left
#define IN4 9   // L298N in4 motor Left

// Constants for motor control
const int MAX_SPEED = 255;
const int MIN_SPEED = 0;
const int SPEED_INCREMENT = 10;
const int TURN_SPEED_REDUCTION = 2;  // Less aggressive than 4
const unsigned long ACCELERATION_DELAY = 20;  // ms between speed changes
const int TURNING_BASE_SPEED = 160;  // Base speed for turns
const unsigned long debounceDelay = 50;  // 50 ms debounce delay

// Global variables
int command = 0;        // Int to store app command state
int speedCar = 75;      // Initial speed at 75
int currentSpeedLeft = 0;
int currentSpeedRight = 0;
int targetSpeedLeft = 0;
int targetSpeedRight = 0;
unsigned long lastSpeedUpdate = 0;
unsigned long lastCommandTime = 0;

void setup() {
    // Initialize motor control pins
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    
    // Start serial communication
    Serial.begin(9600);
    
    // Ensure motors are stopped at startup
    stopRobot();
}

void sendSpeedToApp() {
    Serial.print("SPEED:");
    Serial.println(speedCar);
}

void updateSpeed(int targetSpeed) {
    if (speedCar < targetSpeed) {
        speedCar += SPEED_INCREMENT;
        if (speedCar > targetSpeed) speedCar = targetSpeed;
    } else if (speedCar > targetSpeed) {
        speedCar -= SPEED_INCREMENT;
        if (speedCar < targetSpeed) speedCar = targetSpeed;
    }
    sendSpeedToApp();
}

// Gradually change motor speed to target
void adjustMotorSpeeds() {
    unsigned long currentTime = millis();
    if (currentTime - lastSpeedUpdate >= ACCELERATION_DELAY) {
        bool changed = false;
        
        // Adjust left motor
        if (currentSpeedLeft < targetSpeedLeft) {
            currentSpeedLeft = min(currentSpeedLeft + SPEED_INCREMENT, targetSpeedLeft);
            changed = true;
        } else if (currentSpeedLeft > targetSpeedLeft) {
            currentSpeedLeft = max(currentSpeedLeft - SPEED_INCREMENT, targetSpeedLeft);
            changed = true;
        }
        
        // Adjust right motor
        if (currentSpeedRight < targetSpeedRight) {
            currentSpeedRight = min(currentSpeedRight + SPEED_INCREMENT, targetSpeedRight);
            changed = true;
        } else if (currentSpeedRight > targetSpeedRight) {
            currentSpeedRight = max(currentSpeedRight - SPEED_INCREMENT, targetSpeedRight);
            changed = true;
        }
        
        if (changed) {
            analogWrite(ENA, currentSpeedRight);
            analogWrite(ENB, currentSpeedLeft);
            lastSpeedUpdate = currentTime;
        }
    }
}

void setMotorDirections(bool rightForward, bool leftForward) {
    digitalWrite(IN1, rightForward ? HIGH : LOW);
    digitalWrite(IN2, rightForward ? LOW : HIGH);
    digitalWrite(IN3, leftForward ? HIGH : LOW);
    digitalWrite(IN4, leftForward ? LOW : HIGH);
}

void goAhead() {
    setMotorDirections(true, true);
    targetSpeedRight = speedCar;
    targetSpeedLeft = speedCar;
}

void goBack() {
    setMotorDirections(false, false);
    targetSpeedRight = speedCar;
    targetSpeedLeft = speedCar;
}

void goRight() {
    setMotorDirections(true, false);
    targetSpeedRight = TURNING_BASE_SPEED;
    targetSpeedLeft = TURNING_BASE_SPEED;
}

void goLeft() {
    setMotorDirections(false, true);
    targetSpeedRight = TURNING_BASE_SPEED;
    targetSpeedLeft = TURNING_BASE_SPEED;
}

void goAheadRight() {
    setMotorDirections(true, true);
    targetSpeedRight = speedCar / TURN_SPEED_REDUCTION;
    targetSpeedLeft = speedCar;
}

void goAheadLeft() {
    setMotorDirections(true, true);
    targetSpeedRight = speedCar;
    targetSpeedLeft = speedCar / TURN_SPEED_REDUCTION;
}

void goBackRight() {
    setMotorDirections(false, false);
    targetSpeedRight = speedCar / TURN_SPEED_REDUCTION;
    targetSpeedLeft = speedCar;
}

void goBackLeft() {
    setMotorDirections(false, false);
    targetSpeedRight = speedCar;
    targetSpeedLeft = speedCar / TURN_SPEED_REDUCTION;
}

void stopRobot() {
    setMotorDirections(false, false);
    targetSpeedRight = 0;
    targetSpeedLeft = 0;
    currentSpeedRight = 0;
    currentSpeedLeft = 0;
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
}

void handleMovementCommand(char cmd) {
    switch (cmd) {
        case 'F': goAhead(); break;
        case 'B': goBack(); break;
        case 'L': goLeft(); break;
        case 'R': goRight(); break;
        case 'I': goAheadRight(); break;
        case 'G': goAheadLeft(); break;
        case 'J': goBackRight(); break;
        case 'H': goBackLeft(); break;
        case 'S': stopRobot(); break;
        default: Serial.println("Invalid Movement Command"); break;
    }
}

void handleControlCommand(char cmd) {
    switch (cmd) {
        case '+': 
            updateSpeed(min(speedCar + SPEED_INCREMENT, MAX_SPEED)); 
            break;
        case '-': 
            updateSpeed(max(speedCar - SPEED_INCREMENT, MIN_SPEED)); 
            break;
        default: 
            Serial.println("Invalid Control Command"); 
            break;
    }
}

void loop() {
    // Handle incoming commands
    if (Serial.available() > 0) {
        unsigned long currentTime = millis();
        if (currentTime - lastCommandTime >= debounceDelay) {
            command = Serial.read();
            if (command == '+' || command == '-') {
                handleControlCommand(command);
            } else {
                handleMovementCommand(command);
            }
            lastCommandTime = currentTime;
        }
    }
    
    // Always call this to handle smooth speed transitions
    adjustMotorSpeeds();
}
