#include "io.h"
#include "simulation_state.h"
#include "grid.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

// ============================================================================
// IO.CPP - Level I/O and logging
// ============================================================================

// ----------------------------------------------------------------------------
// LOAD LEVEL FILE
// ----------------------------------------------------------------------------
// Load a .lvl file into global state.
// ----------------------------------------------------------------------------
bool loadLevelFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open level file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::string section = "";
    int mapRowIndex = 0;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        if (line.find("NAME:") == 0) {
            section = "NAME";
            continue;
        } else if (line.find("ROWS:") == 0) {
            section = "ROWS";
            continue;
        } else if (line.find("COLS:") == 0) {
            section = "COLS";
            continue;
        } else if (line.find("SEED:") == 0) {
            section = "SEED";
            continue;
        } else if (line.find("WEATHER:") == 0) {
            section = "WEATHER";
            continue;
        } else if (line.find("MAP:") == 0) {
            section = "MAP";
            mapRowIndex = 0;
            continue;
        } else if (line.find("SWITCHES:") == 0) {
            section = "SWITCHES";
            continue;
        } else if (line.find("TRAINS:") == 0) {
            section = "TRAINS";
            continue;
        }
        
        // Process content based on current section
        if (section == "NAME") {
            levelName = line;
        } else if (section == "ROWS") {
            gridRows = std::stoi(line);
        } else if (section == "COLS") {
            gridCols = std::stoi(line);
        } else if (section == "SEED") {
            seed = std::stoi(line);
        } else if (section == "WEATHER") {
            if (line == "NORMAL") weather = WEATHER_NORMAL;
            else if (line == "RAIN") weather = WEATHER_RAIN;
            else if (line == "FOG") weather = WEATHER_FOG;
        } else if (section == "MAP") {
            if (mapRowIndex < gridRows) {
                for (int col = 0; col < std::min((int)line.length(), gridCols); col++) {
                    grid[mapRowIndex][col] = line[col];
                    
                    // Record spawn and destination points
                    if (line[col] == 'S' && numSpawnPoints < 10) {
                        spawnPoints[numSpawnPoints][SPAWN_X] = mapRowIndex;
                        spawnPoints[numSpawnPoints][SPAWN_Y] = col;
                        spawnPoints[numSpawnPoints][SPAWN_ACTIVE] = 1;
                        numSpawnPoints++;
                    } else if (line[col] == 'D' && numDestinationPoints < 10) {
                        destinationPoints[numDestinationPoints][DEST_X] = mapRowIndex;
                        destinationPoints[numDestinationPoints][DEST_Y] = col;
                        destinationPoints[numDestinationPoints][DEST_ACTIVE] = 1;
                        numDestinationPoints++;
                    } else if (line[col] >= 'A' && line[col] <= 'Z' && line[col] != 'S' && line[col] != 'D') {
                        // Record switch positions
                        int switchIndex = line[col] - 'A';
                        switches[switchIndex][SWITCH_X] = mapRowIndex;
                        switches[switchIndex][SWITCH_Y] = col;
                        switches[switchIndex][SWITCH_LETTER] = line[col];
                    }
                }
                mapRowIndex++;
            }
        } else if (section == "SWITCHES") {
            std::istringstream iss(line);
            char letter;
            std::string modeStr;
            int initState, k0, k1, k2, k3;
            std::string state0, state1;
            
            iss >> letter >> modeStr >> initState >> k0 >> k1 >> k2 >> k3 >> state0 >> state1;
            
            int switchIndex = letter - 'A';
            switches[switchIndex][SWITCH_LETTER] = letter;
            switches[switchIndex][SWITCH_MODE] = (modeStr == "PER_DIR") ? PER_DIR : GLOBAL;
            switches[switchIndex][SWITCH_INIT_STATE] = initState;
            switches[switchIndex][SWITCH_CURRENT_STATE] = initState;
            switches[switchIndex][SWITCH_K0] = k0; // UP
            switches[switchIndex][SWITCH_K1] = k1; // RIGHT
            switches[switchIndex][SWITCH_K2] = k2; // DOWN
            switches[switchIndex][SWITCH_K3] = k3; // LEFT
            switchStateNames[switchIndex][0] = state0;
            switchStateNames[switchIndex][1] = state1;
            
            numSwitches = std::max(numSwitches, switchIndex + 1);
        } else if (section == "TRAINS") {
            std::istringstream iss(line);
            int spawnTick, x, y, direction, colorIndex;
            iss >> spawnTick >> x >> y >> direction >> colorIndex;
            
            trains[numTrains][TRAIN_ID] = numTrains;
            trains[numTrains][TRAIN_SPAWN_TICK] = spawnTick;
            trains[numTrains][TRAIN_X] = x;
            trains[numTrains][TRAIN_Y] = y;
            trains[numTrains][TRAIN_DIRECTION] = direction;
            trains[numTrains][TRAIN_COLOR_INDEX] = colorIndex;
            trains[numTrains][TRAIN_STATE] = TRAIN_SCHEDULED;
            trains[numTrains][TRAIN_WAIT_TICKS] = 0;
            
            // Set destination to first available destination point for now
            if (numDestinationPoints > 0) {
                int destIndex = colorIndex % numDestinationPoints;
                trains[numTrains][TRAIN_DEST_X] = destinationPoints[destIndex][DEST_X];
                trains[numTrains][TRAIN_DEST_Y] = destinationPoints[destIndex][DEST_Y];
            }
            
            numTrains++;
        }
    }
    
    file.close();
    return true;
}

// ----------------------------------------------------------------------------
// INITIALIZE LOG FILES
// ----------------------------------------------------------------------------
// Create/clear CSV logs with headers.
// ----------------------------------------------------------------------------
void initializeLogFiles() {
    system("mkdir -p out");
    
    std::ofstream trace("out/trace.csv");
    trace << "Tick,TrainID,X,Y,Direction,State\n";
    trace.close();
    
    std::ofstream switches_log("out/switches.csv");
    switches_log << "Tick,Switch,Mode,State\n";
    switches_log.close();
    
    std::ofstream signals_log("out/signals.csv");
    signals_log << "Tick,Switch,Signal\n";
    signals_log.close();
}

// ----------------------------------------------------------------------------
// LOG TRAIN TRACE
// ----------------------------------------------------------------------------
// Append tick, train id, position, direction, state to trace.csv.
// ----------------------------------------------------------------------------
void logTrainTrace(int trainID, int x, int y, int direction, const std::string& state) {
    std::ofstream trace("out/trace.csv", std::ios::app);
    trace << currentTick << "," << trainID << "," << x << "," << y << "," << direction << "," << state << "\n";
    trace.close();
}

// ----------------------------------------------------------------------------
// LOG SWITCH STATE
// ----------------------------------------------------------------------------
// Append tick, switch id/mode/state to switches.csv.
// ----------------------------------------------------------------------------
void logSwitchState(int switchIndex) {
    std::ofstream switches_log("out/switches.csv", std::ios::app);
    switches_log << currentTick << "," << (char)switches[switchIndex][SWITCH_LETTER] << "," 
                 << (switches[switchIndex][SWITCH_MODE] == PER_DIR ? "PER_DIR" : "GLOBAL") << ","
                 << switchStateNames[switchIndex][switches[switchIndex][SWITCH_CURRENT_STATE]] << "\n";
    switches_log.close();
}

// ----------------------------------------------------------------------------
// LOG SIGNAL STATE
// ----------------------------------------------------------------------------
// Append tick, switch id, signal color to signals.csv.
// ----------------------------------------------------------------------------
void logSignalState(int switchIndex, const std::string& color) {
    std::ofstream signals_log("out/signals.csv", std::ios::app);
    signals_log << currentTick << "," << (char)switches[switchIndex][SWITCH_LETTER] << "," << color << "\n";
    signals_log.close();
}

// ----------------------------------------------------------------------------
// WRITE FINAL METRICS
// ----------------------------------------------------------------------------
// Write summary metrics to metrics.txt.
// ----------------------------------------------------------------------------
void writeMetrics() {
    std::ofstream metrics("out/metrics.txt");
    
    metrics << "=== SIMULATION METRICS ===\n";
    metrics << "Level: " << levelName << "\n";
    metrics << "Total Ticks: " << currentTick << "\n";
    metrics << "Trains Delivered: " << trainsDelivered << "/" << numTrains << "\n";
    metrics << "Trains Crashed: " << trainsCrashed << "\n";
    metrics << "Switch Flips: " << switchFlips << "\n";
    metrics << "Total Wait Ticks: " << totalWaitTicks << "\n";
    
    double efficiency = (numTrains > 0) ? (double)trainsDelivered / numTrains * 100.0 : 0.0;
    metrics << "Delivery Efficiency: " << efficiency << "%\n";
    
    double avgWait = (trainsDelivered > 0) ? (double)totalWaitTicks / trainsDelivered : 0.0;
    metrics << "Average Wait Time: " << avgWait << " ticks\n";
    
    metrics.close();
}
