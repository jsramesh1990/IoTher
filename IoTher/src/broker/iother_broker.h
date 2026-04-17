#ifndef IOTHER_BROKER_H
#define IOTHER_BROKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define IOTHER_VERSION "1.0.0"
#define IOTHER_NAME "IoTher"
#define IOTHER_TAGLINE "Where devices breathe and data flows"

#define MAX_CLIENTS 100
#define MAX_TOPIC_LEN 100
#define MAX_MSG_LEN 256
#define BUFFER_SIZE 1024
#define PORT 1883

// Color codes for terminal output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

typedef struct {
    int socket;
    char client_id[50];
    char subscribed_topics[MAX_CLIENTS][MAX_TOPIC_LEN];
    int num_subscriptions;
    int active;
    time_t connect_time;
    int messages_sent;
    int messages_received;
} Client;

typedef struct {
    char topic[MAX_TOPIC_LEN];
    char message[MAX_MSG_LEN];
    char publisher[50];
    time_t timestamp;
} Message;

// Global structures
extern Client clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;
extern int server_socket;
extern int total_messages_routed;

// Function prototypes
void print_banner();
void* handle_client(void* arg);
void publish_message(const char* topic, const char* message, const char* publisher);
void subscribe_client(int client_socket, const char* topic);
void unsubscribe_client(int client_socket, const char* topic);
void broadcast_to_subscribers(const char* topic, const char* message, const char* publisher);
void remove_client(int client_socket);
void print_broker_status();
void save_statistics();

#endif
