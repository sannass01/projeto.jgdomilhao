#ifndef PERGUNTA_H
#define PERGUNTA_H

#define MAX_ENUNCIADO 256
#define MAX_ALTERNATIVA 128
#define MAX_DIFICULDADE 32
#define NUM_ALTERNATIVAS 4

typedef struct {
    char enunciado[MAX_ENUNCIADO];
    char alternativas[NUM_ALTERNATIVAS][MAX_ALTERNATIVA];
    char letra_correta;
    char dificuldade[MAX_DIFICULDADE];
} Pergunta;

// Funções 
void inserir_pergunta(Pergunta **perguntas, int *num_perguntas, int *capacidade);
void listar_perguntas(Pergunta *perguntas, int num_perguntas);
void pesquisar_perguntas(Pergunta *perguntas, int num_perguntas);
void alterar_pergunta(Pergunta *perguntas, int num_perguntas);
void excluir_pergunta(Pergunta **perguntas, int *num_perguntas, int *capacidade);

#endif // PERGUNTA_H


