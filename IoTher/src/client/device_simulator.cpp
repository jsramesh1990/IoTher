#include "device_simulator.h"
#include <sstream>
#include <iomanip>

IoTherDevice::IoTherDevice(const std::string& id, const std::string& type) 
    : device_id(id), device_type(type), running(false), 
      start_time(0), messages_published(0), messages_received(0) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Socket creation failed");
    }
}

IoTherDevice::~IoTherDevice() {
    stop();
    close(sock);
}

void IoTherDevice::printDeviceBanner() {
    std::cout << COLOR_CYAN COLOR_BOLD;
    std::cout << "\n  ┌─────────────────────────────────────┐\n";
    std::cout << "  │       IoTher Device initialized     │\n";
    std::cout << "  └─────────────────────────────────────┘\n";
    std::cout << COLOR_RESET;
    std::cout << COLOR_GREEN "  📱 Device: " << device_id << COLOR_RESET << "\n";
    std::cout << COLOR_GREEN "  🏷️  Type: " << device_type << COLOR_RESET << "\n";
}

bool IoTherDevice::connectToBroker() {
    broker_addr.sin_family = AF_INET;
    broker_addr.sin_port = htons(1883);
    inet_pton(AF_INET, "127.0.0.1", &broker_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&broker_addr, sizeof(broker_addr)) < 0) {
        std::cerr << COLOR_RED "[" << device_id << "] Failed to connect to IoTher broker" COLOR_RESET << std::endl;
        return false;
    }
    
    char buffer[1024];
    recv(sock, buffer, sizeof(buffer), 0);
    std::cout << COLOR_GREEN "[" << device_id << "] " << buffer << COLOR_RESET << std::endl;
    
    start_time = time(NULL);
    return true;
}

void IoTherDevice::sendCommand(const std::string& cmd) {
    send(sock, cmd.c_str(), cmd.length(), 0);
}

void IoTherDevice::receiveLoop() {
    char buffer[1024];
    
    while (running) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            
            if (strncmp(buffer, "MSG:", 4) == 0) {
                char topic[100], publisher[50], message[256];
                sscanf(buffer + 4, "%[^:]:%[^:]:%s", topic, publisher, message);
                
                std::lock_guard<std::mutex> lock(mtx);
                messages_received++;
                received_messages[topic] = message;
                
                time_t now = time(NULL);
                struct tm *tm_info = localtime(&now);
                char time_buf[20];
                strftime(time_buf, 20, "%H:%M:%S", tm_info);
                
                std::cout << COLOR_YELLOW "\n[" << time_buf << "] 📥 RECEIVED" COLOR_RESET;
                std::cout << " From: " << publisher;
                std::cout << " | Topic: " << topic;
                std::cout << " | Data: " << message << std::endl;
            }
            else if (strncmp(buffer, "PONG", 4) == 0) {
                std::cout << COLOR_CYAN "[HEARTBEAT] Broker is alive" COLOR_RESET << std::endl;
            }
            else if (strncmp(buffer, "STATUS:", 7) == 0) {
                std::cout << COLOR_BLUE "[STATUS] " << (buffer + 7) << COLOR_RESET << std::endl;
            }
        }
    }
}

void IoTherDevice::publishSensorData() {
    while (running) {
        for (auto& sensor : sensors) {
            double value = sensor.readValue();
            std::string formatted_value = sensor.getFormattedValue(value);
            std::string message = sensor.name + ":" + formatted_value;
            
            std::string command = "PUB:" + sensor.topic + ":" + message;
            sendCommand(command);
            
            messages_published++;
            
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char time_buf[20];
            strftime(time_buf, 20, "%H:%M:%S", tm_info);
            
            std::cout << COLOR_GREEN "[" << time_buf << "] 📤 PUBLISHED" COLOR_RESET;
            std::cout << " Topic: " << sensor.topic;
            std::cout << " | Value: " << formatted_value << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(sensor.publish_interval_ms));
        }
    }
}

void IoTherDevice::subscribeToTopics() {
    for (const auto& topic : subscriptions) {
        std::string command = "SUB:" + topic;
        sendCommand(command);
        std::cout << COLOR_CYAN "[" << device_id << "] Subscribed to: " << topic << COLOR_RESET << std::endl;
    }
}

void IoTherDevice::addSensor(const Sensor& sensor) {
    sensors.push_back(sensor);
}

void IoTherDevice::subscribe(const std::string& topic) {
    subscriptions.push_back(topic);
}

void IoTherDevice::start() {
    printDeviceBanner();
    
    if (!connectToBroker()) {
        return;
    }
    
    running = true;
    
    // Subscribe to topics
    subscribeToTopics();
    
    // Start receive thread
    std::thread receive_thread(&IoTherDevice::receiveLoop, this);
    
    // Start publishing thread
    std::thread publish_thread(&IoTherDevice::publishSensorData, this);
    
    receive_thread.detach();
    publish_thread.detach();
    
    std::cout << COLOR_GREEN "\n✓ " << device_id << " is now breathing life into data!\n" COLOR_RESET;
    std::cout << COLOR_YELLOW "  Press Enter to stop this device\n\n" COLOR_RESET;
}

void IoTherDevice::stop() {
    running = false;
    sendCommand("DISCONNECT");
    
    time_t uptime = time(NULL) - start_time;
    std::cout << COLOR_MAGENTA "\n📊 " << device_id << " Statistics:\n";
    std::cout << "   Uptime: " << uptime << " seconds\n";
    std::cout << "   Messages Published: " << messages_published << "\n";
    std::cout << "   Messages Received: " << messages_received << "\n" COLOR_RESET;
}

void IoTherDevice::printStatus() {
    std::lock_guard<std::mutex> lock(mtx);
    time_t uptime = time(NULL) - start_time;
    
    std::cout << COLOR_CYAN "\n╔════════════════════════════════════════╗\n";
    std::cout << "║        IoTher Device Status           ║\n";
    std::cout << "╚════════════════════════════════════════╝\n" COLOR_RESET;
    std::cout << COLOR_GREEN "Device ID: " << device_id << "\n";
    std::cout << "Device Type: " << device_type << "\n";
    std::cout << "Uptime: " << uptime << " seconds\n";
    std::cout << "Sensors: " << sensors.size() << " active\n";
    std::cout << "Published: " << messages_published << " messages\n";
    std::cout << "Received: " << messages_received << " messages\n" COLOR_RESET;
    
    if (!received_messages.empty()) {
        std::cout << COLOR_YELLOW "\nRecent Data:\n" COLOR_RESET;
        for (const auto& [topic, msg] : received_messages) {
            std::cout << "  • " << topic << ": " << msg << std::endl;
        }
    }
}

void IoTherDevice::sendCustomMessage(const std::string& topic, const std::string& message) {
    std::string command = "PUB:" + topic + ":" + message;
    sendCommand(command);
    std::cout << COLOR_GREEN "[CUSTOM] Published to " << topic << ": " << message << COLOR_RESET << std::endl;
}
