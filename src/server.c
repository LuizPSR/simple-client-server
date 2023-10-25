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
#define BOMB_NUM 3

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
 * Defining the server behavior *
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

void hide_board(int board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = UNREVEALED;
    }
  }
}

void reveal_board(int board[BOARD_SIZE][BOARD_SIZE], int sol[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = sol[i][j];
    }
  }
}

int is_bomb(int coords[2], int solution[BOARD_SIZE][BOARD_SIZE]) {
  return solution[coords[0]][coords[1]] == BOMB;
}

void read_solution(char* path, int solution[BOARD_SIZE][BOARD_SIZE]) {
  
  // read the file
  FILE *file;
  file = fopen(path, "r");
  if (file == NULL) {
      perror("failed to open the file");
      exit(EXIT_FAILURE);
  }
  
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      fscanf(file, "%d,", &solution[i][j]);
    }
  }
  fclose(file);
  
  print_board(solution);
}

/******************************
 ************ IPv4 ************
 ******************************/
int create_server_v4(int port){
  int server;
  struct sockaddr_in server_addr;

  if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("error: failure to create server socket");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("error: failure to bind server socket");
    exit(EXIT_FAILURE);
  }

  if (listen(server, 1) < 0) {
    perror("error: failure to listen to client");
    exit(EXIT_FAILURE);
  }

  return server;
}
int connect_client_v4(int server){
  int client;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  if ((client = accept(server, (struct sockaddr *) &client_addr, &client_len)) < 0) {
    perror("error: failure to accept client");
    exit(EXIT_FAILURE);
  }

  return client;
}

/******************************
 ************ IPv6 ************
 ******************************/
int create_server_v6(int port){
  int server;
  struct sockaddr_in6 server_addr;

  if ((server = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
    perror("error: failure to create server socket");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_addr = in6addr_any;
  server_addr.sin6_port = htons(port);

  if (bind(server, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("error: failure to bind server socket");
    exit(EXIT_FAILURE);
  }

  if (listen(server, 1) < 0) {
    perror("error: failure to listen to client");
    exit(EXIT_FAILURE);
  }

  return server;
}
int connect_client_v6(int server){
  int client;
  struct sockaddr_in6 client_addr;
  socklen_t client_len = sizeof(client_addr);

  if ((client = accept(server, (struct sockaddr *) &client_addr, &client_len)) < 0) {
    perror("error: failure to accept client");
    exit(EXIT_FAILURE);
  }

  return client;
}

/****************************
 *********** MAIN ***********
 ****************************/

int main(int argc, char** argv) {
  int solution[BOARD_SIZE][BOARD_SIZE];
  read_solution(argv[4], solution);
  
  int server_socket, client_socket;
  if(strcmp(argv[1], "v4") == 0){
    server_socket = create_server_v4(atoi(argv[2]));
    client_socket = connect_client_v6(server_socket);
    
  }else if(strcmp(argv[1], "v6") == 0){
    server_socket = create_server_v6(atoi(argv[2]));
    client_socket = connect_client_v6(server_socket);
    
  } else {
    perror("error: invalid IP version");
    exit(EXIT_FAILURE);
  }
  
  printf("client connected\n");
  // message exchanging loop
  char command_buffer[1024];
  int running = 1;
  int remaining;
  struct action client_action, server_response;
  while (running) {
    // wait command
    ssize_t bytes_received = recv(client_socket, &client_action, sizeof(client_action), 0);
    if (bytes_received <= 0) {
      perror("error receiving data");
      break;
    }
    
    // handle command
    switch (client_action.type) {
      case RESET:
        printf("starting new game\n");
      case START:
        hide_board(server_response.board);
        remaining = BOARD_SIZE * BOARD_SIZE;
        server_response.type = STATE;
        break;
        
      case REVEAL:
        // if reveling a bomb
        if (is_bomb(client_action.coordinates, solution)) {
          reveal_board(server_response.board, solution);
          server_response.type = GAME_OVER;
        } else {
          remaining--;
          // if no bombs left
          if (remaining <= BOMB_NUM) {
            reveal_board(server_response.board, solution);
            server_response.type = WIN;
          } else {
            server_response.board[client_action.coordinates[0]][client_action.coordinates[1]] = 
              solution[client_action.coordinates[0]][client_action.coordinates[1]];
            server_response.type = STATE;
          }
        }
        break;
        
      case FLAG:
        server_response.board[client_action.coordinates[0]][client_action.coordinates[1]] = 
              FLAGED;
        server_response.type = STATE;
        break;
        
      case REMOVE_FLAG:
        server_response.board[client_action.coordinates[0]][client_action.coordinates[1]] = 
              UNREVEALED;
        server_response.type = STATE;
        break;
        
      case EXIT:
        running = 0;
      default:
        // we must retorn a error, since the client is waiting
        server_response.type = -1;
    }
    
    // send response
    send(client_socket, &server_response, sizeof(server_response), 0);
  }

  // close sockets
  close(client_socket);
  close(server_socket);
  
  printf("client disconnected\n");
  
  return 0;
}

