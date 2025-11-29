#ifndef GRID_H
#define GRID_H

// ============================================================================
// GRID.H - Grid manipulation functions
// ============================================================================
// Functions for working with the 2D grid map.
// ============================================================================

// Check if a position is within grid bounds
bool isInBounds(int x, int y);

// Check if a tile is a track (can trains move on it?)
bool isTrackTile(int x, int y);
bool isValidMove(int x, int y, int dir);

// Check if a tile is a switch (A-Z)
bool isSwitchTile(int x, int y);

// Get the switch index (0-25) from a switch character (A-Z)
int getSwitchIndex(char switchChar);

// Check if a position is a spawn point
bool isSpawnPoint(int x, int y);

// Check if a position is a destination point
bool isDestinationPoint(int x, int y);

// Place or remove a safety tile at a position (for mouse editing)
// Returns true if successful
bool toggleSafetyTile(int x, int y);

// Print the grid state to terminal
void printGrid();

#endif
