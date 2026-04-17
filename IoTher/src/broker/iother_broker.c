#include "iother_broker.h"

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_socket;
int total_messages_routed = 0;
FILE *log_file = NULL;

void print_banner() {
    printf(COLOR_CYAN COLOR_BOLD);
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════╗\n");
    printf("  ║                                                          ║\n");
    printf("  ║     ██╗ ██████╗ ████████╗██╗  ██╗███████╗██████╗         ║\n");
    printf("  ║     ██║██╔═══██╗╚══██╔══╝██║  ██║██╔════╝██╔══██╗        ║\n");
    printf("  ║     ██║██║   ██║   ██║   ███████║█████╗  ██████╔╝        ║\n");
    printf("  ║     ██║██║   ██║   ██║   ██╔══██║██╔══╝  ██╔══██╗        ║\n");
    printf("  ║     ██║╚██████╔╝   ██║   ██║  ██║███████╗██║  ██║        ║\n");
    printf("  ║     ╚═╝ ╚═════╝    ╚═╝   ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝        ║\n");
    printf("  ║                                                          ║\n");
    printf("  ║         " IOTHER_NAME " - Virtual Sensor Simulation Platform        ║\n");
    printf("  ║         " IOTHER_TAGLINE "                ║\n");
    printf("  ║         Version: " IOTHER_VERSION "                                  ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");
    printf(COLOR_RESET);
    printf("\n");
}

void print_broker_status() {
    static time_t last_print = 0;
    time_t now = time(NULL);
    
    if (now - last_print >= 2) {
        system("clear");
        print_banner();
        
        printf(COLOR_YELLOW "\n═══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf(COLOR_BOLD "📊 BROKER STATUS\n" COLOR_RESET);
        printf(COLOR_YELLOW "═══════════════════════════════════════════════════════════\n" COLOR_RESET);
        
        int active_count = 0;
        pthread_mutex_lock(&clients_mutex);
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) active_count++;
        }
        
        printf(COLOR_GREEN "✓ Port: %d\n" COLOR_RESET, PORT);
        printf(COLOR_GREEN "✓ Active Clients: %d\n" COLOR_RESET, active_count);
        printf(COLOR_GREEN "✓ Messages Routed: %d\n" COLOR_RESET, total_messages_routed);
        printf(COLOR_GREEN "✓ Uptime: %ld seconds\n" COLOR_RESET, now - last_print);
        
        printf(COLOR_YELLOW "\n═══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf(COLOR_BOLD "🖥️  CONNECTED DEVICES\n" COLOR_RESET);
        printf(COLOR_YELLOW "═══════════════════════════════════════════════════════════\n" COLOR_RESET);
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) {
                printf(COLOR_CYAN "\n📱 Device: %s\n" COLOR_RESET, clients[i].client_id);
                printf("   Socket: %d | Connected: %lds ago\n", 
                       clients[i].socket, now - clients[i].connect_time);
                printf("   Messages: Sent=%d | Received=%d\n",
                       clients[i].messages_sent, clients[i].messages_received);
                printf("   📡 Subscriptions: ");
                
                if (clients[i].num_subscriptions == 0) {
                    printf("(none)");
                } else {
                    for (int j = 0; j < clients[i].num_subscriptions; j++) {
                        printf("%s ", clients[i].subscribed_topics[j]);
                    }
                }
                printf("\n");
            }
        }
        
        pthread_mutex_unlock(&clients_mutex);
        
        printf(COLOR_YELLOW "\n═══════════════════════════════════════════════════════════\n" COLOR_RESET);
        printf(COLOR_BOLD "💡 TIP: Devices are breathing and data is flowing!\n" COLOR_RESET);
        printf(COLOR_YELLOW "═══════════════════════════════════════════════════════════\n\n" COLOR_RESET);
        
        last_print = now;
    }
}

void save_statistics() {
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "[%ld] Total messages routed: %d\n", now, total_messages_routed);
        fflush(log_file);
    }
}

void publish_message(const char* topic, const char* message, const char* publisher) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[20];
    strftime(time_buf, 20, "%H:%M:%S", tm_info);
    
    printf(COLOR_MAGENTA "[%s] 📤 PUBLISH" COLOR_RESET, time_buf);
    printf(" %s -> %s: %s\n", publisher, topic, message);
    
    total_messages_routed++;
    
    // Update publisher stats
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].client_id, publisher) == 0) {
            clients[i].messages_sent++;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    broadcast_to_subscribers(topic, message, publisher);
    
    if (log_file) {
        fprintf(log_file, "[%s] PUBLISH %s %s %s\n", time_buf, publisher, topic, message);
        fflush(log_file);
    }
}

void subscribe_client(int client_socket, const char* topic) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == client_socket && clients[i].active) {
            // Check if already subscribed
            int already_subscribed = 0;
            for (int j = 0; j < clients[i].num_subscriptions; j++) {
                if (strcmp(clients[i].subscribed_topics[j], topic) == 0) {
                    already_subscribed = 1;
                    break;
                }
            }
            
            if (!already_subscribed && clients[i].num_subscriptions < MAX_CLIENTS) {
                strcpy(clients[i].subscribed_topics[clients[i].num_subscriptions], topic);
                clients[i].num_subscriptions++;
                printf(COLOR_GREEN "[SUBSCRIBE] %s subscribed to: %s\n" COLOR_RESET, 
                       clients[i].client_id, topic);
            }
            break;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

void unsubscribe_client(int client_socket, const char* topic) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == client_socket && clients[i].active) {
            for (int j = 0; j < clients[i].num_subscriptions; j++) {
                if (strcmp(clients[i].subscribed_topics[j], topic) == 0) {
                    for (int k = j; k < clients[i].num_subscriptions - 1; k++) {
                        strcpy(clients[i].subscribed_topics[k], 
                               clients[i].subscribed_topics[k + 1]);
                    }
                    clients[i].num_subscriptions--;
                    printf(COLOR_YELLOW "[UNSUBSCRIBE] %s unsubscribed from: %s\n" COLOR_RESET, 
                           clients[i].client_id, topic);
                    break;
                }
            }
            break;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

void broadcast_to_subscribers(const char* topic, const char* message, const char* publisher) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "MSG:%s:%s:%s", topic, publisher, message);
    
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            for (int j = 0; j < clients[i].num_subscriptions; j++) {
                // Support wildcard #
                if (strcmp(clients[i].subscribed_topics[j], topic) == 0 ||
                    strcmp(clients[i].subscribed_topics[j], "#") == 0) {
                    
                    send(clients[i].socket, buffer, strlen(buffer), 0);
                    clients[i].messages_received++;
                    
                    printf(COLOR_BLUE "[BROADCAST] To %s: %s\n" COLOR_RESET, 
                           clients[i].client_id, buffer);
                    break;
                }
            }
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == client_socket) {
            printf(COLOR_RED "[DISCONNECT] %s left the network\n" COLOR_RESET, 
                   clients[i].client_id);
            clients[i].active = 0;
            close(clients[i].socket);
            break;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    // Find free client slot
    int client_index = -1;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            client_index = i;
            clients[i].socket = client_socket;
            clients[i].active = 1;
            clients[i].num_subscriptions = 0;
            clients[i].connect_time = time(NULL);
            clients[i].messages_sent = 0;
            clients[i].messages_received = 0;
            snprintf(clients[i].client_id, sizeof(clients[i].client_id), 
                    "Device_%d", client_socket);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    if (client_index == -1) {
        printf(COLOR_RED "[ERROR] Max clients reached, rejecting connection\n" COLOR_RESET);
        close(client_socket);
        return NULL;
    }
    
    printf(COLOR_GREEN "[CONNECT] %s joined IoTher network\n" COLOR_RESET, 
           clients[client_index].client_id);
    
    // Send welcome message
    char welcome[BUFFER_SIZE];
    snprintf(welcome, sizeof(welcome), 
             "CONNECTED:Welcome to IoTher - Where devices breathe and data flows");
    send(client_socket, welcome, strlen(welcome), 0);
    
    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        
        if (strncmp(buffer, "PUB:", 4) == 0) {
            char topic[MAX_TOPIC_LEN], message[MAX_MSG_LEN];
            sscanf(buffer + 4, "%[^:]:%s", topic, message);
            publish_message(topic, message, clients[client_index].client_id);
        }
        else if (strncmp(buffer, "SUB:", 4) == 0) {
            char topic[MAX_TOPIC_LEN];
            sscanf(buffer + 4, "%s", topic);
            subscribe_client(client_socket, topic);
        }
        else if (strncmp(buffer, "UNSUB:", 6) == 0) {
            char topic[MAX_TOPIC_LEN];
            sscanf(buffer + 6, "%s", topic);
            unsubscribe_client(client_socket, topic);
        }
        else if (strncmp(buffer, "PING", 4) == 0) {
            send(client_socket, "PONG", 4, 0);
        }
        else if (strncmp(buffer, "STATUS", 6) == 0) {
            char status_msg[256];
            snprintf(status_msg, sizeof(status_msg), 
                     "STATUS:Device %s active for %ld seconds",
                     clients[client_index].client_id,
                     time(NULL) - clients[client_index].connect_time);
            send(client_socket, status_msg, strlen(status_msg), 0);
        }
        else if (strncmp(buffer, "DISCONNECT", 10) == 0) {
            break;
        }
        
        print_broker_status();
    }
    
    remove_client(client_socket);
    save_statistics();
    free(arg);
    return NULL;
}

void signal_handler(int sig) {
    printf(COLOR_YELLOW "\n\n[SHUTDOWN] IoTher broker is going to sleep...\n" COLOR_RESET);
    if (log_file) {
        fprintf(log_file, "=== Broker shutdown at %ld ===\n", time(NULL));
        fclose(log_file);
    }
    close(server_socket);
    exit(0);
}

int main() {
    struct sockaddr_in server_addr;
    pthread_t thread_id;
    
    signal(SIGINT, signal_handler);
    
    // Open log file
    log_file = fopen("iother.log", "a");
    if (log_file) {
        fprintf(log_file, "=== IoTher Broker started at %ld ===\n", time(NULL));
    }
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(1);
    }
    
    system("clear");
    print_banner();
    printf(COLOR_GREEN "\n✓ IoTher Broker is now LIVE!\n" COLOR_RESET);
    printf(COLOR_CYAN "  Listening on port: %d\n", PORT);
    printf("  Max clients: %d\n", MAX_CLIENTS);
    printf("  Log file: iother.log\n" COLOR_RESET);
    printf(COLOR_YELLOW "\n💡 Press Ctrl+C to stop the broker\n\n" COLOR_RESET);
    
    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        
        int* new_sock = malloc(sizeof(int));
        *new_sock = client_socket;
        pthread_create(&thread_id, NULL, handle_client, (void*)new_sock);
        pthread_detach(thread_id);
    }
    
    close(server_socket);
    return 0;
}
