#ifndef INTERCALACAO_H
#define INTERCALACAO_H

#include "lista.h"
#include "pilha.h"


/*estrutura para armazenar o vetor de partições*/
typedef struct vetor{
    TFunc *func;
    int aux_p;
    FILE *f;
}TVet;

/*Algoritmo de intercalacao de particoes básico, recebe
o nome do arquivo de saída, o número de partições a serem intercaladas
e uma lista com os nomes das partições de entrada a serem lidas.*/
void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes);

/*----------------------funcoes para arvore de vencedores----------------------*/

/* Algoritmo de intercalacao de particoes usando arvore de vencedores*/
void intercalacao_arvore_de_vencedores(TPilha **pilha, int *vetTop, char *nome_arquivo_saida, int num_p);

/* Retorna a chave (codigo) do elemento no topo da pilha da particao p.
 Retorna INT_MAX (HIGH VALUE) se a pilha estiver vazia ou se p >= num_p (no de preenchimento). */
int chave_topo(TPilha **pilha, int *vetTop, int num_p, int p);
 
/* Disputa entre as particoes a e b: retorna o indice da que tem a menor chave.
 Em caso de empate retorna a (esquerda), mantendo a intercalacao estavel. */
int vencedor_entre(TPilha **pilha, int *vetTop, int num_p, int a, int b);

#endif
