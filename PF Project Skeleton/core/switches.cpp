#include "switches.h"
#include "simulation_state.h"
#include "grid.h"
#include "io.h"

// ============================================================================
// SWITCHES.CPP - Switch management
// ============================================================================

// ----------------------------------------------------------------------------
// UPDATE SWITCH COUNTERS
// ----------------------------------------------------------------------------
// Increment counters for trains entering switches.
// ----------------------------------------------------------------------------
void updateSwitchCounters() {
    // Check all trains to see if they entered switches this tick
    for (int i = 0; i < numTrains; i++) {
        if (trains[i].state == TRAIN_ACTIVE) {
            int x = trains[i].x;
            int y = trains[i].y;
            
            if (isSwitchTile(x, y)) {
                int switchIndex = getSwitchIndex(grid[x][y]);
                if (switchIndex >= 0 && switchIndex < numSwitches) {
                    // Increment counter based on switch mode
                    if (switches[switchIndex].mode == PER_DIR) {
                        // Increment counter for the direction the train came FROM
                        int entryDir = trains[i].direction;
                        switches[switchIndex].counters[entryDir]++;
                    } else {
                        // GLOBAL mode - increment global counter
                        switches[switchIndex].globalCounter++;
                    }
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// QUEUE SWITCH FLIPS
// ----------------------------------------------------------------------------
// Queue flips when counters hit K.
// ----------------------------------------------------------------------------
void queueSwitchFlips() {
    for (int i = 0; i < numSwitches; i++) {
        bool shouldFlip = false;
        
        if (switches[i].mode == PER_DIR) {
            // Check if any direction counter reached its K-value
            for (int dir = 0; dir < 4; dir++) {
                if (switches[i].counters[dir] >= switches[i].kValues[dir]) {
                    shouldFlip = true;
                    switches[i].counters[dir] = 0; // Reset counter
                }
            }
        } else {
            // GLOBAL mode - check global counter against first K-value
            if (switches[i].globalCounter >= switches[i].kValues[0]) {
                shouldFlip = true;
                switches[i].globalCounter = 0; // Reset counter
            }
        }
        
        if (shouldFlip) {
            switches[i].flipQueued = true;
        }
    }
}

// ----------------------------------------------------------------------------
// APPLY DEFERRED FLIPS
// ----------------------------------------------------------------------------
// Apply queued flips after movement.
// ----------------------------------------------------------------------------
void applyDeferredFlips() {
    for (int i = 0; i < numSwitches; i++) {
        if (switches[i].flipQueued) {
            // Flip the switch state
            switches[i].currentState = 1 - switches[i].currentState;
            switches[i].flipQueued = false;
            switchFlips++;
            
            // Log the switch flip
            logSwitchState(i);
        }
    }
}

// ----------------------------------------------------------------------------
// UPDATE SIGNAL LIGHTS
// ----------------------------------------------------------------------------
// Update signal colors for switches.
// ----------------------------------------------------------------------------
void updateSignalLights() {
    for (int i = 0; i < numSwitches; i++) {
        int x = switches[i].x;
        int y = switches[i].y;
        
        if (!isInBounds(x, y)) continue;
        
        std::string signalColor = "GREEN"; // Default to green
        
        // Check if there are trains nearby that could cause conflicts
        bool hasConflict = false;
        bool hasWarning = false;
        
        // Check all adjacent tiles for trains
        for (int dir = 0; dir < 4; dir++) {
            int nextX = x + dx[dir];
            int nextY = y + dy[dir];
            
            if (isInBounds(nextX, nextY)) {
                // Check for trains within 2 tiles
                for (int j = 0; j < numTrains; j++) {
                    if (trains[j].state == TRAIN_ACTIVE) {
                        int distance = abs(trains[j].x - nextX) + abs(trains[j].y - nextY);
                        
                        if (distance == 0) {
                            hasConflict = true; // Train directly on next tile
                        } else if (distance <= 1) {
                            hasWarning = true; // Train within warning range
                        }
                    }
                }
            }
        }
        
        // Determine signal color
        if (hasConflict) {
            signalColor = "RED";
        } else if (hasWarning) {
            signalColor = "YELLOW";
        } else {
            signalColor = "GREEN";
        }
        
        // Log signal state
        logSignalState(i, signalColor);
    }
}

// ----------------------------------------------------------------------------
// TOGGLE SWITCH STATE (Manual)
// ----------------------------------------------------------------------------
// Manually toggle a switch state.
// ----------------------------------------------------------------------------
void toggleSwitchState(int switchIndex) {
    if (switchIndex >= 0 && switchIndex < numSwitches) {
        switches[switchIndex].currentState = 1 - switches[switchIndex].currentState;
        switchFlips++;
        logSwitchState(switchIndex);
    }
}

// ----------------------------------------------------------------------------
// GET SWITCH STATE FOR DIRECTION
// ----------------------------------------------------------------------------
// Return the state for a given direction.
// ----------------------------------------------------------------------------
int getSwitchStateForDirection(int switchIndex, int direction) {
    if (switchIndex < 0 || switchIndex >= numSwitches) return 0;
    
    // For now, all directions use the same switch state
    // Could be extended for more complex switch logic
    return switches[switchIndex].currentState;
}
