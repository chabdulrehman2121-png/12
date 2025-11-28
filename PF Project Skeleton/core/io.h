#ifndef IO_H
#define IO_H

#include <string>

// ============================================================================
// IO.H - Level I/O and logging
// ============================================================================

// ----------------------------------------------------------------------------
// LEVEL LOADING
// ----------------------------------------------------------------------------
// Load a .lvl file.
bool loadLevelFile(const std::string& filename);

// ----------------------------------------------------------------------------
// LOGGING
// ----------------------------------------------------------------------------
// Create/clear log files.
void initializeLogFiles();

// Append train movement to trace.csv.
void logTrainTrace(int trainID, int x, int y, int direction, const std::string& state);

// Append switch state to switches.csv.
void logSwitchState(int switchIndex);

// Append signal state to signals.csv.
void logSignalState(int switchIndex, const std::string& color);

// Write final metrics to metrics.txt.
void writeMetrics();

#endif
