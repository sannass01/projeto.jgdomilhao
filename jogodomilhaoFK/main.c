#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "pergunta.h"
#include "csv_handler.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

// Estados do jogo
typedef enum GameScreen {
    MENU = 0,
    GAMEPLAY,
    END
} GameScreen;

// Função para limpar o buffer do teclado (ainda útil para entrada de texto, se necessário)
void limpar_buffer() {
    int c;
    while ((c = GetCharPressed()) != 0);
}

// Função para embaralhar um array de perguntas
void embaralhar_perguntas(Pergunta *perguntas, int num_perguntas) {
    srand(time(NULL));
    for (int i = num_perguntas - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Pergunta temp = perguntas[i];
        perguntas[i] = perguntas[j];
        perguntas[j] = temp;
    }
}

// Função para obter uma pergunta de uma dificuldade específica
Pergunta *obter_pergunta_por_dificuldade(Pergunta *perguntas, int num_perguntas, const char *dificuldade_str, int *indice_pergunta_usada) {
    int *indices_disponiveis = (int *)malloc(num_perguntas * sizeof(int));
    int count_disponiveis = 0;

    for (int i = 0; i < num_perguntas; i++) {
        if (strcmp(perguntas[i].dificuldade, dificuldade_str) == 0 && !indice_pergunta_usada[i]) {
            indices_disponiveis[count_disponiveis++] = i;
        }
    }

    if (count_disponiveis == 0) {
        free(indices_disponiveis);
        return NULL;
    }

    srand(time(NULL));
    int indice_aleatorio = rand() % count_disponiveis;
    int indice_real = indices_disponiveis[indice_aleatorio];

    indice_pergunta_usada[indice_real] = 1;
    free(indices_disponiveis);
    return &perguntas[indice_real];
}

// Variáveis globais para o jogo
Pergunta *todas_perguntas = NULL;
int total_perguntas = 0;
int capacidade_perguntas = 0;

int pontuacao_atual = 0;
int pergunta_atual_idx = 0;
int perguntas_respondidas_count = 0;
int *perguntas_usadas_indices = NULL;

const char *dificuldades_jogo[] = {
    "muito fácil", "muito fácil",
    "fácil", "fácil",
    "médio", "médio", "médio", "médio",
    "difícil", "difícil", "difícil", "difícil",
    "muito difícil", "muito difícil", "muito difícil"
};
int valores_perguntas[] = {1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 300000, 400000, 500000, 600000, 700000, 800000, 1000000};

Pergunta *current_question = NULL;
char player_answer = ' ';
bool answer_submitted = false;
bool correct_answer = false;

// Função para inicializar o jogo
void InitGame(Pergunta *perguntas, int num_perguntas) {
    todas_perguntas = perguntas;
    total_perguntas = num_perguntas;
    perguntas_respondidas_count = 0;
    pontuacao_atual = 0;
    pergunta_atual_idx = 0;
    player_answer = ' ';
    answer_submitted = false;
    correct_answer = false;

    if (perguntas_usadas_indices != NULL) {
        free(perguntas_usadas_indices);
    }
    perguntas_usadas_indices = (int *)calloc(total_perguntas, sizeof(int));

    // Obter a primeira pergunta
    current_question = obter_pergunta_por_dificuldade(todas_perguntas, total_perguntas, dificuldades_jogo[pergunta_atual_idx], perguntas_usadas_indices);
    if (current_question == NULL) {
        // Tratar erro: não há perguntas suficientes ou para a dificuldade
        printf("Erro: Não foi possível iniciar o jogo. Verifique o banco de perguntas.\n");
    }
}

// Função para atualizar o jogo
void UpdateGame(GameScreen *currentScreen) {
    if (IsKeyPressed(KEY_A)) player_answer = 'A';
    if (IsKeyPressed(KEY_B)) player_answer = 'B';
    if (IsKeyPressed(KEY_C)) player_answer = 'C';
    if (IsKeyPressed(KEY_D)) player_answer = 'D';

    if (player_answer != ' ' && !answer_submitted) {
        answer_submitted = true;
        if (player_answer == current_question->letra_correta) {
            correct_answer = true;
            pontuacao_atual = valores_perguntas[pergunta_atual_idx];
        } else {
            correct_answer = false;
        }
    }

    if (answer_submitted && IsKeyPressed(KEY_ENTER)) {
        if (correct_answer) {
            pergunta_atual_idx++;
            if (pergunta_atual_idx < 15) {
                current_question = obter_pergunta_por_dificuldade(todas_perguntas, total_perguntas, dificuldades_jogo[pergunta_atual_idx], perguntas_usadas_indices);
                if (current_question == NULL) {
                    // Tratar erro: não há perguntas suficientes ou para a dificuldade
                    printf("Erro: Não foi possível carregar a próxima pergunta.\n");
                    *currentScreen = END; // Ir para tela final em caso de erro
                }
                player_answer = ' ';
                answer_submitted = false;
                correct_answer = false;
            } else {
                *currentScreen = END; // Fim do jogo, todas as perguntas respondidas
            }
        } else {
            *currentScreen = END; // Resposta incorreta, fim do jogo
        }
    }
}

// Função para desenhar o jogo
void DrawGame() {
    ClearBackground(RAYWHITE);

    if (current_question != NULL) {
        DrawText(current_question->enunciado, 50, 50, 20, BLACK);
        for (int i = 0; i < NUM_ALTERNATIVAS; i++) {
            DrawText(TextFormat("%c: %s", (char)('A' + i), current_question->alternativas[i]), 50, 100 + i * 30, 20, BLACK);
        }

        if (answer_submitted) {
            if (correct_answer) {
                DrawText("CORRETO! Pressione ENTER para continuar.", 50, 300, 20, GREEN);
            } else {
                DrawText(TextFormat("INCORRETO! A resposta correta era %c. Pressione ENTER para continuar.", current_question->letra_correta), 50, 300, 20, RED);
            }
        }
    } else {
        DrawText("Carregando perguntas...", 50, 50, 20, BLACK);
    }

    DrawText(TextFormat("Pontuação: R$ %d", pontuacao_atual), 50, SCREEN_HEIGHT - 50, 20, DARKGRAY);
    DrawText(TextFormat("Pergunta %d de 15", pergunta_atual_idx + 1), SCREEN_WIDTH - 200, SCREEN_HEIGHT - 50, 20, DARKGRAY);
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo do Milhão");
    SetTargetFPS(60);

    carregar_perguntas_csv("bancoperguntas.csv", &todas_perguntas, &total_perguntas, &capacidade_perguntas);

    GameScreen currentScreen = MENU;

    while (!WindowShouldClose()) {
        switch (currentScreen) {
            case MENU: {
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawText("JOGO DO MILHÃO", SCREEN_WIDTH/2 - MeasureText("JOGO DO MILHÃO", 40)/2, 100, 40, BLACK);
                
                // Botão Iniciar Jogo
                Rectangle startButton = { SCREEN_WIDTH/2 - 100, 200, 200, 50 };
                DrawRectangleRec(startButton, LIGHTGRAY);
                DrawText("Iniciar Jogo", startButton.x + 20, startButton.y + 15, 20, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), startButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (total_perguntas < 15) {
                        printf("Não há perguntas suficientes para iniciar o jogo. São necessárias 15 perguntas.\n");
                        // Poderíamos exibir uma mensagem na tela também
                    } else {
                        InitGame(todas_perguntas, total_perguntas);
                        currentScreen = GAMEPLAY;
                    }
                }

                // Botão Sair
                Rectangle exitButton = { SCREEN_WIDTH/2 - 100, 270, 200, 50 };
                DrawRectangleRec(exitButton, LIGHTGRAY);
                DrawText("Sair", exitButton.x + 80, exitButton.y + 15, 20, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), exitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    CloseWindow();
                }

                EndDrawing();
            } break;
            case GAMEPLAY: {
                UpdateGame(&currentScreen);
                BeginDrawing();
                DrawGame();
                EndDrawing();
            } break;
            case END: {
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawText("FIM DE JOGO!", SCREEN_WIDTH/2 - MeasureText("FIM DE JOGO!", 40)/2, 100, 40, BLACK);
                DrawText(TextFormat("Sua pontuação final: R$ %d", pontuacao_atual), SCREEN_WIDTH/2 - MeasureText(TextFormat("Sua pontuação final: R$ %d", pontuacao_atual), 20)/2, 200, 20, BLACK);
                
                // Botão Voltar ao Menu
                Rectangle backToMenuButton = { SCREEN_WIDTH/2 - 100, 300, 200, 50 };
                DrawRectangleRec(backToMenuButton, LIGHTGRAY);
                DrawText("Voltar ao Menu", backToMenuButton.x + 10, backToMenuButton.y + 15, 20, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), backToMenuButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    currentScreen = MENU;
                }

                EndDrawing();
            } break;
            default: break;
        }
    }

    free(todas_perguntas);
    if (perguntas_usadas_indices != NULL) {
        free(perguntas_usadas_indices);
    }
    CloseWindow();

    return 0;
}


