#include "device_simulator.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>

void printGlobalBanner() {
    std::cout << COLOR_CYAN COLOR_BOLD;
    std::cout << "\n";
    std::cout << "  ██╗ ██████╗ ████████╗██╗  ██╗███████╗██████╗ \n";
    std::cout << "  ██║██╔═══██╗╚══██╔══╝██║  ██║██╔════╝██╔══██╗\n";
    std::cout << "  ██║██║   ██║   ██║   ███████║█████╗  ██████╔╝\n";
    std::cout << "  ██║██║   ██║   ██║   ██╔══██║██╔══╝  ██╔══██╗\n";
    std::cout << "  ██║╚██████╔╝   ██║   ██║  ██║███████╗██║  ██║\n";
    std::cout << "  ╚═╝ ╚═════╝    ╚═╝   ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝\n";
    std::cout << COLOR_RESET;
    std::cout << COLOR_YELLOW "\n     Virtual Sensor Simulation Platform\n";
    std::cout << "     \"Where devices breathe and data flows\"\n" COLOR_RESET;
    std::cout << COLOR_GREEN "     Version " IOTHER_VERSION "\n\n" COLOR_RESET;
}

int main() {
    printGlobalBanner();
    
    std::cout << COLOR_YELLOW "Initializing IoTher Network...\n\n" COLOR_RESET;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Create device 1: Environmental Sensor Node
    IoTherDevice device1("EnvSensor_01", "Environmental Monitor");
    device1.addSensor(Sensor("Temperature", "°C", "sensors/temp", 18.0, 35.0, 3000));
    device1.addSensor(Sensor("Humidity", "%", "sensors/humidity", 30.0, 80.0, 4000));
    device1.addSensor(Sensor("Pressure", "hPa", "sensors/pressure", 980.0, 1020.0, 5000));
    device1.subscribe("commands/env/#");
    device1.subscribe("alerts/environment");
    device1.start();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Create device 2: HVAC Controller
    IoTherDevice device2("HVAC_Controller", "Climate Control");
    device2.addSensor(Sensor("Setpoint", "°C", "hvac/setpoint", 20.0, 25.0, 6000));
    device2.addSensor(Sensor("FanSpeed", "%", "hvac/fan_speed", 0.0, 100.0, 8000));
    device2.subscribe("sensors/temp");
    device2.subscribe("sensors/humidity");
    device2.subscribe("commands/hvac/#");
    device2.start();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Create device 3: Smart Lighting
    IoTherDevice device3("Lighting_System", "Smart Lights");
    device3.addSensor(Sensor("Luminosity", "lux", "sensors/light", 100.0, 1000.0, 3500));
    device3.addSensor(Sensor("PowerConsumption", "W", "power/lighting", 10.0, 150.0, 7000));
    device3.subscribe("#");  // Subscribe to all topics
    device3.start();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Create device 4: Security Sensor
    IoTherDevice device4("Security_Hub", "Motion Detection");
    device4.addSensor(Sensor("Motion", "detected", "security/motion", 0.0, 1.0, 2000));
    device4.addSensor(Sensor("DoorStatus", "state", "security/door", 0.0, 1.0, 5000));
    device4.subscribe("alerts/#");
    device4.subscribe("commands/security/#");
    device4.start();
    
    std::cout << COLOR_GREEN "\n╔════════════════════════════════════════════════════╗\n";
    std::cout << "║     IoTher Network is now LIVE!                  ║\n";
    std::cout << "║     " << std::setw(40) << std::right << "║\n";
    std::cout << "║     4 devices are breathing and flowing data    ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n" COLOR_RESET;
    
    std::cout << COLOR_CYAN "\nCommands:\n";
    std::cout << "  • Press 's' + Enter to show device status\n";
    std::cout << "  • Press 'q' + Enter to quit simulation\n" COLOR_RESET;
    
    // Interactive console
    char input;
    while (true) {
        std::cin >> input;
        if (input == 's') {
            std::cout << COLOR_CYAN "\n=== IoTher Network Status ===\n" COLOR_RESET;
            device1.printStatus();
            device2.printStatus();
            device3.printStatus();
            device4.printStatus();
        }
        else if (input == 'q') {
            break;
        }
    }
    
    std::cout << COLOR_YELLOW "\nShutting down IoTher devices...\n" COLOR_RESET;
    device1.stop();
    device2.stop();
    device3.stop();
    device4.stop();
    
    std::cout << COLOR_GREEN "\n✓ All devices have stopped breathing.\n";
    std::cout << "✓ Data flow has ended.\n";
    std::cout << "✓ Thank you for using IoTher!\n" COLOR_RESET;
    
    return 0;
}
