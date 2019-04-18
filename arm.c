#pragma config(Motor,  motorA,          turnMotor,     tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorB,          jointMotor,    tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorC,          pinchMotor,    tmotorNXT, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//--------------------------------------------------Variables

// Bluetooth Messages
const string BODY_START = "BODY_START";
const string ARM_INIT = "ARM_INIT"
const string ARM_START = "ARM_START";
const string ARM_EXIT = "ARM_EXIT";

// Movement variables
int turnPower = 10;
int turnTime = 1000;
int jointRatio = 1; // Account for difference in distances to robot
int jointPower = 10;
int jointTime = 500;
int pinchPower = 10;
int pinchTime = 500;

// RobotC doesn't hoist functions, solve
// this by declaring functions above.
void toDefaultPosition();
void toSushiPosition();
void pinchChopsticks();
void releaseChopsticks();
void messageBody();
void waitForMessage();
void init();
void pickUpSushi();
void dropSushi();
void exit();
void nextArmCycle();

//--------------------------------------------------Movement
void turnArm(int power, int degrees){
	nMotorEncoder[turnMotor] = 0;
	nMotorEncoderTarget[turnMotor] = degrees;
	motor[turnMotor] = power;

	while(nMotorRunState[turnMotor] != runStateIdle)
 { /* This is an idle loop. The program waits until the condition is satisfied*/}

 motor[turnMotor] = 0;
}

// Moves the arm into place.
void toSushiPosition(){
	// Turn to sushi
	turnArm(turnPower, 90);

	// Joint down to sushi
	motor[jointMotor] = jointPower;
	wait1Msec(jointTime * jointRatio);
	motor[jointMotor] = 0;
};
void toDropPosition(){
	// Joint back up
	motor[jointMotor] = -jointPower;
	wait1Msec(jointTime * jointRatio);
	motor[jointMotor] = 0;

	// Turn back
	turnArm(-turnPower, 180);
}
// Moves arm configuration into default position after dropping.
void toDefaultPosition(){
	// Turn back
	turnArm(turnPower, 90);
};
// Moves chopsticks into pinching position.
void pinchChopsticks(){
	motor[pinchMotor] = pinchPower;
	wait1Msec(pinchTime);
	motor[pinchMotor] = 0;
};
// Releases chopsticks from pinching position.
void releaseChopsticks(){
	motor[pinchMotor] = -pinchPower;
	wait1Msec(pinchTime);
	motor[pinchMotor] = 0;
};

//--------------------------------------------------Bluetooth

// COMMAND    | CODE |  TRANSLATION
// -----------|------|-----------------------
// BODY_START |  +3  |  Run body cycle
// ARM_INIT   |  +2  |  Initialize arm
// ARM_START  |  +1  |  Run arm cycle
// ARM_EXIT   |  -1  |  Exit arm program

// Sends a BODY message to the body controller.
void messageBody(const string command) {
	// RobotC bugs out if switch has one case,
  // just use if check for now.
	if (strcmp(command, BODY_START) == 0) {
		sendMessage(3);
	}
};
// Pauses the program while we wait for a
// message from the body controller.
void waitForMessage(){
	waitUntil(message != 0);
};

// Parses an ARM command coming from the
// body controller
void parseMessage() {
	switch(message){
		case 2:
			ClearMessage();
			toDefaultPosition();
			break;
		case 1:
			jointRatio = messageParm[1];
			ClearMessage();
			nextArmCycle();
			break;
		case -1:
			ClearMessage();
			exit();
			break;
		default:
			break;
	}
}

//--------------------------------------------------Utils

// Initialize system variables
void init();
// Pick up the sushi.
void pickUpSushi() {
	toSushiPosition();
	pinchChopsticks();
};
// Drops the sushi.
void dropSushi() {
	toDropPosition();
	releaseChopsticks();
	toDefaultPosition();
};
// Exit the program
void exit(){
	powerOff();
};

//--------------------------------------------------Main
// Runs a single cycle of the arm program.
//
// 1) Wait to receive message from the body.
// 		- If exit message, exit the program
// 2) If pickup message, pickup the sushi.
// 3) Drop Sushi at desired location
// 4) Move to default position
// 5) Message body
// 6) Repeat loop
void nextArmCycle() {
	pickUpSushi();
	dropSushi();

	messageBody(BODY_START);

	waitForMessage();
	parseMessage();
};

task main()
{
	waitForMessage();
	parseMessage();
}
