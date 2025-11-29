#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

#include <string>

// ============================================================================
// SIMULATION_STATE.H - Global constants and state
// ============================================================================
// Global constants and arrays used by the game.
// ============================================================================

// ----------------------------------------------------------------------------
// GRID CONSTANTS
// ----------------------------------------------------------------------------
const int MAX_ROWS = 50;
const int MAX_COLS = 100;

// Directions: 0=UP, 1=RIGHT, 2=DOWN, 3=LEFT
const int DIR_UP = 0;
const int DIR_RIGHT = 1; 
const int DIR_DOWN = 2;
const int DIR_LEFT = 3;

// Direction vectors
const int dx[4] = {-1, 0, 1, 0}; // UP, RIGHT, DOWN, LEFT
const int dy[4] = {0, 1, 0, -1};

// ----------------------------------------------------------------------------
// TRAIN CONSTANTS
// ----------------------------------------------------------------------------
const int MAX_TRAINS = 100;

enum TrainState {
    TRAIN_SCHEDULED,
    TRAIN_ACTIVE, 
    TRAIN_DELIVERED,
    TRAIN_CRASHED
};

// Train data represented as parallel arrays
// Index mapping: 0=id, 1=spawnTick, 2=x, 3=y, 4=direction, 5=colorIndex, 
//                6=destinationX, 7=destinationY, 8=state, 9=waitTicks
const int TRAIN_ID = 0;
const int TRAIN_SPAWN_TICK = 1;
const int TRAIN_X = 2;
const int TRAIN_Y = 3;
const int TRAIN_DIRECTION = 4;
const int TRAIN_COLOR_INDEX = 5;
const int TRAIN_DEST_X = 6;
const int TRAIN_DEST_Y = 7;
const int TRAIN_STATE = 8;
const int TRAIN_WAIT_TICKS = 9;
const int TRAIN_FIELDS = 10;

// ----------------------------------------------------------------------------
// SWITCH CONSTANTS
// ----------------------------------------------------------------------------
const int MAX_SWITCHES = 26; // A-Z

enum SwitchMode {
    PER_DIR,
    GLOBAL
};

// Switch data represented as parallel arrays
// Index mapping: 0=letter, 1=mode, 2=initState, 3-6=kValues[4], 7-10=counters[4],
//                11=globalCounter, 12=state0_hash, 13=state1_hash, 14=currentState, 
//                15=flipQueued, 16=x, 17=y
const int SWITCH_LETTER = 0;
const int SWITCH_MODE = 1;
const int SWITCH_INIT_STATE = 2;
const int SWITCH_K0 = 3;
const int SWITCH_K1 = 4;
const int SWITCH_K2 = 5;
const int SWITCH_K3 = 6;
const int SWITCH_COUNTER0 = 7;
const int SWITCH_COUNTER1 = 8;
const int SWITCH_COUNTER2 = 9;
const int SWITCH_COUNTER3 = 10;
const int SWITCH_GLOBAL_COUNTER = 11;
const int SWITCH_CURRENT_STATE = 12;
const int SWITCH_FLIP_QUEUED = 13;
const int SWITCH_X = 14;
const int SWITCH_Y = 15;
const int SWITCH_FIELDS = 16;

// Helper arrays for switch state names (separate from int arrays)
extern std::string switchStateNames[MAX_SWITCHES][2];

// ----------------------------------------------------------------------------
// WEATHER CONSTANTS
// ----------------------------------------------------------------------------
enum WeatherType {
    WEATHER_NORMAL,
    WEATHER_RAIN,
    WEATHER_FOG
};

// ----------------------------------------------------------------------------
// SIGNAL CONSTANTS
// ----------------------------------------------------------------------------
enum SignalColor {
    SIGNAL_GREEN,
    SIGNAL_YELLOW, 
    SIGNAL_RED
};

// ----------------------------------------------------------------------------
// GLOBAL STATE: GRID
// ----------------------------------------------------------------------------
extern char grid[MAX_ROWS][MAX_COLS];
extern bool safetyTiles[MAX_ROWS][MAX_COLS];
extern int gridRows, gridCols;

// ----------------------------------------------------------------------------
// GLOBAL STATE: TRAINS
// ----------------------------------------------------------------------------
extern int trains[MAX_TRAINS][TRAIN_FIELDS];
extern int numTrains;
extern int activeTrains;

// ----------------------------------------------------------------------------
// GLOBAL STATE: SWITCHES (A-Z mapped to 0-25)
// ----------------------------------------------------------------------------
extern int switches[MAX_SWITCHES][SWITCH_FIELDS];
extern int numSwitches;

// ----------------------------------------------------------------------------
// GLOBAL STATE: SPAWN POINTS
// ----------------------------------------------------------------------------
// SpawnPoint data: 0=x, 1=y, 2=active
const int SPAWN_X = 0;
const int SPAWN_Y = 1;
const int SPAWN_ACTIVE = 2;
const int SPAWN_FIELDS = 3;

extern int spawnPoints[10][SPAWN_FIELDS];
extern int numSpawnPoints;

// ----------------------------------------------------------------------------
// GLOBAL STATE: DESTINATION POINTS
// ----------------------------------------------------------------------------
// DestinationPoint data: 0=x, 1=y, 2=active
const int DEST_X = 0;
const int DEST_Y = 1;
const int DEST_ACTIVE = 2;
const int DEST_FIELDS = 3;

extern int destinationPoints[10][DEST_FIELDS];
extern int numDestinationPoints;

// ----------------------------------------------------------------------------
// GLOBAL STATE: SIMULATION PARAMETERS
// ----------------------------------------------------------------------------
extern std::string levelName;
extern int seed;
extern WeatherType weather;
extern int currentTick;

// ----------------------------------------------------------------------------
// GLOBAL STATE: METRICS
// ----------------------------------------------------------------------------
extern int trainsDelivered;
extern int trainsCrashed;
extern int switchFlips;
extern int totalWaitTicks;

// ----------------------------------------------------------------------------
// GLOBAL STATE: EMERGENCY HALT
// ----------------------------------------------------------------------------
extern bool emergencyHaltActive;
extern int emergencyHaltTicks;
extern int emergencyHaltX, emergencyHaltY, emergencyHaltRange;

// ----------------------------------------------------------------------------
// INITIALIZATION FUNCTION
// ----------------------------------------------------------------------------
// Resets all state before loading a new level.
void initializeSimulationState();

#endif
