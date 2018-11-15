
#include <Arduino.h>
#include "StructuredSerialOutput.h"
#include "TeensyGoNogo.h"



static bool verboseOutput = false;

void SSO_setVerbose(bool isVerbose) {
    verboseOutput = isVerbose;
}

void SSO_version(float versionNum) {
    if (verboseOutput) {
        Serial.println("GoNogo Behavior Controller.");
        Serial.print("(Version ");
        Serial.print(versionNum);
        Serial.println(")");
    } else {
        Serial.println("NAME GoNogo Behavior Controller");
        Serial.print("VER ");
        Serial.println(versionNum);
    }
}

void SSO_handshake() {
    if (verboseOutput) {
        Serial.println("$"); // initial handshake to verify connection
    } else {
        Serial.println("$"); // initial handshake to verify connection
    }
}

void SSO_newState(GoNogoState state) {
    if (verboseOutput) {
        Serial.print("State: ");
    } else {
        Serial.print("S_");
    }
    Serial.println(getStateString(state));

    // if (!verboseOutput) {
    //     if (state==IDLE) {Serial.println("State IDLE");}
    //     else if (state==START_TRIAL) {Serial.println("State START_TRIAL");}
    //     else if (state==WAIT_FOR_NOSE_OUT) {Serial.println("State WAIT_FOR_NOSE_OUT");}
    //     else if (state==WAIT_FOR_POKE) {Serial.println("State WAIT_FOR_POKE");}
    //     else if (state==STIM_DELIVERY) {Serial.println("State STIM_DELIVERY");}
    //     else if (state==REWARD_WINDOW) {Serial.println("State REWARD_WINDOW");}
    //     else if (state==NO_REWARD_WINDOW) {Serial.println("State NO_REWARD_WINDOW");}
    //     else if (state==REWARD_DELIVERY) {Serial.println("State REWARD_DELIVERY");}
    //     else if (state==END_RUN) {Serial.println("State END_RUN");}
    //     else if (state==END_TRIAL) {Serial.println("State END_TRIAL");}
    //     else if (state==MANUAL_REWARD_DELIVERY_ON) {Serial.println("State MANUAL_REWARD_DELIVERY_ON");}
    //     else if (state==MANUAL_REWARD_DELIVERY_OFF) {Serial.println("State MANUAL_REWARD_DELIVERY_OFF");}
    // } else {
    //     if (state==IDLE) {Serial.println("State IDLE");}
    //     else if (state==START_TRIAL) {Serial.println("State START_TRIAL");}
    //     else if (state==WAIT_FOR_NOSE_OUT) {Serial.println("State WAIT_FOR_NOSE_OUT");}
    //     else if (state==WAIT_FOR_POKE) {Serial.println("State WAIT_FOR_POKE");}
    //     else if (state==STIM_DELIVERY) {Serial.println("State STIM_DELIVERY");}
    //     else if (state==REWARD_WINDOW) {Serial.println("State REWARD_WINDOW");}
    //     else if (state==NO_REWARD_WINDOW) {Serial.println("State NO_REWARD_WINDOW");}
    //     else if (state==REWARD_DELIVERY) {Serial.println("State REWARD_DELIVERY");}
    //     else if (state==END_RUN) {Serial.println("State END_RUN");}
    //     else if (state==END_TRIAL) {Serial.println("State END_TRIAL");}
    //     else if (state==MANUAL_REWARD_DELIVERY_ON) {Serial.println("State MANUAL_REWARD_DELIVERY_ON");}
    //     else if (state==MANUAL_REWARD_DELIVERY_OFF) {Serial.println("State MANUAL_REWARD_DELIVERY_OFF");}
    // }
}

void SSO_paramVal(const char* paramName, float val) {
    if (!verboseOutput) {
        Serial.print("P_");
    }
    Serial.print(paramName);
    Serial.print(" ");
    Serial.println(val);
}

void SSO_paramVal(const char* paramName, float val1, float val2) {
    if (!verboseOutput) {
        Serial.print("P_");
    }
    Serial.print(paramName);
    Serial.print(" ");
    Serial.print(val1);
    Serial.print(" ");
    Serial.println(val2);
}


void SSO_lick_outcome(bool prevLickWasLow){
    if (verboseOutput) {
        if (prevLickWasLow) {
            Serial.println("Lick On");
        } else {
            Serial.println("Lick Off");
        }
    } else {
        if (prevLickWasLow) {
            Serial.println("LI");
        } else {
            Serial.println("LO");
        }
    }
}


void SSO_nose(bool isNoseIn){
    if (verboseOutput) {
        if (isNoseIn) {
            Serial.println("Nose In");
        } else {
            Serial.println("Nose Out");
        }
    } else {
        if (isNoseIn) {
            Serial.println("NI");
        } else {
            Serial.println("NO");
        }
    }
}

void SSO_trialOutcome(int outcome) {
    if (!verboseOutput) {
        if (outcome == SSO_OUTCOME_HIT) {
            Serial.println("OH"); // [O]utcome [H]it
        } else if (outcome == SSO_OUTCOME_MISS) {
            Serial.println("OM"); // [O]utcome [M]iss
        } else if (outcome == SSO_OUTCOME_CORRECT_REJECTION) {
            Serial.println("OCR"); // [O]utcome [C]orrect [R]ejection
        } else if (outcome == SSO_OUTCOME_FALSE_ALARM) {
            Serial.println("OFA"); // [O]utcome [F]alse [A]larm
        } else {
            // ERROR
            Serial.println("# Bad outcome type.");
        }
    } else {
        if (outcome == SSO_OUTCOME_HIT) {
            Serial.println("> HIT - Reward Delivered");
        } else if (outcome == SSO_OUTCOME_MISS) {
            Serial.println("> MISS - No Reward");
        } else if (outcome == SSO_OUTCOME_CORRECT_REJECTION) {
            Serial.println("> Correct rejection - No Reward");
        } else if (outcome == SSO_OUTCOME_FALSE_ALARM) {
            Serial.println("> False alarm - (Null penalty?)");
        } else {
            // ERROR
            Serial.println("# Bad outcome type.");
        }
    }
}

void SSO_reward(bool isStart) {
    if (!verboseOutput) {
        if (isStart) {
            Serial.println("RS"); // [R]eward [S]tart
        } else {
            Serial.println("RE"); // [R]eward [E]nd
        }
    } else {
        if (isStart) {
            Serial.println("Delivering Reward");
        }
    }
}


void SSO_trial(int trialNum) {
    if (!verboseOutput) {
        Serial.print("T ");
    } else {
        Serial.print("Trial ");
    }
    Serial.println(trialNum);
}

void SSO_stimulus(int stimType) {
    if (!verboseOutput) {
        Serial.print("SS "); // Stimulus Start
    } else {
        Serial.print("Stimulus On: ");
    }
    Serial.println(stimType);
}

void SSO_stimulusEnd() {
    if (!verboseOutput) {
        Serial.println("SE"); // Stimulus End
    } else {
        Serial.println("Stimulus Off");
    }
}

void SSO_endSession() {
    if (!verboseOutput) {
        Serial.println("E");
    } else {
        // Serial.println("");
    }
}

void SSO_abort() {
    if (!verboseOutput) {
        Serial.println("A");
    } else {
        // Serial.println("");
    }
}



void SSO_comment(const char* message) {
    Serial.print("! ");
    Serial.println(message);
}

void SSO_error(const char* errorMessage) {
    Serial.print("# ");
    Serial.println(errorMessage);
}

void SSO_error(int errorNum) {
    if (!verboseOutput) {
        Serial.print("# ");
        Serial.println(errorNum);
    } else {
        Serial.print("# ERROR ");
        switch (errorNum) {
        case SSO_ERROR_TOO_FEW_ARGS:
            Serial.println("Too few arguments.");
            break;

        case SSO_ERROR_BAD_VALVE_NUM:
            Serial.println("Bad valve number.");
            break;

        case 3:
            Serial.println("Bad valve number.");
            break;

        case 4:
            break;

        case 5:
            break;


        default:
            Serial.println("Unkonwn Error.");
        }
    }
}


    // Serial.println('I');

    // Serial.println(". Lick");
    // Serial.print("Lick Bins: [ ");
    // Serial.print(_lickBins[bin]);
    // Serial.print(" ");
    // Serial.println("]");

    // Serial.println(msg);

    // Serial.print("Error: Bad valve number ("); Serial.print(valveNum); Serial.println(").");
    // Serial.print("Valve "); Serial.print(valveNum); Serial.println("already open.");
    // Serial.print("Valve ");
    // Serial.print(valveNum);
    // Serial.println(" OPEN.");
    // Serial.print("Error: Bad valve number ("); Serial.print(valveNum); Serial.println(").");
    // Serial.print("Valve ");
    // Serial.print(valveNum);
    // Serial.println(" CLOSED.");
    // Serial.print("Valve ");
    // Serial.print(valve+1);
    // Serial.println(" PWM (low power).");
