#include "intercalacao.h"
#include "funcionarios.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {

    int fim = 0; //variavel que controla fim do procedimento
    FILE *out; //declara ponteiro para arquivo

    //abre arquivo de saida para escrita
    if ((out = fopen(nome_arquivo_saida, "wb")) == NULL) {
        printf("Erro ao abrir arquivo de sa?da\n");
    } else {
        //cria vetor de particoes
        TVet v[num_p];

        //abre arquivos das particoes, colocando variavel de arquivo no campo f do vetor
        //e primeiro funcionario do arquivo no campo func do vetor
        for (int i=0; i < num_p; i++) {
            v[i].f = fopen(nome_particoes->nome, "rb");
            v[i].aux_p = 0;
            if (v[i].f != NULL) {
                fseek(v[i].f, v[i].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[i].f);
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posi??o do vetor
                    v[i].func = funcionario(INT_MAX, "","","",0);
                }
                else {
                    //conseguiu ler funcionario, coloca na posi??o atual do vetor
                    v[i].func = f;
                }
            }
            else {
                fim = 1;
            }
            nome_particoes = nome_particoes->prox;
        }

        int aux = 0;
        while (!(fim)) { //conseguiu abrir todos os arquivos
            int menor = INT_MAX;
            int pos_menor;
            //encontra o funcionario com menor chave no vetor
            for(int i = 0; i < num_p; i++){
                if(v[i].func->cod < menor){
                    menor = v[i].func->cod;
                    pos_menor = i;
                }
            }
            if (menor == INT_MAX) {
                fim = 1; //terminou processamento
            }
            else {
                //salva funcionario no arquivo de saída
                fseek(out, aux * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[pos_menor].func, out);
                //atualiza posição pos_menor do vetor com pr?ximo funcionario do arquivo
                v[pos_menor].aux_p++;
                fseek(v[pos_menor].f, v[pos_menor].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[pos_menor].f);
                aux++;
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posiçao do vetor
                    v[pos_menor].func = funcionario(INT_MAX, "", "", "",0.0);
                }
                else {
                    v[pos_menor].func = f;
                }

            }
        }

        //fecha arquivos das partiÇões de entrada
        for(int i = 0; i < num_p; i++){
            fclose(v[i].f);
        //    free(v[i].func);
        }
        //fecha arquivo de saída
        fclose(out);
    }
}

void intercalacao_arvore_de_vencedores(TPilha **pilha, int *vetTop, char *nome_arquivo_saida, int num_p){
    if (num_p <= 0) {
        printf("Numero de particoes invalido\n");
        return;
    }
 
    FILE *out;
    if ((out = fopen(nome_arquivo_saida, "wb")) == NULL) {
        printf("Erro ao abrir arquivo de saida\n");
        return;
    }
 
    /* A arvore e um vetor (heap): no 1 = raiz, filhos de i = 2i e 2i+1.
     As folhas ficam nas posicoes [tam, 2*tam-1], com tam = menor potencia de 2 >= num_p.
     Cada no guarda o INDICE DA PARTICAO que venceu a disputa naquela subarvore.
     Folhas excedentes (>= num_p) sao nos de preenchimento e nunca vencem. */
    int tam = 1;
    while (tam < num_p)
        tam *= 2;
 
    int *arv = (int *) malloc(sizeof(int) * 2 * tam);
    if (arv == NULL) {
        printf("Erro de alocacao da arvore de vencedores\n");
        fclose(out);
        return;
    }
 
    /* 1) folhas: cada folha representa uma particao */
    for (int i = 0; i < tam; i++)
        arv[tam + i] = i;
 
    /* 2) constroi os nos internos de baixo para cima (torneio inicial) */
    for (int i = tam - 1; i >= 1; i--)
        arv[i] = vencedor_entre(pilha, vetTop, num_p, arv[2 * i], arv[2 * i + 1]);
 
    /* 3) repete: remove o vencedor da raiz, grava na saida e refaz so o caminho ate a raiz */
    while (1) {
        int p = arv[1]; //particao dona do menor elemento atual
        if (chave_topo(pilha, vetTop, num_p, p) == INT_MAX)
            break; //raiz e HIGH VALUE: todas as particoes acabaram
 
        TFunc *f = pop(pilha[p], 0, &vetTop[p]); //retira o menor da pilha da particao
        salva_funcionario(f, out);
        free(f);
 
        /* o novo topo da particao p (ou HIGH VALUE) disputa de novo,
         subindo da folha de p ate a raiz: O(log num_p) comparacoes por registro */
        for (int i = (tam + p) / 2; i >= 1; i /= 2)
            arv[i] = vencedor_entre(pilha, vetTop, num_p, arv[2 * i], arv[2 * i + 1]);
    }
 
    free(arv);
    fclose(out);
}

