#include "simulation_state.h"
#include <cstring>

// ============================================================================
// SIMULATION_STATE.CPP - Global state definitions
// ============================================================================

// ----------------------------------------------------------------------------
// GRID
// ----------------------------------------------------------------------------
char grid[MAX_ROWS][MAX_COLS];
bool safetyTiles[MAX_ROWS][MAX_COLS];
int gridRows = 0, gridCols = 0;

// ----------------------------------------------------------------------------
// TRAINS
// ----------------------------------------------------------------------------
Train trains[MAX_TRAINS];
int numTrains = 0;
int activeTrains = 0;

// ----------------------------------------------------------------------------
// SWITCHES
// ----------------------------------------------------------------------------
Switch switches[MAX_SWITCHES];
int numSwitches = 0;

// ----------------------------------------------------------------------------
// SPAWN AND DESTINATION POINTS
// ----------------------------------------------------------------------------
SpawnPoint spawnPoints[10];
int numSpawnPoints = 0;
DestinationPoint destinationPoints[10];
int numDestinationPoints = 0;

// ----------------------------------------------------------------------------
// SIMULATION PARAMETERS
// ----------------------------------------------------------------------------
std::string levelName;
int seed = 0;
WeatherType weather = WEATHER_NORMAL;
int currentTick = 0;

// ----------------------------------------------------------------------------
// METRICS
// ----------------------------------------------------------------------------
int trainsDelivered = 0;
int trainsCrashed = 0;
int switchFlips = 0;
int totalWaitTicks = 0;

// ----------------------------------------------------------------------------
// EMERGENCY HALT
// ----------------------------------------------------------------------------
bool emergencyHaltActive = false;
int emergencyHaltTicks = 0;
int emergencyHaltX = 0, emergencyHaltY = 0, emergencyHaltRange = 3;

// ============================================================================
// INITIALIZE SIMULATION STATE
// ============================================================================
// ----------------------------------------------------------------------------
// Resets all global simulation state.
// ----------------------------------------------------------------------------
// Called before loading a new level.
// ----------------------------------------------------------------------------
void initializeSimulationState() {
    // Clear grid
    memset(grid, ' ', sizeof(grid));
    memset(safetyTiles, false, sizeof(safetyTiles));
    gridRows = gridCols = 0;
    
    // Reset trains
    memset(trains, 0, sizeof(trains));
    numTrains = 0;
    activeTrains = 0;
    
    // Reset switches
    memset(switches, 0, sizeof(switches));
    numSwitches = 0;
    
    // Clear spawn/destination points
    numSpawnPoints = 0;
    numDestinationPoints = 0;
    for (int i = 0; i < 10; i++) {
        spawnPoints[i].active = false;
        destinationPoints[i].active = false;
    }
    
    // Reset simulation parameters
    levelName.clear();
    seed = 0;
    weather = WEATHER_NORMAL;
    currentTick = 0;
    
    // Reset metrics
    trainsDelivered = 0;
    trainsCrashed = 0;
    switchFlips = 0;
    totalWaitTicks = 0;
    
    // Reset emergency halt
    emergencyHaltActive = false;
    emergencyHaltTicks = 0;
    emergencyHaltX = emergencyHaltY = 0;
}
