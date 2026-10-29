#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "funcionarios.h"

TFunc* busca_binaria(int cod, FILE *arq, int tam) {

    if (arq == NULL || tam <= 0) {
        return NULL;
    }
    int left = 0, right = tam-1;
    while(left <= right){
        int middle = (left+right)/2;
        fseek(arq, middle*tamanho_registro(), SEEK_SET);
        TFunc* func = le_funcionario(arq);
        if (func == NULL) {
            return NULL;
        }
        if(cod == func->cod) {
           // printf("Encontrado na posicao %d \n", middle);
            return func;
        }
        else if(func->cod < cod) {
            left = middle+1;
        }
        else {
            right = middle-1;
        }
        free(func);
    }
    return NULL;
}


void insertion_sort_disco(FILE *arq, int tam) {
    if (arq == NULL || tam <= 1) {
        return;
    }

    for (int j = 1; j < tam; j++) {
        // Lê o registro que será inserido na posição correta
        if (fseek(arq,
                  (long)j * tamanho_registro(),
                  SEEK_SET) != 0) {
            return;
        }

        TFunc *fj = le_funcionario(arq);

        if (fj == NULL) {
            return;
        }

        int i = j - 1;

        while (i >= 0) {
            // Lê o registro anterior
            if (fseek(arq,
                      (long)i * tamanho_registro(),
                      SEEK_SET) != 0) {
                free(fj);
                return;
            }

            TFunc *fi = le_funcionario(arq);

            if (fi == NULL) {
                free(fj);
                return;
            }

            // O registro anterior já está na posição correta
            if (fi->cod <= fj->cod) {
                free(fi);
                break;
            }

            // Desloca o registro anterior uma posição à direita
            if (fseek(arq,
                      (long)(i + 1) * tamanho_registro(),
                      SEEK_SET) != 0) {
                free(fi);
                free(fj);
                return;
            }

            salva_funcionario(fi, arq);
            free(fi);

            i--;
        }

        // Insere o registro atual na posição correta
        if (fseek(arq,
                  (long)(i + 1) * tamanho_registro(),
                  SEEK_SET) != 0) {
            free(fj);
            return;
        }

        salva_funcionario(fj, arq);
        free(fj);
    }

    fflush(arq);
}



