#ifndef COMMON_H
#define COMMON_H

#define BUFSZ 20
#define BOARDSZ 4

typedef struct action {
    int type;
    int coordinates[2];
    int board[4][4];
}action;

void logexit(char *msg);
int addrparse(const char *addrstr, const char *portaStr, struct sockaddr_storage *storage);
void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);
int server_sockaddr_init(const char *tipoEndereco, char * portaStr, 
                          struct sockaddr_storage *storage);
void serialize_action(const struct action *src, char *dest);
void deserialize_action(const char *src, struct action *dest);
char intToChar(int valor);
void imprimeBoard(int board[][BOARDSZ], int size);
void copiaBoard(int origem[][BOARDSZ], int destino[][BOARDSZ], int size);

#endif