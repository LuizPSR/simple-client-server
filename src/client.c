#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/******************************************
 * Defining the constants of comunication *
 ******************************************/

// Sizes
#define BUFFER_SIZE sizeof(struct action)
#define BOARD_SIZE 4

// Action types
#define START 0
#define REVEAL 1
#define FLAG 2
#define STATE 3
#define REMOVE_FLAG 4
#define RESET 5
#define WIN 6
#define EXIT 7
#define GAME_OVER 8

// Board states
#define UNREVEALED -2
#define FLAGED -3
#define BOMB -1

struct action {
    int type;
    int coordinates[2];
    int board[BOARD_SIZE][BOARD_SIZE];
};

/********************************
 * Defining the client behavior *
 ********************************/

void print_board(const int board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      switch (board[i][j]) {
        case UNREVEALED:
          printf("-\t\t");
          break;
        case FLAGED:
          printf(">\t\t");
          break;
        case BOMB:
          printf("*\t\t");
          break;
        default:
          printf("%d\t\t", board[i][j]);
      }
    }
    printf("\n");
  }
  printf("\n");
}

int read_command(char* command) {
  if (strcmp(command, "start") == 0) 
    return START;
  
  if (strcmp(command, "reveal") == 0) 
    return REVEAL;
    
  if (strcmp(command, "flag") == 0) 
    return FLAG;
    
  if (strcmp(command, "remove_flag") == 0)
    return REMOVE_FLAG;
    
  if (strcmp(command, "reset") == 0) 
    return RESET;
    
  if (strcmp(command, "exit") == 0) 
    return EXIT;

  return EXIT_FAILURE;
}

int out_of_bounds(int coords[2]) {
  return coords[0] < 0 || coords[0] >= BOARD_SIZE || coords[1] < 0 || coords[1] >= BOARD_SIZE;
}

int is_revealed(int coords[2], int board[BOARD_SIZE][BOARD_SIZE]) {
  return board[coords[0]][coords[1]] != UNREVEALED;
}

int is_flaged(int coords[2], int board[BOARD_SIZE][BOARD_SIZE]) {
  return board[coords[0]][coords[1]] == FLAGED;
}

/******************************
 ************ IPv4 ************
 ******************************/

int create_and_connect_v4(char* ip, int port) {
  int client;
  struct sockaddr_in server_addr;

  inet_pton(AF_INET, ip, &server_addr.sin_addr);

  if ((client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("error: failure to create socket");
    exit(EXIT_FAILURE);
  }
  
  memset(&server_addr, '0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
    perror("error: invalid address");
    exit(EXIT_FAILURE);
  }

  if (connect(client, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("error: failure to connect to server");
    exit(EXIT_FAILURE);
  }

  return client;
}

/******************************
 ************ IPv6 ************
 ******************************/

int create_and_connect_v6(char* ip, int port) {
  int client;
  struct sockaddr_in6 server_addr;

  inet_pton(AF_INET6, ip, &server_addr.sin6_addr);

  if ((client = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
    perror("error: failure to create socket");
    exit(EXIT_FAILURE);
  }
  
  memset(&server_addr, '0', sizeof(server_addr));
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_port = htons(port);

  if (inet_pton(AF_INET6, ip, &server_addr.sin6_addr) <= 0) {
    perror("error: invalid address");
    exit(EXIT_FAILURE);
  }

  if (connect(client, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("error: failure to connect to server");
    exit(EXIT_FAILURE);
  }

  return client;
}

/****************************
 *********** MAIN ***********
 ****************************/

int main(int argc, char** argv) {

  int client_socket;
  if (strchr(argv[1], '.') != NULL) {
    client_socket = create_and_connect_v4(argv[1], atoi(argv[2]));
    
  } else if (strchr(argv[1], ':') != NULL) {
    client_socket = create_and_connect_v6(argv[1], atoi(argv[2]));
    
  } else {
    perror("error: invalid IP address");
    exit(EXIT_FAILURE);
  }
  
  // message exchanging loop
  char command_buffer[1024];
  int running = 1;
  struct action client_action, server_response;
  while (running) {
    scanf("%s", &command_buffer);
    client_action.type = read_command(command_buffer);
    
    // prepare message
    switch (client_action.type) {
      case START:
        break;
        
      case REVEAL:
        scanf("%d", &client_action.coordinates[0]);
        scanf("%d", &client_action.coordinates[1]);
        
        if (out_of_bounds(client_action.coordinates)) {
          printf("error: invalid cell\n");
          continue;
        }
        // you can reveal only unrevealed or flaged cells
        if (is_revealed(client_action.coordinates, server_response.board) &&
            !is_flaged(client_action.coordinates, server_response.board)) {
          printf("error: cell already revealed\n");
          continue;
        }
        break;
    
      case FLAG:
        scanf("%d", &client_action.coordinates[0]);
        scanf("%d", &client_action.coordinates[1]);
        
        if (out_of_bounds(client_action.coordinates)) {
          printf("error: invalid cell\n");
          continue;
        }
        if (is_flaged(client_action.coordinates, server_response.board)) {
          printf("error: cell already has a flag\n");
          continue;
        }
        if (is_revealed(client_action.coordinates, server_response.board)) {
          printf("error: cannot insert flag in revealed cell\n");
          continue;
        }
        break;
        
      case REMOVE_FLAG:
        scanf("%d", &client_action.coordinates[0]);
        scanf("%d", &client_action.coordinates[1]);
        
        if (out_of_bounds(client_action.coordinates)) {
          printf("error: invalid cell\n");
          continue;
        }
        if (!is_flaged(client_action.coordinates, server_response.board)) {
          printf("error: cell has no flag to remove\n");
          continue;
        }
        break;
        

      case EXIT:
        running = 0;
      case RESET:
        break;
        
      default:
        printf("error: command not found\n");
        continue;
    }

    // send message
    send(client_socket, &client_action, sizeof(client_action), 0);
    if (!running)
      break;

    // receive response
    ssize_t bytes_received = recv(client_socket, &server_response, sizeof(server_response), 0);
    if (bytes_received <= 0) {
      perror("error receiving data");
      break;
    }

    // handle response
    switch (server_response.type) {
      case STATE:
        break;
        
      case WIN:
        printf("YOU WIN!\n");
        break;
        
      case GAME_OVER:
        printf("GAME OVER!\n");
        break;
        
      default:
        printf("error: unexpected server's response\n");
    }
    print_board(server_response.board);
  }

  // Close the socket
  close(client_socket);

  return 0;
}

