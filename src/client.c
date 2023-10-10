#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "common.h"

void trataComandoCliente(char *bufTerminal, int *estadoCliente, int *estadoPartida,
                         int boardCliente[][BOARDSZ], int size, int sock){
    
    int x, y;
    action mensagemAEnviar;
    action mensagemRecebida;
    
    if (strncmp(bufTerminal, "start", 5) == 0 && *estadoPartida == 0) {
        *estadoPartida = 1;
        mensagemAEnviar.type = 0;

        //envia mensagem de start para o servidor
        size_t count = send(sock, &mensagemAEnviar, sizeof(struct action), 0);
        if (count != sizeof(struct action)){
            logexit("send");
        }

        //recebe board atual do servidor e imprime na tela
        recv(sock, &mensagemRecebida, sizeof(struct action), 0);
        copiaBoard(mensagemRecebida.board, boardCliente, size);
        imprimeBoard(boardCliente, size);
    } 
    
    else if (strncmp(bufTerminal, "reveal ", 7) == 0 && *estadoPartida == 1) {
        sscanf(bufTerminal, "reveal %d,%d", &x, &y);

        //verifica se as coordenadas estao dentro do range do board
        if(x > BOARDSZ - 1 || x < 0 || y > BOARDSZ - 1 || y < 0){
            printf("error: invalid cell\n");
            return;
        }

        //verifica se a celula ja foi revelada e nao tem flag
        if(boardCliente[x][y] != -2 && boardCliente[x][y] != -3){
            printf("error: cell already revealed\n");
            return;
        }

        mensagemAEnviar.type = 1;
        mensagemAEnviar.coordinates[0] = x;
        mensagemAEnviar.coordinates[1] = y;
        
        //envia mensagem de reveal para o servidor
        size_t count = send(sock, &mensagemAEnviar, sizeof(struct action), 0);
        if (count != sizeof(struct action)){
            logexit("send");
        }

        //recebe novo estado do board do servidor
        recv(sock, &mensagemRecebida, sizeof(struct action), 0);

        //se a celula revelada for uma bomba
        if(mensagemRecebida.type == 8){
            printf("GAME OVER!\n");
            imprimeBoard(mensagemRecebida.board, size);
            *estadoPartida = 0;
            return;
        }

        //se ganhou o jogo
        else if(mensagemRecebida.type == 6){
            printf("YOU WIN!\n");
            imprimeBoard(mensagemRecebida.board, size);
            *estadoPartida = 0;
            return;
        }

        //se for apenas um novo estado do jogo
        else if(mensagemRecebida.type == 3){
            //copia novo estado e imprime na tela
            copiaBoard(mensagemRecebida.board, boardCliente, size);
            imprimeBoard(boardCliente, size);
        }        
    
    } 
    
    else if (strncmp(bufTerminal, "flag ", 5) == 0 && *estadoPartida == 1) {
        sscanf(bufTerminal, "flag %d,%d", &x, &y);

        //verifica se as coordenadas estao dentro do range do board
        if(x > BOARDSZ - 1 || x < 0 || y > BOARDSZ - 1 || y < 0){
            printf("error: invalid cell");
            return;
        }

        //verifica se a celula ja foi revelada e nao esta com flag
        if(boardCliente[x][y] != -2 && boardCliente[x][y] != -3){
            printf("error: cannot insert flag in revealed cell\n");
            return;
        }

        //verifica se a celula ja esta marcada
        if(boardCliente[x][y] == -3){
            printf("error: cell already has a flag\n");
            return;
        }

        mensagemAEnviar.type = 2;
        mensagemAEnviar.coordinates[0] = x;
        mensagemAEnviar.coordinates[1] = y;

        //envia mensagem de flag para o servidor
        size_t count = send(sock, &mensagemAEnviar, sizeof(struct action), 0);
        if (count != sizeof(struct action)){
            logexit("send");
        }

        //recebe e imprime novo estado do board do servidor
        recv(sock, &mensagemRecebida, sizeof(struct action), 0);
        copiaBoard(mensagemRecebida.board, boardCliente, size);
        imprimeBoard(boardCliente, size);
    } 
    
    else if (strncmp(bufTerminal, "remove_flag ", 12) == 0 && *estadoPartida == 1) {
        sscanf(bufTerminal, "remove_flag %d,%d", &x, &y);

        //verifica se as coordenadas estao dentro do range do board
        if(x > BOARDSZ - 1 || x < 0 || y > BOARDSZ - 1 || y < 0){
            printf("error: invalid cell\n");
            return;
        }

        //se a celula nao possui flag
        if(boardCliente[x][y] != -3){
            //?????????
            imprimeBoard(boardCliente, size);
            return;
        }

        mensagemAEnviar.type = 4;
        mensagemAEnviar.coordinates[0] = x;
        mensagemAEnviar.coordinates[1] = y;

        //envia mensagem de flag para o servidor
        size_t count = send(sock, &mensagemAEnviar, sizeof(struct action), 0);
        if (count != sizeof(struct action)){
            logexit("send");
        }

        //recebe e imprime novo estado do board do servidor
        recv(sock, &mensagemRecebida, sizeof(struct action), 0);
        copiaBoard(mensagemRecebida.board, boardCliente, size);
        imprimeBoard(boardCliente, size);
    }

    else if (strncmp(bufTerminal, "reset", 5) == 0) {
        *estadoPartida = 1;
        mensagemAEnviar.type = 5;

        //envia mensagem de reset para o servidor
        size_t count = send(sock, &mensagemAEnviar, sizeof(struct action), 0);
        if (count != sizeof(struct action)){
            logexit("send");
        }

        //recebe e imprime novo estado do board do servidor
        recv(sock, &mensagemRecebida, sizeof(struct action), 0);
        copiaBoard(mensagemRecebida.board, boardCliente, size);
        imprimeBoard(boardCliente, size);
    } 

    else if (strncmp(bufTerminal, "exit", 4) == 0) {
        mensagemAEnviar.type = 7;
        *estadoCliente = 0;

        //envia mensagem de exit para o servidor
        size_t count = send(sock, &mensagemAEnviar, sizeof(struct action), 0);
        if (count != sizeof(struct action)){
            logexit("send");
        }

        close(sock);
        return;
    } 
    
    else {
        printf("error: command not found\n");
    }
}

void main(int argc, char *argv[]){
    int boardCliente[BOARDSZ][BOARDSZ];

    // capturando argumentos de entrada
    const char *enderecoIPServidor = argv[1];
    const char *porta = argv[2];

    // converte as entradas em um endereco
    struct sockaddr_storage storage;
    if (addrparse(enderecoIPServidor, porta, &storage) != 0){
        logexit("entradas");
    }

    // abre socket da internet, TCP
    int sock = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sock == -1){
        logexit("socket");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    // estabelece conexao com o servidor
    if (connect(sock, addr, sizeof(storage)) != 0){
        logexit("connect");
    }
    
    char bufTerminal[BUFSZ];
    int estadoCliente = 1;
    int estadoPartida = 0;

    //enquanto jogo estiver rolando
    while(estadoCliente){
        //recebe e trata comando da linha de comando
        fgets(bufTerminal, sizeof(bufTerminal), stdin);
        trataComandoCliente(bufTerminal, &estadoCliente, &estadoPartida,
                            boardCliente, BOARDSZ, sock);
    }
}