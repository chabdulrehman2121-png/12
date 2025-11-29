#include "simulation.h"
#include "simulation_state.h"
#include "trains.h"
#include "switches.h"
#include "grid.h"
#include "io.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

// ============================================================================
// SIMULATION.CPP - Implementation of main simulation logic
// ============================================================================

// ----------------------------------------------------------------------------
// INITIALIZE SIMULATION
// ----------------------------------------------------------------------------

void initializeSimulation() {
    // Reset simulation state
    initializeSimulationState();
    
    // Initialize logging
    initializeLogFiles();
    
    std::cout << "Simulation initialized successfully!" << std::endl;
}

// ----------------------------------------------------------------------------
// SIMULATE ONE TICK
// ----------------------------------------------------------------------------

void simulateOneTick() {
    // Phase 1: Spawn trains scheduled for this tick
    spawnTrainsForTick();
    
    // Phase 2: Determine routes for all active trains
    determineAllRoutes();
    
    // Phase 3: Update switch counters based on train entries
    updateSwitchCounters();
    
    // Phase 4: Queue switch flips when counters reach K-values
    queueSwitchFlips();
    
    // Phase 5: Move trains and handle collisions
    moveAllTrains();
    
    // Phase 6: Apply deferred switch flips
    applyDeferredFlips();
    
    // Phase 7: Check for arrivals at destination points
    checkArrivals();
    
    // Apply emergency halt effects if active
    applyEmergencyHalt();
    updateEmergencyHalt();
    
    // Update signal lights for visualization
    updateSignalLights();
    
    // Print current grid state to terminal
    printGrid();
    
    // Increment tick counter
    currentTick++;
}

// ----------------------------------------------------------------------------
// CHECK IF SIMULATION IS COMPLETE
// ----------------------------------------------------------------------------

bool isSimulationComplete() {
    // Check if there are any trains that haven't been processed yet (scheduled trains still remaining)
    int scheduledTrains = 0;
    int actualActive = 0;
    int delivered = 0;
    int crashed = 0;
    
    for (int i = 0; i < numTrains; i++) {
        switch (trains[i][TRAIN_STATE]) {
            case TRAIN_SCHEDULED: scheduledTrains++; break;
            case TRAIN_ACTIVE: actualActive++; break;
            case TRAIN_DELIVERED: delivered++; break;
            case TRAIN_CRASHED: crashed++; break;
        }
    }
    
    // Update global counters
    activeTrains = actualActive;
    trainsDelivered = delivered;
    trainsCrashed = crashed;
    
    // If we have active or scheduled trains, keep running
    if (actualActive > 0 || scheduledTrains > 0) {
        return false;
    }
    
    // All trains are processed (delivered or crashed)
    std::cout << "\n=== SIMULATION COMPLETE ===" << std::endl;
    std::cout << "All trains have been processed!" << std::endl;
    std::cout << "Final state:" << std::endl;
    std::cout << "  Delivered: " << delivered << std::endl;
    std::cout << "  Crashed: " << crashed << std::endl;
    std::cout << "  Total ticks: " << currentTick << std::endl;
    writeMetrics();
    return true;
}
