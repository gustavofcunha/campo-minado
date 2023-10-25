#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "common.h"

void inicializaBoardNovoJogo(int board[][BOARDSZ], int size){
    int i, j;
    
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            board[i][j] = -2;
        }
    }
}


void leituraGabaritoArquivo(char *nomeArquivoResposta, int board[][BOARDSZ], int size){
    int valor, i, j;
    int count = 0;
    FILE *arquivo = fopen(nomeArquivoResposta, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo");
        return;
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            fscanf(arquivo, "%d,", &valor);
            board[i][j] = valor;
        }
    }

    fclose(arquivo);
}

void trataMensagem(action *mensagemRecebida, int clientSock, int answerBoard[][BOARDSZ], 
                   int currentBoard[][BOARDSZ], int size, char *nomeArquivoResposta, 
                   int *estadoConexao, int numeroCelulasSemBomba, 
                   int *numeroCelulasReveladas){
    
    int x, y;
    
    action mensagemAEnviar;

    //se a mensagem for um start
    if(mensagemRecebida->type == 0){
        //inicializa estado do jogo
        inicializaBoardNovoJogo(currentBoard, size);
        *numeroCelulasReveladas = 0;

        mensagemAEnviar.type = 3;

        //envia tabuleiro atual para o cliente (todo ocultado)
        copiaBoard(currentBoard, mensagemAEnviar.board, size);
        size_t count = send(clientSock, &mensagemAEnviar, sizeof(action), 0);
        if(count != sizeof(action)){
            logexit("send");
        }
    }

    //se a mensagem for um reveal
    else if(mensagemRecebida->type == 1){

        x = mensagemRecebida->coordinates[0];
        y = mensagemRecebida->coordinates[1];

        //se a celula for uma bomba
        if(answerBoard[x][y] == -1){
            mensagemAEnviar.type = 8;
            copiaBoard(answerBoard, mensagemAEnviar.board, size);
        }

        //se a celula nao for uma bomba 
        else{
            (*numeroCelulasReveladas)++;

            //vitoria
            if(*numeroCelulasReveladas == numeroCelulasSemBomba){
                mensagemAEnviar.type = 6;

                //revela a ultima celula e manda para o cliente
                currentBoard[x][y] = answerBoard[x][y];
                copiaBoard(answerBoard, mensagemAEnviar.board, size);
            }

            //revela a celula, sem vitoria
            else{
                mensagemAEnviar.type = 3;
                //altera estado atual do jogo
                currentBoard[x][y] = answerBoard[x][y];
                copiaBoard(currentBoard, mensagemAEnviar.board, size);
            }
            
        }

        //envia a mensagem
        size_t count = send(clientSock, &mensagemAEnviar, sizeof(action), 0);
        if(count != sizeof(action)){
            logexit("send");
        }
    }

    //se a mensagem for um flag
    else if(mensagemRecebida->type == 2){
        
        x = mensagemRecebida->coordinates[0];
        y = mensagemRecebida->coordinates[1];

        mensagemAEnviar.type = 3;
        currentBoard[x][y] = -3;
        copiaBoard(currentBoard, mensagemAEnviar.board, size);

        size_t count = send(clientSock, &mensagemAEnviar, sizeof(action), 0);
        if(count != sizeof(action)){
            logexit("send");
        }
    }

    //se a mensagem for um remove_flag
    else if(mensagemRecebida->type == 4){
        
        x = mensagemRecebida->coordinates[0];
        y = mensagemRecebida->coordinates[1];

        mensagemAEnviar.type = 3;
        currentBoard[x][y] = -2;
        copiaBoard(currentBoard, mensagemAEnviar.board, size);

        size_t count = send(clientSock, &mensagemAEnviar, sizeof(action), 0);
        if(count != sizeof(action)){
            logexit("send");
        }
    }

    //se a mensagem for um reset
    else if(mensagemRecebida->type == 5){
        printf("starting new game\n");

        //reinicializa estado do jogo
        inicializaBoardNovoJogo(currentBoard, size);
        *numeroCelulasReveladas = 0;
        
        mensagemAEnviar.type = 3;

        //envia tabuleiro atual para o cliente (todo ocultado)
        copiaBoard(currentBoard, mensagemAEnviar.board, size);
        size_t count = send(clientSock, &mensagemAEnviar, sizeof(action), 0);
        if(count != sizeof(action)){
            logexit("send");
        }
    }

    //se a mensagem for um exit
    else if(mensagemRecebida->type == 7){
        *estadoConexao = 0;
        close(clientSock);
        printf("\nclient disconnected\n");
        //reseta estado do jogo
        inicializaBoardNovoJogo(currentBoard, size);
    }

    else{
        *estadoConexao = 0;
        close(clientSock);
        printf("error: command not found");
    }

}

int contaCelulasSemBomba(int board[][BOARDSZ], int size){
    int i, j;
    int count = 0;

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            if(board[i][j] != -1){
                count++;
            }
        }
    }

    return count;
}

void main(int argc, char *argv[]){
    //variaveis auxiliares
    int i, j;
    int numeroCelulasReveladas = 0;
    int numeroCelulasSemBomba = 0;

    //capturando argumentos de entrada
    char *tipoEndereco = argv[1];
    char *porta = (argv[2]);
    char *nomeArquivoResposta = argv[4];

    //quadros (representacao em inteiros)
    int answerBoard[BOARDSZ][BOARDSZ];
    int currentBoard[BOARDSZ][BOARDSZ];

    //le e imprime gabarito do jogo em arquivo
    leituraGabaritoArquivo(nomeArquivoResposta, answerBoard, BOARDSZ);
    numeroCelulasSemBomba = contaCelulasSemBomba(answerBoard, BOARDSZ);
    imprimeBoard(answerBoard, BOARDSZ);

    //converte as entradas em um endereco
    struct sockaddr_storage storage;
    if (server_sockaddr_init(tipoEndereco, porta, &storage) != 0){
        logexit("entradas");
    }
    
    //abre socket da internet, TCP
    int sock = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sock == -1){
        logexit("socket");
    }

    //seta socket para reutilizar portas
    int enable = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0){
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (bind(sock, addr, sizeof(storage)) != 0){
        logexit("bind");
    }

    if (listen(sock, 10) != 0){
        logexit("listen");
    }

    action mensagem;
    int estadoServidor = 1;
    int estadoConexao = 1;
     
    //enquanto servidor estiver ativo
    while(estadoServidor){
        struct sockaddr_storage clientStorage;
        struct sockaddr *clientAddr = (struct sockaddr *)(&clientStorage);

        socklen_t clientAddrLen = sizeof(clientStorage);
        int clientSock = accept(sock, (struct sockaddr *)&clientStorage, &clientAddrLen);
        if(clientSock == -1){
            logexit("accept");
        }

        estadoConexao = 1;

        printf("\nclient connected\n");
        
        while(estadoConexao){
            //recebe mensagem do cliente
            recv(clientSock, &mensagem, sizeof(struct action), 0);
            //trata mensagem recebida do cliente
            trataMensagem(&mensagem, clientSock, answerBoard, currentBoard, BOARDSZ, 
                          nomeArquivoResposta, &estadoConexao, numeroCelulasSemBomba,
                          &numeroCelulasReveladas); 
        }
        
    }
}