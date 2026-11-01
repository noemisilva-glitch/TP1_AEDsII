#include "funcionarios.h"
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include "particoes.h"

void selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n) {
    rewind(arq); // Posiciona no início do arquivo de entrada

    // Aloca memória para a RAM (tamanho M) e Reservatório (tamanho n)
    TFunc **v = (TFunc **) malloc(sizeof(TFunc *) * M);
    TFunc **reservatorio = (TFunc **) malloc(sizeof(TFunc *) * n);
    bool *congelado = (bool *) malloc(sizeof(bool) * M);

    int reg_lidos = 0;
    int num_reservatorio = 0;
    int elementos_na_memoria = 0;

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
            }
            continue;
        }

        // Grava o menor registro na partição ativa
        if (p != NULL) {
            salva_funcionario(v[menor_idx], p);
            imprime_funcionario(v[menor_idx]);
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
            if (atual != NULL) {
                nome_particao = atual->nome;
                atual = atual->prox;
                p = fopen(nome_particao, "wb+");
                printf("\nCriando particao (via reservatorio): %s\n", nome_particao);
            } else {
                p = NULL;
            }
        }
    }

    if (p != NULL) {
        fclose(p);
    }

    // Garante que qualquer arquivo de partição restante na lista seja criado vazio (evita erro de arquivo não encontrado)
    while (atual != NULL) {
        FILE *aux_p = fopen(atual->nome, "wb+");
        if (aux_p != NULL) fclose(aux_p);
        atual = atual->prox;
    }

    free(v);
    free(reservatorio);
    free(congelado);
}