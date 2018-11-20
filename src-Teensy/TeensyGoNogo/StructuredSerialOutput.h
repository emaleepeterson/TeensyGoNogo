
#ifndef _STRUCTURED_SERIAL_OUTPUT_
#define _STRUCTURED_SERIAL_OUTPUT_

#include "TeensyGoNogo.h"

#define SSO_OUTCOME_HIT 				1
#define SSO_OUTCOME_MISS 				2
#define SSO_OUTCOME_CORRECT_REJECTION 	3
#define SSO_OUTCOME_FALSE_ALARM 		4

#define SSO_ERROR_TOO_FEW_ARGS 1
#define SSO_ERROR_BAD_VALVE_NUM 2


void SSO_setVerbose(bool isVerbose);
void SSO_handshake();
void SSO_version(float versionNum);
void SSO_newState(GoNogoState state);
void SSO_paramVal(const char* paramName, float val);
void SSO_paramVal(const char* paramName, float val1, float val2);

void SSO_lick();
void SSO_nose(bool isNoseIn);
void SSO_trialOutcome(int outcome);
void SSO_reward(bool isStart);

void SSO_trial(int trialNum);
void SSO_stimulus(int stimType);
void SSO_stimulusEnd();
void SSO_endSession();
void SSO_abort();

void SSO_comment(const char* message);
void SSO_error(const char* errorMessage);
void SSO_error(int errorNum);

#endif // _STRUCTURED_SERIAL_OUTPUT_
