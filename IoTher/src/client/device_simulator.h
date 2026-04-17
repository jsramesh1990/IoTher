#ifndef IOTHER_DEVICE_SIMULATOR_H
#define IOTHER_DEVICE_SIMULATOR_H

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <map>

#define IOTHER_VERSION "1.0.0"

// ANSI Color Codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

class Sensor {
public:
    std::string name;
    std::string unit;
    std::string topic;
    double min_value;
    double max_value;
    int publish_interval_ms;
    
    Sensor(std::string n, std::string u, std::string t, double minv, double maxv, int interval = 2000)
        : name(n), unit(u), topic(t), min_value(minv), max_value(maxv), publish_interval_ms(interval) {}
    
    double readValue() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min_value, max_value);
        return dis(gen);
    }
    
    std::string getFormattedValue(double value) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f", value);
        return std::string(buffer) + unit;
    }
};

class IoTherDevice {
private:
    std::string device_id;
    std::string device_type;
    int sock;
    struct sockaddr_in broker_addr;
    std::vector<Sensor> sensors;
    std::vector<std::string> subscriptions;
    std::mutex mtx;
    bool running;
    std::map<std::string, std::string> received_messages;
    time_t start_time;
    int messages_published;
    int messages_received;
    
    bool connectToBroker();
    void sendCommand(const std::string& cmd);
    void receiveLoop();
    void publishSensorData();
    void subscribeToTopics();
    void printDeviceBanner();
    
public:
    IoTherDevice(const std::string& id, const std::string& type = "Generic");
    ~IoTherDevice();
    void addSensor(const Sensor& sensor);
    void subscribe(const std::string& topic);
    void start();
    void stop();
    void printStatus();
    void sendCustomMessage(const std::string& topic, const std::string& message);
};

#endif
