#include "grid.h"
#include "simulation_state.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// ============================================================================
// GRID.CPP - Grid utilities with smooth animation
// ============================================================================

bool isInBounds(int x, int y) {
    return x >= 0 && x < gridRows && y >= 0 && y < gridCols;
}

bool isTrackTile(int x, int y) {
    if (!isInBounds(x, y)) return false;
    
    char tile = grid[x][y];
    return (tile == '-' || tile == '|' || tile == '=' || tile == '\\' || tile == '/' || 
            tile == '+' || tile == 'S' || tile == 'D' ||
            (tile >= 'A' && tile <= 'Z'));
}

bool isSwitchTile(int x, int y) {
    if (!isInBounds(x, y)) return false;
    
    char tile = grid[x][y];
    return (tile >= 'A' && tile <= 'Z' && tile != 'S' && tile != 'D');
}

int getSwitchIndex(char switchChar) {
    if (switchChar >= 'A' && switchChar <= 'Z') {
        return switchChar - 'A';
    }
    return -1;
}

bool isSpawnPoint(int x, int y) {
    if (!isInBounds(x, y)) return false;
    return grid[x][y] == 'S';
}

bool isDestinationPoint(int x, int y) {
    if (!isInBounds(x, y)) return false;
    return grid[x][y] == 'D';
}

bool toggleSafetyTile(int x, int y) {
    if (!isInBounds(x, y)) return false;
    if (!isTrackTile(x, y)) return false;
    safetyTiles[x][y] = !safetyTiles[x][y];
    return true;
}

void printGrid() {
    // Clear screen for smooth animation
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    
    std::cout << "\n=== RAILWAY SIMULATION ===\n";
    std::cout << "Tick: " << currentTick << " | Delivered: " << trainsDelivered 
              << " | Crashed: " << trainsCrashed << "\n\n";
    
    // Print the railway map with trains
    for (int row = 0; row < gridRows; row++) {
        for (int col = 0; col < gridCols; col++) {
            char tile = grid[row][col];
            
            // Check if there's a train here
            bool hasActiveTrain = false;
            for (int i = 0; i < numTrains; i++) {
                if (trains[i][TRAIN_STATE] == TRAIN_ACTIVE && trains[i][TRAIN_X] == row && trains[i][TRAIN_Y] == col) {
                    // Show train with arrows
                    switch (trains[i][TRAIN_DIRECTION]) {
                        case DIR_UP:    std::cout << "^"; break;
                        case DIR_DOWN:  std::cout << "v"; break;
                        case DIR_LEFT:  std::cout << "<"; break;
                        case DIR_RIGHT: std::cout << ">"; break;
                        default:        std::cout << trains[i][TRAIN_ID]; break;
                    }
                    hasActiveTrain = true;
                    break;
                }
            }
            
            // If no train, show the track
            if (!hasActiveTrain) {
                std::cout << tile;
            }
        }
        std::cout << std::endl;
    }
    
    // Show active trains
    std::cout << "\nActive Trains: ";
    bool hasActive = false;
    for (int i = 0; i < numTrains; i++) {
        if (trains[i][TRAIN_STATE] == TRAIN_ACTIVE) {
            if (hasActive) std::cout << ", ";
            std::cout << "T" << trains[i][TRAIN_ID] << "(" << trains[i][TRAIN_X] << "," << trains[i][TRAIN_Y] << ")";
            hasActive = true;
        }
    }
    if (!hasActive) std::cout << "None";
    std::cout << std::endl;
    
    // Small delay for animation
#ifdef _WIN32
    Sleep(800);
#else
    usleep(800000);
#endif
}
