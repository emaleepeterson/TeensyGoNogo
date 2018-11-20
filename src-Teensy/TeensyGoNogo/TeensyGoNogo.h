
#ifndef _TEENSY_GO_NOGO_
#define _TEENSY_GO_NOGO_

#include <Arduino.h>


enum state {
    IDLE,

    // States for normal behavior protocol
    START_TRIAL,
    WAIT_FOR_NOSE_OUT,
    WAIT_FOR_POKE,
    STIM_DELIVERY,
    REWARD_WINDOW,
    NO_REWARD_WINDOW,
    REWARD_DELIVERY,
    END_RUN,
    NULLPENALTY_END_TRIAL,
    END_TRIAL,
    PAUSE_LICKING_WINDOW,

    // Other states
    MANUAL_REWARD_DELIVERY_ON,
    MANUAL_REWARD_DELIVERY_OFF
};
typedef enum state GoNogoState;

inline const char* getStateString(GoNogoState state) {
    const char* stateStr = "";
    if (state == IDLE)                      {stateStr = "IDLE";}
    else if (state == START_TRIAL)          {stateStr = "START_TRIAL";}
    else if (state == WAIT_FOR_NOSE_OUT)    {stateStr = "WAIT_FOR_NOSE_OUT";}
    else if (state == WAIT_FOR_POKE)        {stateStr = "WAIT_FOR_POKE";}
    else if (state == STIM_DELIVERY)        {stateStr = "STIM_DELIVERY";}
    else if (state == REWARD_WINDOW)        {stateStr = "REWARD_WINDOW";}
    else if (state == NO_REWARD_WINDOW)     {stateStr = "NO_REWARD_WINDOW";}
    else if (state == REWARD_DELIVERY)      {stateStr = "REWARD_DELIVERY";}
    else if (state == END_RUN)              {stateStr = "END_RUN";}
    else if (state == NULLPENALTY_END_TRIAL){stateStr = "NULLPENALTY_END_TRIAL";}
    else if (state == END_TRIAL)            {stateStr = "END_TRIAL";}
    else if (state == PAUSE_LICKING_WINDOW) {stateStr = "PAUSE_LICKING_WINDOW";}
    else if (state == MANUAL_REWARD_DELIVERY_ON)    {stateStr = "MANUAL_REWARD_DELIVERY_ON";}
    else if (state == MANUAL_REWARD_DELIVERY_OFF)   {stateStr = "MANUAL_REWARD_DELIVERY_OFF";}
    return stateStr;
}

class TeensyGoNogo {
public:
    static const int MAX_TRIALS = 4096;
    static const int MAX_BINS = 32;
    static const int MAX_ODOR_PINS = 5;

    // debug/verbose output
    uint8_t _debugLevel = 1;

private:
    // user-defined preferences
    float _pStim;
    unsigned int _numTrials;
    unsigned int _maxRunLength;
    uint8_t _numLickBins;
    uint8_t _minLickBins;
    uint8_t numRewardedOdors;
    unsigned long _stimulusDuration_us;
    unsigned long _binSize_us; // auto-computed from above vars
    unsigned long _rewardDuration_us;
    unsigned long _interTrialDuration_us;
    unsigned long _interRewardDuration_us; // for multi-reward delivery
    unsigned long _nullPenalty_us;
    unsigned long _pauseLickingDelay_us;
    unsigned int _lickPenaltyThresh;

    // pre-computed trial type information
    uint8_t _trialTypeList[MAX_TRIALS];
    uint8_t _lickBins[MAX_BINS];

    // state variables
    bool _isRunning;
    GoNogoState _currentState;
    GoNogoState _previousState;
    unsigned int _trialNum;
    bool _isRewardTrial;
    bool _currentLickState;
    int _currentOdorPin;
    unsigned int _numRewardsRemaining;

    // timing variable
    elapsedMicros _stimTimer;
    elapsedMicros _rewardTimer;
    elapsedMicros _itiTimer;
    elapsedMicros _pause_licking_timer;
    unsigned long _lickTime;

    // links to hardware
    
    //int _led;
    uint8_t _optoPin;
    uint8_t _beambreakPin;
    uint8_t _lickPin;
    uint8_t _LEDPin;
    uint8_t _rewardPin;
    uint8_t _blankOdorPin;
    uint8_t _unrewardedOdorPin;
    uint8_t _rewardedOdorPins[MAX_ODOR_PINS];
    uint8_t _numRewardedOdors;
    // interrupt counters
    volatile int _lickCount = 0;
    volatile unsigned int _pause_lickCount = 0;
    volatile bool _noseChange = false;
    // hack to replace interrupts:
    bool _prevLickWasLow;


public:
    TeensyGoNogo();
    ~TeensyGoNogo();
    void run();
    void update();
    void stop();
    void manualReward();
    void multiManualReward(unsigned int numRewards);

    bool isNoseIn() {return digitalRead(_beambreakPin)==HIGH;};
    bool isNoseOut() {return digitalRead(_beambreakPin)==LOW;};

    bool isOptoOn() {return digitalRead(_optoPin)==HIGH;};
    bool isOptoOff() {return digitalRead(_optoPin)==LOW;};

    // set and get user-modifiable parameters
    void set_numRewardedOdors(uint8_t numRewardedOdors);
    void set_rewardedOdorPin(uint8_t rewardedOdorNum, uint8_t pin);
    void set_optoPin(uint8_t optoPin);
    void set_beambreakPin(uint8_t beambreakPin);
    void set_lickPin(uint8_t lickPin);
    void set_LEDPin(uint8_t LEDPin);
    void set_rewardPin(uint8_t rewardPin);
    void set_blankOdorPin(uint8_t blankOdorPin);
    void set_unrewardedOdorPin(uint8_t unrewardedOdorPin);
    void set_pStim(float pStim);
    void set_numTrials(unsigned int numTrials);
    void set_maxRunLength(unsigned int maxRunLength);
    void set_minLickBins(uint8_t minLickBins);
    void set_numLickBins(uint8_t numLickBins);
    void set_stimulusDuration_ms(unsigned long stimulusDuration_ms);
    void set_rewardDuration_ms(unsigned long rewardDuration_ms);
    void set_interTrialDuration_ms(unsigned long interTrialDuration_ms);
    void set_interRewardDuration_ms(unsigned long interRewardDuration_ms);
    void set_nullPenalty_ms(unsigned long _nullPenalty_ms);
    void set_pauseLickingDelay_ms(unsigned long pauseLickingDelay_ms);
    void set_lickPenaltyThresh(unsigned int lickPenaltyThresh);

    uint8_t get_numRewardedOdors() {return _numRewardedOdors;};
    int get_rewardedOdorPin(uint8_t rewardedOdorNum); // defined in .cpp
    uint8_t get_optoPin() {return _optoPin;};
    uint8_t get_beambreakPin() {return _beambreakPin;};
    uint8_t get_lickPin() {return _lickPin;};
    uint8_t get_LEDPin() {return _LEDPin;};
    uint8_t get_rewardPin() {return _rewardPin;};
    uint8_t get_blankOdorPin() {return _blankOdorPin;};
    uint8_t get_unrewardedOdorPin() {return _unrewardedOdorPin;};
    float get_pStim() {return _pStim;};
    unsigned int get_numTrials() {return _numTrials;};
    unsigned int get_maxRunLength() {return _maxRunLength;};
    uint8_t get_minLickBins() {return _minLickBins;};
    uint8_t get_numLickBins() {return _numLickBins;};
    unsigned long get_stimulusDuration_ms() {return _stimulusDuration_us/1000;};
    unsigned long get_rewardDuration_ms() {return _rewardDuration_us/1000;};
    unsigned long get_interTrialDuration_ms() {return _interTrialDuration_us/1000;};
    unsigned long get_interRewardDuration_ms() {return _interRewardDuration_us/1000;};
    unsigned long get_nullPenalty_ms() {return _nullPenalty_us/1000;};
    unsigned long get_pauseLickingDelay_ms() {return _pauseLickingDelay_us/1000;};
    unsigned int get_lickPenaltyThresh() {return _lickPenaltyThresh;};

    void debugOut(const char* msg, int debugLevel);

// private:
    void nosePokeInterrupt();
    void lickInterrupt();

};

#endif //_TEENSY_GO_NOGO_
