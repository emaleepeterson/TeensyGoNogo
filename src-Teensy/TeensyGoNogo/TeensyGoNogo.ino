#include "TeensyGoNogo.h"
#include "SerialParser.h"
#include "ValveDriver.h"
#include "StructuredSerialOutput.h"


#include <string.h>

const bool isVerbose = false;
const float VERSION = 0.3;

void interpretGNGCommand(char* command, int numArgs, long* args);

TeensyGoNogo gng;
SerialParser sp = SerialParser(interpretGNGCommand);

void setup() {
    // set up Serial
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }

    // set up valves
    setupValves();

    // set up TeensyGoNogo instance
    gng.set_pStim(0.5);
    gng.set_numTrials(20);
    gng.set_numLickBins(5);
    gng.set_minLickBins(4);
    gng.set_stimulusDuration_ms(2000);
    gng.set_rewardDuration_ms(100);
    gng.set_interRewardDuration_ms(500);
    gng.set_interTrialDuration_ms(1000);
    gng.set_numRewardedOdors(1);

    // set up default pins for HMS Valve Driver Board
    gng.set_beambreakPin(IO_1);
    gng.set_lickPin(LICK_1);
    gng.set_LEDPin(IO_2);
    gng.set_rewardPin(VALVE_1);
    gng.set_blankOdorPin(VALVE_2);
    gng.set_unrewardedOdorPin(VALVE_3);
    gng.set_rewardedOdorPin(1, VALVE_4);
    gng.set_rewardedOdorPin(2, VALVE_5);
    gng.set_rewardedOdorPin(3, VALVE_6);
    gng.set_rewardedOdorPin(4, VALVE_7);
    gng.set_rewardedOdorPin(5, VALVE_8);

    // For debugging output:
    gng._debugLevel = 1;


    delay(500);
    // Serial.println("GoNogo Behavior Controller.");
    // Serial.print("(Version ");
    // Serial.print(VERSION);
    // Serial.println(")");
    SSO_setVerbose(isVerbose);
    SSO_version(VERSION);

}


void interpretGNGCommand(char* command, int numArgs, long* args) {
    // Serial.print("Command: ");
    // Serial.print(command);
    // // Serial.print(" (");
    // // Serial.print(numArgs);
    // // Serial.println(" args)");
    // Serial.print(" [ ");
    // for (int i=0; i<numArgs; i++) {
    //     Serial.print(args[i]);
    //     Serial.print(" ");
    // }
    // Serial.println(" ] ");

    String msg;

    if ((strcmp(command,"$") == 0)) {
        SSO_handshake();
        // Serial.println("$"); // initial handshake to verify connection

    } else if ((strcmp(command,"run") == 0)) {
        gng.run();
        SSO_comment("RUNNING NEW SESSION...");

    } else if ((strcmp(command,"stop") == 0)) {
        gng.stop();
        SSO_comment("ABORTING SESSION...");

    } else if ((strcmp(command,"reward") == 0)) {
        gng.manualReward();
        SSO_comment("Delivereing single reward");

    } else if ((strcmp(command,"multiReward") == 0)) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.multiManualReward(args[0]);
        SSO_comment("Delivering multiple rewards");

    } else if (strcmp(command,"numRewardedOdors") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_numRewardedOdors(args[0]);
        SSO_paramVal("numRewardedOdors", gng.get_numRewardedOdors());

    } else if (strcmp(command,"rewardedOdorPin") == 0) {
        if (numArgs < 2) {Error_TooFewArgs(); return;}
        gng.set_rewardedOdorPin(args[0], args[1]);
        SSO_paramVal("rewardedOdorPin", args[0], gng.get_rewardedOdorPin(args[0]));

    } else if (strcmp(command,"beambreakPin") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_beambreakPin(args[0]);
        SSO_paramVal("beambreakPin ", gng.get_beambreakPin());

    } else if (strcmp(command,"lickPin") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_lickPin(args[0]);
        SSO_paramVal("lickPin ", gng.get_lickPin());

    } else if (strcmp(command,"LEDPin") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_LEDPin(args[0]);
        SSO_paramVal("LEDPin ", gng.get_LEDPin());

    } else if (strcmp(command,"rewardPin") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_rewardPin(args[0]);
        SSO_paramVal("rewardPin ", gng.get_rewardPin());

    } else if (strcmp(command,"blankOdorPin") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_blankOdorPin(args[0]);
        SSO_paramVal("blankOdorPin ", gng.get_blankOdorPin());

    } else if (strcmp(command,"unrewardedOdorPin") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_unrewardedOdorPin(args[0]);
        SSO_paramVal("unrewardedOdorPin ", gng.get_unrewardedOdorPin());

    } else if (strcmp(command,"pStim") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_pStim(float(args[0])/1000.0);
        SSO_paramVal("pStim ", gng.get_pStim());

    } else if (strcmp(command,"numTrials") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_numTrials(args[0]);
        SSO_paramVal("numTrials ", gng.get_numTrials());

    } else if (strcmp(command,"maxRunLength") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_maxRunLength(args[0]);
        SSO_paramVal("maxRunLength ", gng.get_maxRunLength());

    } else if (strcmp(command,"minLickBins") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_minLickBins(args[0]);
        SSO_paramVal("minLickBins ", gng.get_minLickBins());

    } else if (strcmp(command,"numLickBins") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_numLickBins(args[0]);
        SSO_paramVal("numLickBins ", gng.get_numLickBins());

    } else if (strcmp(command,"stimulusDuration_ms") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_stimulusDuration_ms(args[0]);
        SSO_paramVal("stimulusDuration_ms ", gng.get_stimulusDuration_ms());

    } else if (strcmp(command,"rewardDuration_ms") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_rewardDuration_ms(args[0]);
        SSO_paramVal("rewardDuration_ms ", gng.get_rewardDuration_ms());

    } else if (strcmp(command,"interTrialDuration_ms") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_interTrialDuration_ms(args[0]);
        SSO_paramVal("interTrialDuration_ms ", gng.get_interTrialDuration_ms());

    } else if (strcmp(command,"interRewardDuration_ms") == 0) {
        if (numArgs < 1) {Error_TooFewArgs(); return;}
        gng.set_interRewardDuration_ms(args[0]);
        SSO_paramVal("interRewardDuration_ms ", gng.get_interRewardDuration_ms());
    }
}

void Error_TooFewArgs() {
    SSO_error(SSO_ERROR_TOO_FEW_ARGS);
    // Serial.println("Error: Too few arguments.");
}


elapsedMicros accumulatedLoopTime = 0;
unsigned long maxLoopTime = 0;
int loopCounter = 0;

void loop() {
	loopCounter++;
	elapsedMicros singleLoopTime = 0;

    gng.update();
    sp.update();
    updateValves();

	if (singleLoopTime > maxLoopTime) {
		maxLoopTime = singleLoopTime;
	}
	if (loopCounter == 1e7L) {
        String msg = String("					## Mean, Max loop time: ");
		msg += accumulatedLoopTime / loopCounter;
		msg +=  " us,    ";
		msg += maxLoopTime;
		msg += " us ##";
        SSO_comment(msg.c_str());
		accumulatedLoopTime = 0;
		loopCounter = 0;
		maxLoopTime = 0;
	}
}

