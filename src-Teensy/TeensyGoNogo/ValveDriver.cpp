
// =======================================
// Valve control functions

#include "ValveDriver.h"

// for debugging
#define DISPLAY_VALVE_STATUS false

// For valve PWM
static uint8_t valve_status_list[NUM_VALVES];
#define VALVE_CLOSED (0)
#define VALVE_NO_PWM (1)
#define VALVE_PWM    (2)
#define VALVE_OPEN_1 (3)
#define VALVE_OPEN_2 (4)


// FOR DEBUGGING
// #define digitalWrite(pin, val) digitalWrite((pin), (val)); Serial.print("DWrite "); Serial.print((pin)); Serial.print(" "); Serial.println((val))
// #define analogWrite(pin, val) analogWrite((pin), (val)); Serial.print("AWrite "); Serial.print((pin)); Serial.print(" "); Serial.println((val))

void setupValves() {
	// Set up pins for valve control
	for (int i=0; i<NUM_VALVES; i++) {
		pinMode(VALVES[i], OUTPUT);
		digitalWrite(VALVES[i], LOW);
		valve_status_list[i] = VALVE_CLOSED;
	}
	// set PWM frequency
	// (many pins share the same clock, so we only
	//  need to set for pins 3 & 5 for the 8 pins
	//	we're using on a Teensy3.2.)
	// (Ref: https://www.pjrc.com/teensy/td_pulse.html)
	analogWriteFrequency(3, 234375);
	analogWriteFrequency(5, 234375);
}

void OpenValvePin(int pin) {
	for (int valveNum = 0; valveNum < NUM_VALVES; valveNum++) {
		if (VALVES[valveNum] == pin) {
			OpenValveNum(valveNum+1);
			break;
		}
	}
}

void OpenValveNum(int valveNum) { // valveNum is 1-based
	// error checking
	if ((valveNum > NUM_VALVES) || (valveNum == 0)) {
		Serial.print("Error: Bad valve number ("); Serial.print(valveNum); Serial.println(").");
		return;
	}
	if (valve_status_list[valveNum-1] > VALVE_CLOSED) {
		Serial.print("Valve "); Serial.print(valveNum); Serial.println("already open.");
		return;
	}

	// open the valve
	digitalWrite(VALVES[valveNum-1], HIGH);
	valve_status_list[valveNum-1] = VALVE_OPEN_2;
	if (DISPLAY_VALVE_STATUS) {
		Serial.print("Valve ");
		Serial.print(valveNum);
		Serial.println(" OPEN.");
	}
}

void CloseValvePin(int pin) {
	for (int valveNum = 0; valveNum < NUM_VALVES; valveNum++) {
		if (VALVES[valveNum] == pin) {
			CloseValveNum(valveNum+1);
			break;
		}
	}
}

void CloseValveNum(int valveNum) {
	// error checking
	if ((valveNum > NUM_VALVES) || (valveNum == 0)) {
		Serial.print("Error: Bad valve number ("); Serial.print(valveNum); Serial.println(").");
		return;
	}
	if (valve_status_list[valveNum-1] == VALVE_CLOSED) {
		// Serial.print("Cannot close valve "); Serial.print(valveNum); Serial.println(", it's not open.");
		return;
	}

	// close valve
	digitalWrite(VALVES[valveNum-1], LOW);
	if (valve_status_list[valveNum-1] == VALVE_PWM) {
		analogWrite(VALVES[valveNum-1], 0);
	}
	valve_status_list[valveNum-1] = VALVE_CLOSED;

	if (DISPLAY_VALVE_STATUS) {
		Serial.print("Valve ");
		Serial.print(valveNum);
		Serial.println(" CLOSED.");
	}
}

static elapsedMillis previousUpdateInterval = 0;
void updateValves() {
	if (previousUpdateInterval > 500) {
		previousUpdateInterval = 0;
		for (int valve = 0; valve < NUM_VALVES; valve++) {
			// test for VALVE_CLOSED first, since it's most likely
			if (valve_status_list[valve] > VALVE_CLOSED) {
				if (valve_status_list[valve] == VALVE_OPEN_2) {
					valve_status_list[valve] = VALVE_OPEN_1;
				} else if (valve_status_list[valve] == VALVE_OPEN_1) {
					if (digitalPinHasPWM(VALVES[valve])) {
						valve_status_list[valve] = VALVE_PWM;
						analogWrite(VALVES[valve], 127);
						if (DISPLAY_VALVE_STATUS) {
							Serial.print("Valve ");
							Serial.print(valve+1);
							Serial.println(" PWM (low power).");
						}
					}
				}
			}
		}
	}
}
