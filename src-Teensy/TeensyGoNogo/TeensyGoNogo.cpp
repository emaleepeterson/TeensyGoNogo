
#include "TeensyGoNogo.h"
#include "ValveDriver.h"
#include "StructuredSerialOutput.h"

TeensyGoNogo::TeensyGoNogo() {
    // set default values
    _pStim = 0.5;
    _numTrials = 10;
    _numLickBins = 4;
    _minLickBins = 3;
    _numRewardedOdors = 5;
    _stimulusDuration_us = 2000 * 1000;
    _rewardDuration_us = 100 * 1000;
    _interTrialDuration_us = 2000 * 1000;
    _interRewardDuration_us = 500 * 1000;
    _nullPenalty_us = 5000 * 1000;
    _pauseLickingDelay_us = 1000 * 1000;
    _lickPenaltyThresh = 1;

    // initialize to idle state
    _isRunning = false;
    _currentState = IDLE;

    // seed RNG
    randomSeed(analogRead(0));
}

TeensyGoNogo::~TeensyGoNogo() {
}


void TeensyGoNogo::set_numRewardedOdors(uint8_t numRewardedOdors) {
    if ((numRewardedOdors < 1) || (numRewardedOdors > MAX_ODOR_PINS)) {
        // ERROR
        return;
    }
    if (!_isRunning) {
        _numRewardedOdors = numRewardedOdors;
    }
}

void TeensyGoNogo::set_rewardedOdorPin(uint8_t rewardedOdorNum, uint8_t pin) {
    if ((rewardedOdorNum < 1) || (rewardedOdorNum > _numRewardedOdors)) {
        // ERROR
        return;
    }
    if (!_isRunning) {
        _rewardedOdorPins[rewardedOdorNum-1] = pin;
//        Serial.print("A rewarded odor pin has been set to "); Serial.println(_rewardedOdorPins[rewardedOdorNum-1]);
    }
}

int TeensyGoNogo::get_rewardedOdorPin(uint8_t rewardedOdorNum) {
    if ((rewardedOdorNum < 1) || (rewardedOdorNum > _numRewardedOdors)) {
        // ERROR
        return -1;
    }
    return _rewardedOdorPins[rewardedOdorNum-1];
}

void TeensyGoNogo::set_optoPin(uint8_t optoPin) {
    if (!_isRunning) {
        _optoPin = optoPin;
    }
}

void TeensyGoNogo::set_beambreakPin(uint8_t beambreakPin) {
    if (!_isRunning) {
        _beambreakPin = beambreakPin;
    }
}

void TeensyGoNogo::set_lickPin(uint8_t lickPin) {
    if (!_isRunning) {
        _lickPin = lickPin;
    }
}

void TeensyGoNogo::set_LEDPin(uint8_t LEDPin) {
    if (!_isRunning) {
        _LEDPin = LEDPin;
    }
}

void TeensyGoNogo::set_rewardPin(uint8_t rewardPin) {
    if (!_isRunning) {
        _rewardPin = rewardPin;
    }
}

void TeensyGoNogo::set_blankOdorPin(uint8_t blankOdorPin) {
    if (!_isRunning) {
        _blankOdorPin = blankOdorPin;
    }
}

void TeensyGoNogo::set_unrewardedOdorPin(uint8_t unrewardedOdorPin) {
    if (!_isRunning) {
        _unrewardedOdorPin = unrewardedOdorPin;
    }
}

void TeensyGoNogo::set_pStim(float pStim) {
    if (pStim < 0 || pStim > 1) {
        // ERROR
        return;
    }
    if (!_isRunning) {
        _pStim = pStim;
    }
}

void TeensyGoNogo::set_numTrials(unsigned int numTrials) {
    if (!_isRunning) {
        _numTrials = numTrials;
    }
}

void TeensyGoNogo::set_maxRunLength(unsigned int maxRunLength) {
    if (!_isRunning) {
        _maxRunLength = maxRunLength;
    }
}

void TeensyGoNogo::set_minLickBins(uint8_t minLickBins) {
    if (!_isRunning) {
        _minLickBins = minLickBins;
    }
}

void TeensyGoNogo::set_numLickBins(uint8_t numLickBins) {
    if (!_isRunning) {
        _numLickBins = numLickBins;
    }
}

void TeensyGoNogo::set_stimulusDuration_ms(unsigned long stimulusDuration_ms) {
    if (!_isRunning) {
        _stimulusDuration_us = stimulusDuration_ms * 1000;
    }
}

void TeensyGoNogo::set_rewardDuration_ms(unsigned long rewardDuration_ms) {
    if (!_isRunning) {
        _rewardDuration_us = rewardDuration_ms * 1000;
    }
}

void TeensyGoNogo::set_interTrialDuration_ms(unsigned long interTrialDuration_ms) {
    if (!_isRunning) {
        _interTrialDuration_us = interTrialDuration_ms * 1000;
    }
}

void TeensyGoNogo::set_interRewardDuration_ms(unsigned long interRewardDuration_ms) {
    if (!_isRunning) {
        _interRewardDuration_us = interRewardDuration_ms * 1000;
    }
}

void TeensyGoNogo::set_nullPenalty_ms(unsigned long nullPenalty_ms) {
    if (!_isRunning) {
        _nullPenalty_us = nullPenalty_ms * 1000;
    }
}

void TeensyGoNogo::set_pauseLickingDelay_ms(unsigned long pauseLickingDelay_ms) {
    if (!_isRunning) {
        _pauseLickingDelay_us = pauseLickingDelay_ms * 1000;
    }
}

void TeensyGoNogo::set_lickPenaltyThresh(unsigned int lickPenaltyThresh) {
    if (!_isRunning) {
        _lickPenaltyThresh = lickPenaltyThresh;
    }
}

void TeensyGoNogo::nosePokeInterrupt() {
    _noseChange = true;
}

void TeensyGoNogo::lickInterrupt() {
    _lickCount++;
}


void TeensyGoNogo::manualReward() {
    if (_currentState == IDLE) {
        _numRewardsRemaining = 1;
        _currentState = MANUAL_REWARD_DELIVERY_ON;
    }
}

void TeensyGoNogo::multiManualReward(unsigned int numRewards) {
    if (_currentState == IDLE) {
        _numRewardsRemaining = numRewards;
        _currentState = MANUAL_REWARD_DELIVERY_ON;
    }
}

void TeensyGoNogo::run() {
    // initialize hardware
    pinMode(_beambreakPin, INPUT);
    _noseChange = false;
    // attachInterrupt(digitalPinToInterrupt(_beambreakPin), nosePokeInterrupt, CHANGE);
    pinMode(_lickPin, INPUT);
    _lickCount = 0;
    // attachInterrupt(digitalPinToInterrupt(_lickPin), lickInterrupt, RISING);
    pinMode(_LEDPin, OUTPUT);
    digitalWrite(_LEDPin, LOW);
    _led = 13;
    pinMode(_led, OUTPUT);
    digitalWrite(_led, LOW);
    setupValves();
    OpenValvePin(_blankOdorPin);
    // initialize trial information
    if (_maxRunLength == 0) {
        for (unsigned int trial=0; trial < _numTrials; trial++) {
            if (random(999) < _pStim * 1000) {
                _trialTypeList[trial] = random(1,_numRewardedOdors+1); // defaults to all 1s if _numRewardedOdors==1
            } else {
                _trialTypeList[trial] = 0;
            }
        }
    } else {
        // set up trial 1
        unsigned int currRunLength = 1;
        bool prevTrialWasType0;
        if (random(999) < _pStim * 1000) {
            _trialTypeList[0] = random(1,_numRewardedOdors+1); // defaults to all 1s if _numRewardedOdors==1
            prevTrialWasType0 = false;
        } else {
            _trialTypeList[0] = 0;
            prevTrialWasType0 = true;
        }
        for (unsigned int trial=1; trial < _numTrials; trial++) {
            if (currRunLength == _maxRunLength) {
                // if we have reached the max run length, then
                // force the current trial to be the opposite type
                // from prev trial
                if (prevTrialWasType0) {
                    _trialTypeList[trial] = random(1,_numRewardedOdors+1); // defaults to 1 if _numRewardedOdors==1
                } else {
                    _trialTypeList[trial] = 0;
                }
                currRunLength = 1;
                prevTrialWasType0 = !prevTrialWasType0;
            } else if (random(999) < _pStim * 1000) {
                _trialTypeList[trial] = random(1,_numRewardedOdors+1); // defaults to 1 if _numRewardedOdors==1
                if (!prevTrialWasType0) {
                    currRunLength++;
                } else {
                    currRunLength = 1;
                }
                prevTrialWasType0 = false;
            } else {
                _trialTypeList[trial] = 0;
                if (prevTrialWasType0) {
                    currRunLength++;
                } else {
                    currRunLength = 1;
                }
                prevTrialWasType0 = true;
            }
        }
    }

    // DEBUG
    String trialsList;
    trialsList = "Trials: [ ";
    for (unsigned int trial=0; trial < _numTrials; trial++) {
        trialsList += _trialTypeList[trial];
        trialsList += " ";
    }
    trialsList += "]";
    debugOut(trialsList.c_str(), 1);

    // Bin size calculation
    _binSize_us = _stimulusDuration_us / _numLickBins;

    // initialize state variables
    _isRunning = true;
    _currentState = START_TRIAL;
    _trialNum = 0;
    // _rewardCount = 0;
    // _nullSuccess = 0;
}

void TeensyGoNogo::stop() {
    SSO_abort();
    _isRunning = false;
    _currentState = IDLE;
    // detachInterrupt(digitalPinToInterrupt(_beambreakPin));
    // detachInterrupt(digitalPinToInterrupt(_lickPin));
    _trialNum = 0;
    _lickCount = 0;
    // _rewardCount = 0;
    _pause_lickCount = 0;
    _noseChange = false;
    digitalWrite(_LEDPin, LOW);
    CloseValvePin(_rewardPin);
    CloseValvePin(_optoPin); // turn off laser pin at end of experiment
    digitalWrite(_led, LOW);
    SSO_reward(false);
    CloseValvePin(_blankOdorPin);
    CloseValvePin(_unrewardedOdorPin);
    for (int odor = 0; odor < _numRewardedOdors; odor++) {
        CloseValvePin(_rewardedOdorPins[odor]);
    }
}

void TeensyGoNogo::update() {
    bool firstTime = (_currentState != _previousState);
    GoNogoState nextState = _currentState; // state stays the same unless explicitly changed
    String debugMsg;

    switch (_currentState) {
        case IDLE:
            // do nothing... wait for explicit start() command
            // if (firstTime) {
            //     Serial.println('I');
            // }
            break;

        case START_TRIAL:
            // load trial type from list, then wait for nose poke to start tone & stim
            if (_trialNum > _numTrials) {
                // Shouldn't get here
                nextState = END_RUN;
                debugOut("## Exceeded trial count!", 1);
            } else {
                int trialNumberForDisplay = _trialNum + 1;
                SSO_trial(trialNumberForDisplay);
                debugMsg = String("\n-- Trial ") + trialNumberForDisplay + " --";
                debugOut("-", 1);
                debugOut(debugMsg.c_str(), 1);
                _isRewardTrial = _trialTypeList[_trialNum] > 0;
                if (_isRewardTrial) {
                    _currentOdorPin = _rewardedOdorPins[_trialTypeList[_trialNum] - 1];
                    debugOut(" Reward trial.", 1);
                } else {
                    _currentOdorPin = _unrewardedOdorPin;
                    debugOut(" Null trial.", 1);
                }
                for (int bin = 0; bin < _numLickBins; bin++) {
                    _lickBins[bin] = 0;
                }
            }
            nextState = WAIT_FOR_NOSE_OUT;
            break;

        case WAIT_FOR_NOSE_OUT:
            if (isNoseOut()) {
                SSO_nose(isNoseIn());
                nextState = WAIT_FOR_POKE;
            }
            break;

        case WAIT_FOR_POKE:
            if (firstTime) {
                digitalWrite(_LEDPin, HIGH);
                debugOut("Waiting for nose in...",1);
            }
            if (isNoseIn()) {
                nextState = STIM_DELIVERY;
                digitalWrite(_LEDPin, LOW);
                SSO_nose(isNoseIn());
            }
            break;

        case STIM_DELIVERY:
            // start stimulus
            // Serial.print("Telling pin"); Serial.print(_currentOdorPin); Serial.println(" to open.");
            CloseValvePin(_blankOdorPin);
            OpenValvePin(_currentOdorPin);
            digitalWrite(_led, HIGH);
            OpenValvePin(_optoPin); //turn on opto pin to turn on laser 
            SSO_stimulus(_trialTypeList[_trialNum]);
            debugMsg = String("Stimulus delivery (") + _trialTypeList[_trialNum] + ")";
            debugOut(debugMsg.c_str(), 1);
            _stimTimer = 0;
            if (_isRewardTrial) {
                nextState = REWARD_WINDOW;
            } else {
                nextState = NO_REWARD_WINDOW;
            }
            break;

        case REWARD_WINDOW:
            // stay in REWARD_WINDOW for <rewardWindowDuration>:
            // - cout how many licks are detected in every bin

            if (firstTime) {
                _lickCount = 0;
                _prevLickWasLow = (digitalRead(_lickPin) == LOW);
            }

            // if NoseOut, then abort trial (miss)
            if (isNoseOut()) {
                // end stimulus
                OpenValvePin(_blankOdorPin);
                CloseValvePin(_currentOdorPin);
                digitalWrite(_led, LOW);
                CloseValvePin(_optoPin);// turn off laser pin because of miss
                SSO_nose(isNoseIn());
                SSO_stimulusEnd();
                SSO_trialOutcome(SSO_OUTCOME_MISS);
                debugOut("> Nose out - Miss - No Reward",1);
                nextState = END_TRIAL;
            } else {
                // check if we are still in stimulus window
                if (_stimTimer < _stimulusDuration_us) {
                    // check for licks and log in bins
                    // if (_lickCount > 0) {
                    //     int binNum = _stimTimer / _binSize_us; // integer division
                    //     _lickBins[binNum] += _lickCount;
                    //     _lickCount = 0;
                    // }
                    if (digitalRead(_lickPin) == HIGH) {
                        if (_prevLickWasLow) {
                            // detected lick onset
                            SSO_lick_outcome(_prevLickWasLow);
                            int binNum = _stimTimer / _binSize_us; // integer division
                            _lickBins[binNum]++;
                        }
                        _prevLickWasLow = false;
                    } else {
                        if (!_prevLickWasLow) {
                            SSO_lick_outcome(_prevLickWasLow);
                        }
                        _prevLickWasLow = true;
                    }
                } else {
                    // We've reached the end of the stimulus window
                    // end stimulus
                    OpenValvePin(_blankOdorPin);
                    CloseValvePin(_currentOdorPin);
                    digitalWrite(_led, LOW);
                    CloseValvePin(_optoPin); // turn off laser pin because reward trial ended
                    SSO_stimulusEnd();
                    int numBinsWithLicks = 0;
                    debugMsg = "Lick Bins: [ ";
                    for (int bin = 0; bin < _numLickBins; bin++) {
                        debugMsg += _lickBins[bin];
                        debugMsg += " ";
                        if (_lickBins[bin] > 0) {
                            numBinsWithLicks++;
                        }
                    }
                    debugMsg += "]";
                    debugOut(debugMsg.c_str(), 1);
                    if (numBinsWithLicks >= _minLickBins) {
                        // HIT
                        SSO_trialOutcome(SSO_OUTCOME_HIT);
                        //debugOut(num2str(_lickBins),1);
                        nextState = REWARD_DELIVERY;
                    } else {
                        // MISS
                        SSO_trialOutcome(SSO_OUTCOME_MISS);
                        //debugOut(num2str(_lickBins),1);
                        nextState = END_TRIAL;
                    }
                }
            }
            break;

        case NO_REWARD_WINDOW:
            if (firstTime) {
                _lickCount = 0;
                _prevLickWasLow = (digitalRead(_lickPin) == LOW);
            }
            // stay in NO_REWARD_WINDOW for <rewardWindowDuration>:
            // - if nose is out, then abort trial
            if (isNoseOut()) {
                // end stimulus
                OpenValvePin(_blankOdorPin);
                CloseValvePin(_currentOdorPin);
                digitalWrite(_led, LOW);
                CloseValvePin (_optoPin); // turn off laser pin because of CR
                SSO_nose(isNoseIn());
                SSO_stimulusEnd();
                SSO_trialOutcome(SSO_OUTCOME_CORRECT_REJECTION); // correct rejection
                nextState = END_TRIAL;
            } else {
                if (_stimTimer < _stimulusDuration_us) {
                    if (digitalRead(_lickPin) == HIGH) {
                        if (_prevLickWasLow) {
                            SSO_lick_outcome(_prevLickWasLow);
                            Serial.println(". Lick");
                            int binNum = _stimTimer / _binSize_us; //integer division
                            _lickBins[binNum]++;
                        }
                        _prevLickWasLow = false;
                    } else {
                        if (!_prevLickWasLow) {
                            SSO_lick_outcome(_prevLickWasLow);
                        }
                        _prevLickWasLow = true;
                    }
                } else {
                    OpenValvePin(_blankOdorPin);
                    CloseValvePin(_currentOdorPin);
                    CloseValvePin(_optoPin);// turn off laser pin because of FA
                    digitalWrite(_led, LOW);
                    SSO_stimulusEnd();
                    int numBinsWithLicks = 0;
                    Serial.print("Lick Bins: [ ");
                    for (int bin = 0; bin < _numLickBins; bin++) {
                        Serial.print(_lickBins[bin]);
                        Serial.print(" ");
                        if (_lickBins[bin] > 0) {
                            numBinsWithLicks++;
                        }
                    }
                    Serial.println("]");
                    if (numBinsWithLicks >= _minLickBins) {
                        debugOut("> False Alarm - Adding Penalty",1);
                        SSO_trialOutcome(SSO_OUTCOME_FALSE_ALARM);
                        nextState = NULLPENALTY_END_TRIAL;
                    } else {
                        debugOut("> Correct Rejection",1);
                        SSO_trialOutcome(SSO_OUTCOME_CORRECT_REJECTION);
                        nextState = END_TRIAL;
                    }
                }
            }
            break;

        case REWARD_DELIVERY:
            if (firstTime) {
                OpenValvePin(_rewardPin);
                _rewardTimer = 0;
                SSO_reward(true);
            } else if (_rewardTimer >= _rewardDuration_us) {
                CloseValvePin(_rewardPin);
                SSO_reward(false);
                nextState = END_TRIAL;
            }
            break;

        case NULLPENALTY_END_TRIAL:
            if (firstTime) {
               if (_trialNum == _numTrials) {
                    nextState = END_RUN;
               } else {
                    _itiTimer = 0;
                    debugOut("Inter-Trial-Interval with penalty...",1);
                }
            } else if (_itiTimer >= (_interTrialDuration_us + _nullPenalty_us)) {
                nextState = PAUSE_LICKING_WINDOW;
            }
            break;

        case END_TRIAL:
            if (firstTime) {
                _trialNum = _trialNum + 1;
                if (_trialNum == _numTrials) {
                    nextState = END_RUN;
                } else {
                    _itiTimer = 0;
                    debugOut("Inter-Trial-Interval...",1);
                }
            } else if (_itiTimer >= _interTrialDuration_us) {
                nextState = PAUSE_LICKING_WINDOW;
            }
            break;

        case PAUSE_LICKING_WINDOW:
            if (firstTime) {
                _pause_licking_timer = 0;
                _pause_lickCount = 0;
                _prevLickWasLow = (digitalRead(_lickPin) == LOW);
            }

            if (digitalRead(_lickPin) == HIGH) {
                if (_prevLickWasLow) {
                    Serial.println(". Lick");
                    _pause_lickCount++;
                }
                _prevLickWasLow = false;
            } else {
                    _prevLickWasLow = true;
                }
            if (_pause_lickCount > _lickPenaltyThresh) {
                debugOut(">  Licked during ITI. Reset timer", 1);
                _pause_lickCount = 0;
                _pause_licking_timer = 0;
            } else if (_pause_licking_timer > _pauseLickingDelay_us) {
            nextState = START_TRIAL;
            }

        break;  // switch

        case END_RUN:
            SSO_endSession();
            debugOut("==== END OF RUN ===\n", 1);

            // reset state for next run
            _trialNum = 0;
            nextState = IDLE;
            break;

        case MANUAL_REWARD_DELIVERY_ON:
            if (firstTime) {
                if (_numRewardsRemaining < 1) {
                    // ERROR
                    debugOut("ERROR: _numRewardsRemaining is 0.\n", 1);
                    nextState = IDLE;
                    break;
                }
                _numRewardsRemaining--;
                OpenValvePin(_rewardPin);
                SSO_reward(true);
                _rewardTimer = 0;
            } else if (_rewardTimer >= _rewardDuration_us) {
                CloseValvePin(_rewardPin);
                SSO_reward(false);
                if (_numRewardsRemaining == 0) {
                    nextState = IDLE;
                } else {
                    nextState = MANUAL_REWARD_DELIVERY_OFF;
                }
            }
            break;

        case MANUAL_REWARD_DELIVERY_OFF:
            if (firstTime) {
                _rewardTimer = 0;
            } else if (_rewardTimer >= _interRewardDuration_us) {
                nextState = MANUAL_REWARD_DELIVERY_ON;
            }
            break;

        default:
            // ERROR should never get here
            break;

    }  // switch

    //if (_currentState != nextState) {
    if (firstTime) {
        // debugOut("State: ", 2);
        // debugOut(getStateString(_currentState), 2);
        SSO_newState(_currentState);
    }
    _previousState = _currentState;
    _currentState = nextState;
} // update()

void TeensyGoNogo::debugOut(const char* msg, int debugLevel) {
    // TODO: check debugLevel
    if (debugLevel <= _debugLevel) {
        SSO_comment(msg);
    }
}
