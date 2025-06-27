#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pergunta.h"
#include "csv_handler.h"

// Função para limpar o buffer de entrada
void limpar_buffer_entrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    Pergunta *perguntas = NULL;
    int num_perguntas = 0;
    int capacidade_perguntas = 0;
    char nome_arquivo_csv[] = "bancoperguntas.csv";//variave

    // Carregar perguntas existentes ao iniciar
    carregar_perguntas_csv(nome_arquivo_csv, &perguntas, &num_perguntas, &capacidade_perguntas);
    printf("\n%d perguntas carregadas de %s.\n", num_perguntas, nome_arquivo_csv);

    int opcao;
    do {
        printf("\n--- Menu de Gerenciamento de Perguntas ---\n");
        printf("1. Inserir nova pergunta\n");
        printf("2. Listar todas as perguntas\n");
        printf("3. Pesquisar perguntas\n");
        printf("4. Alterar pergunta\n");
        printf("5. Excluir pergunta\n");
        printf("6. Salvar perguntas no CSV\n");
        printf("0. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        limpar_buffer_entrada(); // Limpa o buffer após scanf

        switch (opcao) {
            case 1:
                inserir_pergunta(&perguntas, &num_perguntas, &capacidade_perguntas);
                break;
            case 2:
                listar_perguntas(perguntas, num_perguntas);
                break;
            case 3:
                pesquisar_perguntas(perguntas, num_perguntas);
                break;
            case 4:
                alterar_pergunta(perguntas, num_perguntas);
                break;
            case 5:
                excluir_pergunta(&perguntas, &num_perguntas, &capacidade_perguntas);
                break;
            case 6:
                salvar_perguntas_csv(nome_arquivo_csv, perguntas, num_perguntas);
                printf("Perguntas salvas em %s.\n", nome_arquivo_csv);
                break;
            case 0:
                printf("Saindo do programa. Não se esqueça de salvar as alterações!\n");
                break;
            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    } while (opcao != 0);

    // Liberar memória alocada antes de sair
    if (perguntas != NULL) {
        free(perguntas);
    }

    return 0;
}


