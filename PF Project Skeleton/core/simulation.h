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

struct Train {
    int id;
    int spawnTick;
    int x, y;
    int direction;
    int colorIndex;
    int destinationX, destinationY;
    TrainState state;
    int waitTicks; // for safety buffer delays
};

// ----------------------------------------------------------------------------
// SWITCH CONSTANTS
// ----------------------------------------------------------------------------
const int MAX_SWITCHES = 26; // A-Z

enum SwitchMode {
    PER_DIR,
    GLOBAL
};

struct Switch {
    char letter;
    SwitchMode mode;
    int initState;
    int kValues[4]; // K-values for UP, RIGHT, DOWN, LEFT
    int counters[4]; // counters for each direction
    int globalCounter;
    std::string states[2]; // state names (e.g., "STRAIGHT", "TURN")
    int currentState; // 0 or 1
    bool flipQueued;
    int x, y; // position on grid
};

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
extern Train trains[MAX_TRAINS];
extern int numTrains;
extern int activeTrains;

// ----------------------------------------------------------------------------
// GLOBAL STATE: SWITCHES (A-Z mapped to 0-25)
// ----------------------------------------------------------------------------
extern Switch switches[MAX_SWITCHES];
extern int numSwitches;

// ----------------------------------------------------------------------------
// GLOBAL STATE: SPAWN POINTS
// ----------------------------------------------------------------------------
struct SpawnPoint {
    int x, y;
    bool active;
};
extern SpawnPoint spawnPoints[10];
extern int numSpawnPoints;

// ----------------------------------------------------------------------------
// GLOBAL STATE: DESTINATION POINTS
// ----------------------------------------------------------------------------
struct DestinationPoint {
    int x, y;
    bool active;
};
extern DestinationPoint destinationPoints[10];
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
