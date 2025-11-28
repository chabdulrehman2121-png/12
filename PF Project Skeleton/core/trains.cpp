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
struct PlannedMove {
    int trainIndex;
    int nextX, nextY;
    int distance; // to destination
};
PlannedMove plannedMoves[MAX_TRAINS];
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
        if (trains[i].state == TRAIN_SCHEDULED && trains[i].spawnTick == currentTick) {
            trains[i].state = TRAIN_ACTIVE;
            activeTrains++;
            
            // Store previous position
            prevX[i] = trains[i].x;
            prevY[i] = trains[i].y;
            
            logTrainTrace(trains[i].id, trains[i].x, trains[i].y, trains[i].direction, "SPAWNED");
        }
    }
}

// ----------------------------------------------------------------------------
// DETERMINE NEXT POSITION for a train
// ----------------------------------------------------------------------------
// Compute next position/direction from current tile and rules.
// ----------------------------------------------------------------------------
bool determineNextPosition(int trainIndex) {
    Train& train = trains[trainIndex];
    
    // Store current position as previous
    prevX[trainIndex] = train.x;
    prevY[trainIndex] = train.y;
    
    // Calculate next position based on current direction
    int nextX = train.x + dx[train.direction];
    int nextY = train.y + dy[train.direction];
    
    // Check if next position is valid
    if (!isInBounds(nextX, nextY) || !isTrackTile(nextX, nextY)) {
        // Train would go off track - crash it
        train.state = TRAIN_CRASHED;
        trainsCrashed++;
        activeTrains--;
        logTrainTrace(train.id, train.x, train.y, train.direction, "CRASHED");
        return false;
    }
    
    // Calculate distance to destination for priority system
    int distance = abs(nextX - train.destinationX) + abs(nextY - train.destinationY);
    
    // Add to planned moves for collision detection
    if (numPlannedMoves < MAX_TRAINS) {
        plannedMoves[numPlannedMoves].trainIndex = trainIndex;
        plannedMoves[numPlannedMoves].nextX = nextX;
        plannedMoves[numPlannedMoves].nextY = nextY;
        plannedMoves[numPlannedMoves].distance = distance;
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
            if (switches[switchIndex].currentState == 0) {
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
    Train& train = trains[trainIndex];
    
    // Simple approach for easy level: find the direction that gets closest to destination
    int bestDirection = currentDir;
    int bestDistance = 999;
    
    for (int dir = 0; dir < 4; dir++) {
        int nextX = x + dx[dir];
        int nextY = y + dy[dir];
        
        // Check if this direction is valid
        if (isInBounds(nextX, nextY) && isTrackTile(nextX, nextY)) {
            int distance = abs(nextX - train.destinationX) + abs(nextY - train.destinationY);
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
        if (trains[i].state == TRAIN_ACTIVE) {
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
        PlannedMove& move = plannedMoves[moveIndex];
        int i = move.trainIndex;
        if (trains[i].state == TRAIN_ACTIVE) {
            // Check for safety tiles that cause delays
            if (safetyTiles[trains[i].x][trains[i].y]) {
                trains[i].waitTicks++;
                totalWaitTicks++;
                
                // Apply weather effects
                int delayTicks = 1; // Normal delay
                if (weather == WEATHER_RAIN) delayTicks = 2;
                if (weather == WEATHER_FOG) delayTicks = 3;
                
                if (trains[i].waitTicks >= delayTicks) {
                    trains[i].waitTicks = 0; // Reset wait counter
                    
                    // Move the train
                    trains[i].x = move.nextX;
                    trains[i].y = move.nextY;
                    trains[i].direction = getNextDirection(move.nextX, move.nextY, trains[i].direction, i);
                    
                    logTrainTrace(trains[i].id, trains[i].x, trains[i].y, trains[i].direction, "MOVING");
                }
            } else {
                // Move the train normally
                trains[i].x = move.nextX;
                trains[i].y = move.nextY;
                trains[i].direction = getNextDirection(move.nextX, move.nextY, trains[i].direction, i);
                
                logTrainTrace(trains[i].id, trains[i].x, trains[i].y, trains[i].direction, "MOVING");
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
            if (plannedMoves[j].distance < plannedMoves[j + 1].distance) {
                // Swap
                PlannedMove temp = plannedMoves[j];
                plannedMoves[j] = plannedMoves[j + 1];
                plannedMoves[j + 1] = temp;
            }
        }
    }
    
    // Check for collisions
    for (int i = 0; i < numPlannedMoves; i++) {
        for (int j = i + 1; j < numPlannedMoves; j++) {
            // Same destination collision
            if (plannedMoves[i].nextX == plannedMoves[j].nextX && 
                plannedMoves[i].nextY == plannedMoves[j].nextY) {
                
                // Higher distance train (i) gets priority, lower distance train (j) waits
                if (plannedMoves[i].distance > plannedMoves[j].distance) {
                    trains[plannedMoves[j].trainIndex].waitTicks++;
                } else if (plannedMoves[j].distance > plannedMoves[i].distance) {
                    trains[plannedMoves[i].trainIndex].waitTicks++;
                } else {
                    // Equal distance - both crash
                    trains[plannedMoves[i].trainIndex].state = TRAIN_CRASHED;
                    trains[plannedMoves[j].trainIndex].state = TRAIN_CRASHED;
                    trainsCrashed += 2;
                    activeTrains -= 2;
                    
                    logTrainTrace(trains[plannedMoves[i].trainIndex].id, 
                                trains[plannedMoves[i].trainIndex].x, 
                                trains[plannedMoves[i].trainIndex].y, 
                                trains[plannedMoves[i].trainIndex].direction, "CRASHED");
                    logTrainTrace(trains[plannedMoves[j].trainIndex].id, 
                                trains[plannedMoves[j].trainIndex].x, 
                                trains[plannedMoves[j].trainIndex].y, 
                                trains[plannedMoves[j].trainIndex].direction, "CRASHED");
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
        if (trains[i].state == TRAIN_ACTIVE) {
            if (trains[i].x == trains[i].destinationX && trains[i].y == trains[i].destinationY) {
                trains[i].state = TRAIN_DELIVERED;
                trainsDelivered++;
                activeTrains--;
                
                logTrainTrace(trains[i].id, trains[i].x, trains[i].y, trains[i].direction, "DELIVERED");
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
        if (trains[i].state == TRAIN_ACTIVE) {
            int distance = abs(trains[i].x - emergencyHaltX) + abs(trains[i].y - emergencyHaltY);
            if (distance <= emergencyHaltRange) {
                trains[i].waitTicks += 3; // Halt for 3 ticks
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
