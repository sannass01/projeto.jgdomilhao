#ifndef CSV_HANDLER_H
#define CSV_HANDLER_H

#include "pergunta.h"

// funções de carregamento e o salvamento CSV
void carregar_perguntas_csv(const char *nome_arquivo, Pergunta **perguntas, int *num_perguntas, int *capacidade);
void salvar_perguntas_csv(const char *nome_arquivo, Pergunta *perguntas, int num_perguntas);

#endif // csv_handler.h  


