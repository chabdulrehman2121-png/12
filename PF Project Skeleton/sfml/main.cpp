#include "app.h"
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/io.h"
#include <iostream>
#include <unistd.h>

// ============================================================================
// MAIN.CPP - Entry point of the application (NO CLASSES)
// ============================================================================

// ----------------------------------------------------------------------------
// MAIN ENTRY POINT
// ----------------------------------------------------------------------------
// This function is the main entry point of the application. It handles command
// line arguments to specify the level file to load, loads the level file using
// loadLevelFile, initializes the simulation system, initializes the SFML
// application window, prints control instructions to the console, runs the
// main application loop, cleans up resources, and prints final simulation
// statistics. Returns 0 on success, 1 on error (e.g., failed to load level
// file or initialize application).
// ----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    std::cout << "=== SWITCHBACK RAILS SIMULATION ===" << std::endl;
    
    // Check command line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <level_file.lvl>" << std::endl;
        std::cerr << "Example: " << argv[0] << " data/levels/easy_level.lvl" << std::endl;
        return 1;
    }
    
    std::string levelFile = argv[1];
    
    // Initialize simulation system
    initializeSimulation();
    
    // Load the level file
    std::cout << "\nLoading level file: " << levelFile << std::endl;
    if (!loadLevelFile(levelFile)) {
        std::cerr << "Error: Failed to load level file: " << levelFile << std::endl;
        return 1;
    }
    
    std::cout << "\nLevel loaded successfully!" << std::endl;
    std::cout << "Starting simulation..." << std::endl;
    std::cout << "Press Ctrl+C to stop the simulation.\n" << std::endl;
    
    // Run the simulation loop
    while (!isSimulationComplete()) {
        simulateOneTick();
        
        // Add a small delay to make output readable
        // In a real implementation, this could be controlled by SFML or user input
        usleep(100000); // 0.1 second delay
    }
    
    std::cout << "\n=== SIMULATION ENDED ===" << std::endl;
    std::cout << "Check the out/ directory for detailed logs and metrics." << std::endl;
    
    return 0;
}
