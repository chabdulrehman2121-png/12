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
        if (trains[i][TRAIN_STATE] == TRAIN_ACTIVE) {
            int x = trains[i][TRAIN_X];
            int y = trains[i][TRAIN_Y];
            
            if (isSwitchTile(x, y)) {
                int switchIndex = getSwitchIndex(grid[x][y]);
                if (switchIndex >= 0 && switchIndex < numSwitches) {
                    // Increment counter based on switch mode
                    if (switches[switchIndex][SWITCH_MODE] == PER_DIR) {
                        // Increment counter for the direction the train came FROM
                        int entryDir = trains[i][TRAIN_DIRECTION];
                        switches[switchIndex][SWITCH_COUNTER0 + entryDir]++;
                    } else {
                        // GLOBAL mode - increment global counter
                        switches[switchIndex][SWITCH_GLOBAL_COUNTER]++;
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
        
        if (switches[i][SWITCH_MODE] == PER_DIR) {
            // Check if any direction counter reached its K-value
            for (int dir = 0; dir < 4; dir++) {
                if (switches[i][SWITCH_COUNTER0 + dir] >= switches[i][SWITCH_K0 + dir]) {
                    shouldFlip = true;
                    switches[i][SWITCH_COUNTER0 + dir] = 0; // Reset counter
                }
            }
        } else {
            // GLOBAL mode - check global counter against first K-value
            if (switches[i][SWITCH_GLOBAL_COUNTER] >= switches[i][SWITCH_K0]) {
                shouldFlip = true;
                switches[i][SWITCH_GLOBAL_COUNTER] = 0; // Reset counter
            }
        }
        
        if (shouldFlip) {
            switches[i][SWITCH_FLIP_QUEUED] = 1;
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
        if (switches[i][SWITCH_FLIP_QUEUED]) {
            // Flip the switch state
            switches[i][SWITCH_CURRENT_STATE] = 1 - switches[i][SWITCH_CURRENT_STATE];
            switches[i][SWITCH_FLIP_QUEUED] = 0;
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
        int x = switches[i][SWITCH_X];
        int y = switches[i][SWITCH_Y];
        
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
                    if (trains[j][TRAIN_STATE] == TRAIN_ACTIVE) {
                        int distance = abs(trains[j][TRAIN_X] - nextX) + abs(trains[j][TRAIN_Y] - nextY);
                        
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
        switches[switchIndex][SWITCH_CURRENT_STATE] = 1 - switches[switchIndex][SWITCH_CURRENT_STATE];
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
    return switches[switchIndex][SWITCH_CURRENT_STATE];
}
