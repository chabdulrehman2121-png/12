#include "trains.h"
#include "simulation_state.h"
#include "grid.h"
#include "switches.h"
#include "io.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

// ============================================================================
// TRAINS.CPP - Train logic
// ============================================================================

// Storage for planned moves (for collisions).
// PlannedMove array: 0=trainIndex, 1=nextX, 2=nextY, 3=distance
const int PLANNED_TRAIN_IDX = 0;
const int PLANNED_NEXT_X = 1;
const int PLANNED_NEXT_Y = 2;
const int PLANNED_DISTANCE = 3;
const int PLANNED_FIELDS = 4;

int plannedMoves[MAX_TRAINS][PLANNED_FIELDS];
int numPlannedMoves = 0;

// Previous positions (to detect switch entry).
int prevX[MAX_TRAINS], prevY[MAX_TRAINS];

// ----------------------------------------------------------------------------
// SPAWN TRAINS FOR CURRENT TICK
// ----------------------------------------------------------------------------
// Activate trains scheduled for this tick.
// ----------------------------------------------------------------------------
void spawnTrainsForTick() {
    for (int i = 0; i < numTrains; i++) {
        if (trains[i][TRAIN_STATE] == TRAIN_SCHEDULED && trains[i][TRAIN_SPAWN_TICK] == currentTick) {
            trains[i][TRAIN_STATE] = TRAIN_ACTIVE;
            activeTrains++;
            
            // Store previous position
            prevX[i] = trains[i][TRAIN_X];
            prevY[i] = trains[i][TRAIN_Y];
            
            logTrainTrace(trains[i][TRAIN_ID], trains[i][TRAIN_X], trains[i][TRAIN_Y], trains[i][TRAIN_DIRECTION], "SPAWNED");
        }
    }
}

// ----------------------------------------------------------------------------
// DETERMINE NEXT POSITION for a train
// ----------------------------------------------------------------------------
// Compute next position/direction from current tile and rules.
// ----------------------------------------------------------------------------
bool determineNextPosition(int trainIndex) {
    // Store current position as previous
    prevX[trainIndex] = trains[trainIndex][TRAIN_X];
    prevY[trainIndex] = trains[trainIndex][TRAIN_Y];
    
    // Calculate next position based on current direction
    int nextX = trains[trainIndex][TRAIN_X] + dx[trains[trainIndex][TRAIN_DIRECTION]];
    int nextY = trains[trainIndex][TRAIN_Y] + dy[trains[trainIndex][TRAIN_DIRECTION]];
    
    // Check if next position is valid
    if (!isInBounds(nextX, nextY) || !isTrackTile(nextX, nextY)) {
        // Train would go off track - crash it
        trains[trainIndex][TRAIN_STATE] = TRAIN_CRASHED;
        trainsCrashed++;
        activeTrains--;
        logTrainTrace(trains[trainIndex][TRAIN_ID], trains[trainIndex][TRAIN_X], trains[trainIndex][TRAIN_Y], trains[trainIndex][TRAIN_DIRECTION], "CRASHED");
        return false;
    }
    
    // Calculate distance to destination for priority system
    int distance = abs(nextX - trains[trainIndex][TRAIN_DEST_X]) + abs(nextY - trains[trainIndex][TRAIN_DEST_Y]);
    
    // Add to planned moves for collision detection
    if (numPlannedMoves < MAX_TRAINS) {
        plannedMoves[numPlannedMoves][PLANNED_TRAIN_IDX] = trainIndex;
        plannedMoves[numPlannedMoves][PLANNED_NEXT_X] = nextX;
        plannedMoves[numPlannedMoves][PLANNED_NEXT_Y] = nextY;
        plannedMoves[numPlannedMoves][PLANNED_DISTANCE] = distance;
        numPlannedMoves++;
    }
    
    return true;
}

// ----------------------------------------------------------------------------
// GET NEXT DIRECTION based on current tile and direction
// ----------------------------------------------------------------------------
// Return new direction after entering the tile.
// ----------------------------------------------------------------------------
int getNextDirection(int x, int y, int currentDir, int trainIndex) {
    if (!isInBounds(x, y)) return currentDir;
    
    char tile = grid[x][y];
    
    // Handle switches
    if (isSwitchTile(x, y)) {
        int switchIndex = getSwitchIndex(tile);
        if (switchIndex >= 0) {
            // Determine exit direction based on switch state
            if (switches[switchIndex][SWITCH_CURRENT_STATE] == 0) {
                // STRAIGHT state - maintain direction
                return currentDir;
            } else {
                // TURN state - turn the train
                switch (currentDir) {
                    case DIR_UP: return DIR_RIGHT;
                    case DIR_RIGHT: return DIR_DOWN;
                    case DIR_DOWN: return DIR_LEFT;
                    case DIR_LEFT: return DIR_UP;
                }
            }
        }
    }
    
    // Handle crossings (+)
    if (tile == '+') {
        return getSmartDirectionAtCrossing(x, y, currentDir, trainIndex);
    }
    
    // Handle track curves
    if (tile == '\\') {
        switch (currentDir) {
            case DIR_UP: return DIR_LEFT;
            case DIR_DOWN: return DIR_RIGHT;
            case DIR_LEFT: return DIR_UP;
            case DIR_RIGHT: return DIR_DOWN;
        }
    } else if (tile == '/') {
        switch (currentDir) {
            case DIR_UP: return DIR_RIGHT;
            case DIR_DOWN: return DIR_LEFT;
            case DIR_LEFT: return DIR_DOWN;
            case DIR_RIGHT: return DIR_UP;
        }
    }
    
    // For straight tracks (-, =, |) and destinations (D), maintain direction
    return currentDir;
}

// ----------------------------------------------------------------------------
// SMART ROUTING AT CROSSING - Route train to its matched destination
// ----------------------------------------------------------------------------
// Choose best direction at '+' toward destination.
// ----------------------------------------------------------------------------
int getSmartDirectionAtCrossing(int x, int y, int currentDir, int trainIndex) {
    // Simple approach for easy level: find the direction that gets closest to destination
    int bestDirection = currentDir;
    int bestDistance = 999;
    
    for (int dir = 0; dir < 4; dir++) {
        int nextX = x + dx[dir];
        int nextY = y + dy[dir];
        
        // Check if this direction is valid
        if (isInBounds(nextX, nextY) && isTrackTile(nextX, nextY)) {
            int distance = abs(nextX - trains[trainIndex][TRAIN_DEST_X]) + abs(nextY - trains[trainIndex][TRAIN_DEST_Y]);
            if (distance < bestDistance) {
                bestDistance = distance;
                bestDirection = dir;
            }
        }
    }
    
    return bestDirection;
}

// ----------------------------------------------------------------------------
// DETERMINE ALL ROUTES (PHASE 2)
// ----------------------------------------------------------------------------
// Fill next positions/directions for all trains.
// ----------------------------------------------------------------------------
void determineAllRoutes() {
    numPlannedMoves = 0;  // Clear planned moves
    
    for (int i = 0; i < numTrains; i++) {
        if (trains[i][TRAIN_STATE] == TRAIN_ACTIVE) {
            determineNextPosition(i);
        }
    }
}

// ----------------------------------------------------------------------------
// MOVE ALL TRAINS (PHASE 5)
// ----------------------------------------------------------------------------
// Move trains; resolve collisions and apply effects.
// ----------------------------------------------------------------------------
void moveAllTrains() {
    // First detect and resolve collisions
    detectCollisions();
    
    // Then move all non-crashed trains
    for (int moveIndex = 0; moveIndex < numPlannedMoves; moveIndex++) {
        int i = plannedMoves[moveIndex][PLANNED_TRAIN_IDX];
        int nextX = plannedMoves[moveIndex][PLANNED_NEXT_X];
        int nextY = plannedMoves[moveIndex][PLANNED_NEXT_Y];
        
        if (trains[i][TRAIN_STATE] == TRAIN_ACTIVE) {
            // Check for safety tiles that cause delays
            if (safetyTiles[trains[i][TRAIN_X]][trains[i][TRAIN_Y]]) {
                trains[i][TRAIN_WAIT_TICKS]++;
                totalWaitTicks++;
                
                // Apply weather effects
                int delayTicks = 1; // Normal delay
                if (weather == WEATHER_RAIN) delayTicks = 2;
                if (weather == WEATHER_FOG) delayTicks = 3;
                
                if (trains[i][TRAIN_WAIT_TICKS] >= delayTicks) {
                    trains[i][TRAIN_WAIT_TICKS] = 0; // Reset wait counter
                    
                    // Move the train
                    trains[i][TRAIN_X] = nextX;
                    trains[i][TRAIN_Y] = nextY;
                    trains[i][TRAIN_DIRECTION] = getNextDirection(nextX, nextY, trains[i][TRAIN_DIRECTION], i);
                    
                    logTrainTrace(trains[i][TRAIN_ID], trains[i][TRAIN_X], trains[i][TRAIN_Y], trains[i][TRAIN_DIRECTION], "MOVING");
                }
            } else {
                // Move the train normally
                trains[i][TRAIN_X] = nextX;
                trains[i][TRAIN_Y] = nextY;
                trains[i][TRAIN_DIRECTION] = getNextDirection(nextX, nextY, trains[i][TRAIN_DIRECTION], i);
                
                logTrainTrace(trains[i][TRAIN_ID], trains[i][TRAIN_X], trains[i][TRAIN_Y], trains[i][TRAIN_DIRECTION], "MOVING");
            }
        }
    }
}

// ----------------------------------------------------------------------------
// DETECT COLLISIONS WITH PRIORITY SYSTEM
// ----------------------------------------------------------------------------
// Resolve same-tile, swap, and crossing conflicts.
// ----------------------------------------------------------------------------
void detectCollisions() {
    // Simple bubble sort by distance (higher distance = higher priority) 
    for (int i = 0; i < numPlannedMoves - 1; i++) {
        for (int j = 0; j < numPlannedMoves - i - 1; j++) {
            if (plannedMoves[j][PLANNED_DISTANCE] < plannedMoves[j + 1][PLANNED_DISTANCE]) {
                // Swap
                for (int k = 0; k < PLANNED_FIELDS; k++) {
                    int temp = plannedMoves[j][k];
                    plannedMoves[j][k] = plannedMoves[j + 1][k];
                    plannedMoves[j + 1][k] = temp;
                }
            }
        }
    }
    
    // Check for collisions
    for (int i = 0; i < numPlannedMoves; i++) {
        for (int j = i + 1; j < numPlannedMoves; j++) {
            // Same destination collision
            if (plannedMoves[i][PLANNED_NEXT_X] == plannedMoves[j][PLANNED_NEXT_X] && 
                plannedMoves[i][PLANNED_NEXT_Y] == plannedMoves[j][PLANNED_NEXT_Y]) {
                
                int trainI = plannedMoves[i][PLANNED_TRAIN_IDX];
                int trainJ = plannedMoves[j][PLANNED_TRAIN_IDX];
                
                // Higher distance train (i) gets priority, lower distance train (j) waits
                if (plannedMoves[i][PLANNED_DISTANCE] > plannedMoves[j][PLANNED_DISTANCE]) {
                    trains[trainJ][TRAIN_WAIT_TICKS]++;
                } else if (plannedMoves[j][PLANNED_DISTANCE] > plannedMoves[i][PLANNED_DISTANCE]) {
                    trains[trainI][TRAIN_WAIT_TICKS]++;
                } else {
                    // Equal distance - both crash
                    trains[trainI][TRAIN_STATE] = TRAIN_CRASHED;
                    trains[trainJ][TRAIN_STATE] = TRAIN_CRASHED;
                    trainsCrashed += 2;
                    activeTrains -= 2;
                    
                    logTrainTrace(trains[trainI][TRAIN_ID], 
                                trains[trainI][TRAIN_X], 
                                trains[trainI][TRAIN_Y], 
                                trains[trainI][TRAIN_DIRECTION], "CRASHED");
                    logTrainTrace(trains[trainJ][TRAIN_ID], 
                                trains[trainJ][TRAIN_X], 
                                trains[trainJ][TRAIN_Y], 
                                trains[trainJ][TRAIN_DIRECTION], "CRASHED");
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// CHECK ARRIVALS
// ----------------------------------------------------------------------------
// Mark trains that reached destinations.
// ----------------------------------------------------------------------------
void checkArrivals() {
    for (int i = 0; i < numTrains; i++) {
        if (trains[i][TRAIN_STATE] == TRAIN_ACTIVE) {
            if (trains[i][TRAIN_X] == trains[i][TRAIN_DEST_X] && trains[i][TRAIN_Y] == trains[i][TRAIN_DEST_Y]) {
                trains[i][TRAIN_STATE] = TRAIN_DELIVERED;
                trainsDelivered++;
                activeTrains--;
                
                logTrainTrace(trains[i][TRAIN_ID], trains[i][TRAIN_X], trains[i][TRAIN_Y], trains[i][TRAIN_DIRECTION], "DELIVERED");
            }
        }
    }
}

// ----------------------------------------------------------------------------
// APPLY EMERGENCY HALT
// ----------------------------------------------------------------------------
// Apply halt to trains in the active zone.
// ----------------------------------------------------------------------------
void applyEmergencyHalt() {
    if (!emergencyHaltActive) return;
    
    for (int i = 0; i < numTrains; i++) {
        if (trains[i][TRAIN_STATE] == TRAIN_ACTIVE) {
            int distance = abs(trains[i][TRAIN_X] - emergencyHaltX) + abs(trains[i][TRAIN_Y] - emergencyHaltY);
            if (distance <= emergencyHaltRange) {
                trains[i][TRAIN_WAIT_TICKS] += 3; // Halt for 3 ticks
                totalWaitTicks += 3;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// UPDATE EMERGENCY HALT
// ----------------------------------------------------------------------------
// Decrement timer and disable when done.
// ----------------------------------------------------------------------------
void updateEmergencyHalt() {
    if (emergencyHaltActive) {
        emergencyHaltTicks--;
        if (emergencyHaltTicks <= 0) {
            emergencyHaltActive = false;
        }
    }
}
