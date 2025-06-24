#include "pergunta.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//funcao põe uma pergunta nova
void inserir_pergunta(Pergunta **perguntas, int *num_perguntas, int *capacidade) {
    if (*num_perguntas == *capacidade) {
        *capacidade = (*capacidade == 0) ? 1 : *capacidade * 2;
        *perguntas = (Pergunta *) realloc(*perguntas, *capacidade * sizeof(Pergunta));
        if (*perguntas == NULL) {
            perror("Erro de alocação de memória.\n");
            exit(1);
        }
    }
    // pedindo pro usuario digitar uma pergunta 
    Pergunta nova_pergunta;
    printf("Digite o enunciado da pergunta: ");
    fgets(nova_pergunta.enunciado, MAX_ENUNCIADO, stdin);
    nova_pergunta.enunciado[strcspn(nova_pergunta.enunciado, "\n")] = 0; 
    

    //usuario digitar todas as alternativas 
    for (int i = 0; i < NUM_ALTERNATIVAS; i++) {
        printf("Digite a alternativa %c: ", (char)("A"[0] + i));
        fgets(nova_pergunta.alternativas[i], MAX_ALTERNATIVA, stdin);
        nova_pergunta.alternativas[i][strcspn(nova_pergunta.alternativas[i], "\n")] = 0; 
    }


    //pedindo ao usuario a letra certa da alternativa 
    printf("Digite a letra da alternativa correta (A, B, C ou D): ");
    scanf(" %c", &nova_pergunta.letra_correta);
    nova_pergunta.letra_correta = toupper(nova_pergunta.letra_correta);
    while (nova_pergunta.letra_correta < 'A' || nova_pergunta.letra_correta > 'D') {
        printf("Letra inválida. Digite A, B, C ou D: ");
        scanf(" %c", &nova_pergunta.letra_correta);
        nova_pergunta.letra_correta = toupper(nova_pergunta.letra_correta);
    }
    while (getchar() != '\n'); 
    // pedindo para o usuario o nivel da dificuldade
    printf("Digite o nível de dificuldade (muito fácil, fácil, médio, difícil, muito difícil): ");
    fgets(nova_pergunta.dificuldade, MAX_DIFICULDADE, stdin);
    nova_pergunta.dificuldade[strcspn(nova_pergunta.dificuldade, "\n")] = 0; 
    (*perguntas)[*num_perguntas] = nova_pergunta;
    (*num_perguntas)++;
    printf("Pergunta inserida com sucesso!\n");
}

//funçao para listar todas as perguntas, alternativas, as corretas e as dificuldades 
void listar_perguntas(Pergunta *perguntas, int num_perguntas) {
    if (num_perguntas == 0) {
        printf("Nenhuma pergunta cadastrada.\n");
        return;
    }
    for (int i = 0; i < num_perguntas; i++) {
        printf("\n--- Pergunta %d ---\n", i + 1);
        printf("Enunciado: %s\n", perguntas[i].enunciado);
        for (int j = 0; j < NUM_ALTERNATIVAS; j++) {
            printf("Alternativa %c: %s\n", (char)('A' + j), perguntas[i].alternativas[j]);
        }
        printf("Correta: %c\n", perguntas[i].letra_correta);
        printf("Dificuldade: %s\n", perguntas[i].dificuldade);
    }
}

//funçao para pesquisar as perguntas no codigo
void pesquisar_perguntas(Pergunta *perguntas, int num_perguntas) {
    char termo[MAX_ENUNCIADO];
    printf("Digite o termo de pesquisa (enunciado ou dificuldade): ");
    fgets(termo, MAX_ENUNCIADO, stdin);
    termo[strcspn(termo, "\n")] = 0; 

    int encontrados = 0; //contador para ver quantas foram encontradas 
    for (int i = 0; i < num_perguntas; i++) {
        if (strstr(perguntas[i].enunciado, termo) != NULL || strstr(perguntas[i].dificuldade, termo) != NULL) {
            printf("\n--- Resultado %d ---\n", encontrados + 1);
            printf("Enunciado: %s\n", perguntas[i].enunciado);
            for (int j = 0; j < NUM_ALTERNATIVAS; j++) {
                printf("Alternativa %c: %s\n", (char)('A' + j), perguntas[i].alternativas[j]);
            }
            printf("Correta: %c\n", perguntas[i].letra_correta);
            printf("Dificuldade: %s\n", perguntas[i].dificuldade);
            encontrados++;
        }
    }

    //If encontra perguntas
    if (encontrados == 0) {
        printf("Nenhuma pergunta encontrada com o termo fornecido.\n");
    }
}

//função alterar pergunta
void alterar_pergunta(Pergunta *perguntas, int num_perguntas) {
    listar_perguntas(perguntas, num_perguntas);
    if (num_perguntas == 0) return;

    int indice;
    printf("Digite o número da pergunta que deseja alterar: ");
    scanf("%d", &indice);
    while (getchar() != '\n'); 

    //quantidade de perguntas que deseja alterar
    if (indice > 0 && indice <= num_perguntas) {
        Pergunta *pergunta = &perguntas[indice - 1];
        printf("\nAlterando Pergunta %d:\n", indice);
        printf("Enunciado atual: %s\n", pergunta->enunciado);
        printf("Novo enunciado (deixe em branco para manter): ");
        char novo_enunciado[MAX_ENUNCIADO];
        fgets(novo_enunciado, MAX_ENUNCIADO, stdin);
        novo_enunciado[strcspn(novo_enunciado, "\n")] = 0;
        if (strlen(novo_enunciado) > 0) {
            strcpy(pergunta->enunciado, novo_enunciado);
        }

        
        for (int i = 0; i < NUM_ALTERNATIVAS; i++) {
            printf("Alternativa %c atual: %s\n", (char)('A' + i), pergunta->alternativas[i]);
            printf("Nova alternativa %c (deixe em branco para manter): ", (char)('A' + i));
            char nova_alternativa[MAX_ALTERNATIVA];
            fgets(nova_alternativa, MAX_ALTERNATIVA, stdin);
            nova_alternativa[strcspn(nova_alternativa, "\n")] = 0;
            if (strlen(nova_alternativa) > 0) {
                strcpy(pergunta->alternativas[i], nova_alternativa);
            }
        }

        printf("Letra correta atual: %c\n", pergunta->letra_correta);
        printf("Nova letra correta (A, B, C ou D - deixe em branco para manter): ");
        char nova_letra_correta_str[2];
        fgets(nova_letra_correta_str, 2, stdin);
        while (getchar() != '\n'); 
        if (strlen(nova_letra_correta_str) > 0) {
            char nova_letra_correta = nova_letra_correta_str[0];
            if (nova_letra_correta >= 'a' && nova_letra_correta <= 'd') {
                nova_letra_correta = toupper(nova_letra_correta);
            }
            if (nova_letra_correta >= 'A' && nova_letra_correta <= 'D') {
                pergunta->letra_correta = nova_letra_correta;
            } else {
                printf("Letra correta inválida. Mantendo a anterior.\n");
            }
        }

        printf("Dificuldade atual: %s\n", pergunta->dificuldade);
        printf("Nova dificuldade (deixe em branco para manter): ");
        char nova_dificuldade[MAX_DIFICULDADE];
        fgets(nova_dificuldade, MAX_DIFICULDADE, stdin);
        nova_dificuldade[strcspn(nova_dificuldade, "\n")] = 0;
        if (strlen(nova_dificuldade) > 0) {
            strcpy(pergunta->dificuldade, nova_dificuldade);
        }
        printf("Pergunta alterada com sucesso!\n");
    } else {
        printf("Índice de pergunta inválido.\n");
    }
}

//funcao exclui pergunta
void excluir_pergunta(Pergunta **perguntas, int *num_perguntas, int *capacidade) {
    listar_perguntas(*perguntas, *num_perguntas);
    if (*num_perguntas == 0) return;

    int indice;
    printf("Digite o número da pergunta que deseja excluir: ");
    scanf("%d", &indice);
    while (getchar() != '\n'); 

    if (indice > 0 && indice <= *num_perguntas) {
        printf("Pergunta \'%s\' excluída com sucesso!\n", (*perguntas)[indice - 1].enunciado);
        for (int i = indice - 1; i < *num_perguntas - 1; i++) {
            (*perguntas)[i] = (*perguntas)[i + 1];
        }
        (*num_perguntas)--;
        if (*num_perguntas > 0 && *num_perguntas < *capacidade / 4) {
            *capacidade /= 2;
            *perguntas = (Pergunta *) realloc(*perguntas, *capacidade * sizeof(Pergunta));
            if (*perguntas == NULL) {
                printf("Erro de realocação de memória.\n");
                exit(1);
            }
        }
    } else {
        printf("Índice de pergunta inválido.\n");
    }
}


