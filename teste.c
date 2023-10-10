#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOARDSZ 4

void leituraGabaritoArquivo(char *nomeArquivoResposta, int board[][BOARDSZ], int size){
    int valor, count;
    FILE *arquivo = fopen(nomeArquivoResposta, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo");
        return;
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            count = fscanf(arquivo, "%d,", &valor);
            printf("\n%d\n", count);
            if (count != 1) {
                printf("Erro ao ler o valor na posição (%d, %d)", i, j); 
                fclose(arquivo);
                return;
            }
            board[i][j] = valor;
        }
    }

    fclose(arquivo);
}

char intToChar(int valor) {
    char caractere;
    
    if (valor == -1) {
        caractere = '*';
    } else if (valor == -2) {
        caractere = '-';
    } else if (valor == -3) {
        caractere = '>';
    } else {
        // Se o valor for um número, convertemos para caractere
        caractere = (char)('0' + valor);
    }
    
    return caractere;
}

void main(){
    int answerBoard[BOARDSZ][BOARDSZ];
    leituraGabaritoArquivo("resposta.txt", answerBoard, BOARDSZ);

    
    for (int i = 0; i < BOARDSZ; i++) {
        for (int j = 0; j < BOARDSZ; j++) {
            printf("%d\t\t", answerBoard[i][j]);
        }
        printf("\n");
    }
}