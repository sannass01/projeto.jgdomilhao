#include "csv_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void carregar_perguntas_csv(const char *nome_arquivo, Pergunta **perguntas, int *num_perguntas, int *capacidade) {
    FILE *file = fopen(nome_arquivo, "r");
    if (file == NULL) {
        printf("Arquivo %s não encontrado. Criando um novo arquivo CSV.\n", nome_arquivo);
        file = fopen(nome_arquivo, "w"); // cria o arquivo 
        if (file == NULL) { // verifica se abriu certo
            perror("Erro ao criar o arquivo CSV.\n");
            exit(1);
        }
        fclose(file);
        return;
    }

    char linha[1024]; // tamanho 
    fgets(linha, sizeof(linha), file);
    while (fgets(linha, sizeof(linha), file) != NULL) {
       
        linha[strcspn(linha, "\n")] = 0;
        //alocando mais espaços 
        if (*num_perguntas == *capacidade) {
            *capacidade = (*capacidade == 0) ? 1 : *capacidade * 2;
            *perguntas = (Pergunta *) realloc(*perguntas, *capacidade * sizeof(Pergunta));
            if (*perguntas == NULL) {
                perror("Erro de alocação de memória ao carregar CSV.\n");
                exit(1);
            }
        }

        Pergunta nova_pergunta;
        char *token;
        int campo = 0;

        token = strtok(linha, ";");
        if (token != NULL) strcpy(nova_pergunta.enunciado, token);
        campo++;

        for (int i = 0; i < NUM_ALTERNATIVAS; i++) {
            token = strtok(NULL, ";");
            if (token != NULL) strcpy(nova_pergunta.alternativas[i], token);
            campo++;
        }

        token = strtok(NULL, ";");
        if (token != NULL) nova_pergunta.letra_correta = token[0];
        campo++;

        //dividi a string 
        token = strtok(NULL, ";");
        if (token != NULL) {
            int dificuldade_num = atoi(token);
            switch (dificuldade_num) {
                case 1:
                    strcpy(nova_pergunta.dificuldade, "muito fácil");
                    break;
                case 2:
                    strcpy(nova_pergunta.dificuldade, "fácil");
                    break;
                case 3:
                    strcpy(nova_pergunta.dificuldade, "médio");
                    break;
                case 4:
                    strcpy(nova_pergunta.dificuldade, "difícil");
                    break;
                case 5:
                    strcpy(nova_pergunta.dificuldade, "muito difícil");
                    break;
                default:
                    strcpy(nova_pergunta.dificuldade, "desconhecida");
                    break;
            }
        }
        campo++;

        
        if (campo == 7) { 
            (*perguntas)[*num_perguntas] = nova_pergunta;
            (*num_perguntas)++;
        } else {
            printf("Linha inválida no CSV (campos esperados: 7, encontrados: %d): %s\n", campo, linha);
        }
    }
    fclose(file);
}
//funçao para salvar as perguntas e verificar se abriu o arquivo
void salvar_perguntas_csv(const char *nome_arquivo, Pergunta *perguntas, int num_perguntas) {
    FILE *file = fopen(nome_arquivo, "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo %s para escrita.\n", nome_arquivo);
        return;
    }
    //print das perguntas e alternativas, alternativa certas e dificuldade
    for (int i = 0; i < num_perguntas; i++) {
        fprintf(file, "%s;", perguntas[i].enunciado);
        for (int j = 0; j < NUM_ALTERNATIVAS; j++) {
            fprintf(file, "%s;", perguntas[i].alternativas[j]);
        }
        fprintf(file, "%c;", perguntas[i].letra_correta);
        fprintf(file, "%s\n", perguntas[i].dificuldade);
    }
    fclose(file);
}


