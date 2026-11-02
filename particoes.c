#include "funcionarios.h"
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include "particoes.h"

void classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc) {
    rewind(arq); //posiciona cursor no inicio do arquivo
    int reg = 0;
 
    while (reg != nFunc) {
        //le o arquivo e coloca no vetor
        TFunc *v[M];
        int i = 0;
        while (!feof(arq)) {
            fseek(arq, (reg) * tamanho_registro(), SEEK_SET);
            v[i] = le_funcionario(arq);
       //     imprime_funcionario(v[i]);
            i++;
            reg++;
            if(i>=M) break;
        }
 
        //ajusta tamanho M caso arquivo de entrada tenha terminado antes do vetor
        if (i != M) {
            M = i;
        }
 
        //faz ordenacao
        for (int j = 1; j < M; j++) {
            TFunc *f = v[j];
            i = j - 1;
            while ((i >= 0) && (v[i]->cod > f->cod)) {
                v[i + 1] = v[i];
                i = i - 1;
            }
            v[i + 1] = f;
        }
 
        //cria arquivo de particao e faz gravacao
        char *nome_particao = nome_arquivos_saida->nome;
        nome_arquivos_saida = nome_arquivos_saida->prox;
        printf("\n%s\n", nome_particao);
        FILE *p;
        if ((p = fopen(nome_particao, "wb+")) == NULL) {
            printf("Erro criar arquivo de saida\n");
        } else {
            for (int i = 0; i < M; i++) {
                fseek(p, (i) * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[i], p);
                imprime_funcionario(v[i]);
            }
            fclose(p);
        }
        for(int jj = 0; jj<M; jj++)
            free(v[jj]);
    }
}
//Selecao natural 
int selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n, int *tam_particoes) { //retorna int e recebe tam_particoes
    rewind(arq); // Posiciona no início do arquivo de entrada
    
    if (n > M) n = M; // o reservatório volta inteiro para a RAM, então não pode ser maior que M

    // Aloca memória para a RAM (tamanho M) e Reservatório (tamanho n)
    TFunc **v = (TFunc **) malloc(sizeof(TFunc *) * M);
    TFunc **reservatorio = (TFunc **) malloc(sizeof(TFunc *) * n);
    bool *congelado = (bool *) malloc(sizeof(bool) * M);

    int reg_lidos = 0;
    int num_reservatorio = 0;
    int elementos_na_memoria = 0;
    int gravados = 0;   //quantos registros já foram gravados na partição atual
    int n_part = 0; //quantas partições já foram concluídas


    // 1. Preenche a memória RAM inicial com M registros
    while (elementos_na_memoria < M && reg_lidos < nFunc) {
        fseek(arq, reg_lidos * tamanho_registro(), SEEK_SET);
        v[elementos_na_memoria] = le_funcionario(arq);
        congelado[elementos_na_memoria] = false;
        reg_lidos++;
        elementos_na_memoria++;
    }

    Lista *atual = nome_arquivos_saida;
    FILE *p = NULL;
    char *nome_particao = NULL;

    // Abre o primeiro arquivo de partição se houver
    if (atual != NULL) {
        nome_particao = atual->nome;
        atual = atual->prox;
        p = fopen(nome_particao, "wb+");
        printf("\nCriando particao: %s\n", nome_particao);
    }

    int ultimo_cod_gravado = -1;

    // Laço principal de geração de partições
    while (elementos_na_memoria > 0) {
        // Encontra o menor registro na RAM que NÃO esteja congelado
        int menor_idx = -1;
        int menor_cod = INT_MAX;

        for (int j = 0; j < elementos_na_memoria; j++) {
            if (!congelado[j] && v[j]->cod < menor_cod) {
                menor_cod = v[j]->cod;
                menor_idx = j;
            }
        }

        // Se todos na RAM estão congelados, fecha a partição atual e inicia uma nova
        if (menor_idx == -1) {
            if (p != NULL) fclose(p);
            //guarda o tamanho da partição que acabou de fechar
            if (gravados > 0) tam_particoes[n_part++] = gravados;
            gravados = 0;

            // Descongela todos para a nova partição
            for (int j = 0; j < elementos_na_memoria; j++) {
                congelado[j] = false;
            }
            ultimo_cod_gravado = -1;

            if (atual != NULL) {
                nome_particao = atual->nome;
                atual = atual->prox;
                p = fopen(nome_particao, "wb+");
                printf("\nCriando particao: %s\n", nome_particao);
            } else {
                p = NULL;
                printf("\nErro: faltam nomes de arquivos para as particoes\n"); // ALTERACAO 6

            }
            continue;
        }

        // Grava o menor registro na partição ativa
        if (p != NULL) {
            fseek(p, gravados * tamanho_registro(), SEEK_SET); //posiciona antes de gravar (como a classificacao_interna)
            salva_funcionario(v[menor_idx], p);
            imprime_funcionario(v[menor_idx]);
            gravados++;

        }
        ultimo_cod_gravado = v[menor_idx]->cod;
        free(v[menor_idx]);

        // Lê o próximo registro do arquivo de entrada principal
        TFunc *novo_func = NULL;
        if (reg_lidos < nFunc) {
            fseek(arq, reg_lidos * tamanho_registro(), SEEK_SET);
            novo_func = le_funcionario(arq);
            reg_lidos++;
        }

        if (novo_func != NULL) {
            if (novo_func->cod < ultimo_cod_gravado) {
                // Menor que o último gravado: vai para o reservatório (se houver espaço)
                if (num_reservatorio < n) {
                    reservatorio[num_reservatorio++] = novo_func;
                    // a posição menor_idx ficou vazia (o registro antigo foi liberado
                    // com free e o novo foi para o reservatório). Remove a posição, igual ao caso
                    // de fim de arquivo: o último elemento ocupa o buraco e a RAM diminui.
                    v[menor_idx] = v[elementos_na_memoria - 1];
                    congelado[menor_idx] = congelado[elementos_na_memoria - 1];
                    elementos_na_memoria--;
                } else {
                    // Reservatório cheio: armazena na própria posição e congela
                    v[menor_idx] = novo_func;
                    congelado[menor_idx] = true;
                }
            } else {
                // Maior ou igual: entra na RAM normalmente
                v[menor_idx] = novo_func;
                congelado[menor_idx] = false;
            }
        } else {
            // Arquivo de entrada acabou: remove a posição movendo o último elemento para o buraco
            v[menor_idx] = v[elementos_na_memoria - 1];
            congelado[menor_idx] = congelado[elementos_na_memoria - 1];
            elementos_na_memoria--;
        }

        // Se a RAM esvaziou mas temos elementos no reservatório, descarrega o reservatório para a RAM
        if (elementos_na_memoria == 0 && num_reservatorio > 0) {
            for (int j = 0; j < num_reservatorio; j++) {
                v[j] = reservatorio[j];
                congelado[j] = false;
            }
            elementos_na_memoria = num_reservatorio;
            num_reservatorio = 0;

            if (p != NULL) fclose(p);
            //guarda o tamanho da partição que acabou de fechar
            if (gravados > 0) tam_particoes[n_part++] = gravados;
            gravados = 0;
            if (atual != NULL) {
                nome_particao = atual->nome;
                atual = atual->prox;
                p = fopen(nome_particao, "wb+");
                printf("\nCriando particao (via reservatorio): %s\n", nome_particao);
            } else {
                p = NULL;
                printf("\nErro: faltam nomes de arquivos para as particoes\n");

            }
        }
    }

    if (p != NULL) {
        fclose(p);
    }
//guarda o tamanho da última partição
    if (gravados > 0) tam_particoes[n_part++] = gravados;

    // Garante que qualquer arquivo de partição restante na lista seja criado vazio (evita erro de arquivo não encontrado)
    while (atual != NULL) {
        FILE *aux_p = fopen(atual->nome, "wb+");
        if (aux_p != NULL) fclose(aux_p);
        atual = atual->prox;
    }

    free(v);
    free(reservatorio);
    free(congelado);
    return n_part; //devolve o número de partições geradas

}
