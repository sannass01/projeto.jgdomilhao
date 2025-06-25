#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "pergunta.h"
#include "csv_handler.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

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
int pontuacao_garantida = 0; // Nova variável para pontuação garantida
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

Pergunta *pergunta_atual = NULL;
char resposta_jogador = ' ';
bool resposta_enviada = false;
bool resposta_certa = false;

// Função para desenhar texto com quebra de linha
void DrawTextWrapped(Font font, const char *texto, Rectangle rec, float fontSize, float spacing, Color tint)
{
    int tamanho = TextLength(texto);
    float textoOffsetY = 0;
    int comeco_linha = 0;
    int fim_linha = -1;

    for (int i = 0; i < tamanho; i++)
    {
        if ((texto[i] == ' ') || (texto[i] == '\t') || (texto[i] == '\n'))
        {
            fim_linha = i;
        }

        Vector2 textSize = MeasureTextEx(font, TextSubtext(texto, comeco_linha, (fim_linha - comeco_linha)), fontSize, spacing);

        if (((textSize.x + rec.x) >= (rec.x + rec.width)) && (fim_linha != -1))
        {
            DrawTextEx(font, TextSubtext(texto, comeco_linha, (fim_linha - comeco_linha)), (Vector2){ rec.x, rec.y + textoOffsetY }, fontSize, spacing, tint);
            textoOffsetY += (fontSize + spacing);
            comeco_linha = fim_linha + 1;
            fim_linha = -1;
        }
        else if (texto[i] == '\n')
        {
            DrawTextEx(font, TextSubtext(texto, comeco_linha, (i - comeco_linha)), (Vector2){ rec.x, rec.y + textoOffsetY }, fontSize, spacing, tint);
            textoOffsetY += (fontSize + spacing);
            comeco_linha = i + 1;
            fim_linha = -1;
        }
    }

    DrawTextEx(font, TextSubtext(texto, comeco_linha, (tamanho - comeco_linha)), (Vector2){ rec.x, rec.y + textoOffsetY }, fontSize, spacing, tint);
}

// Função para inicializar o jogo
void InitGame(Pergunta *perguntas, int num_perguntas) {
    todas_perguntas = perguntas;
    total_perguntas = num_perguntas;
    perguntas_respondidas_count = 0;
    pontuacao_atual = 0;
    pontuacao_garantida = 0; // Resetar pontuação garantida
    pergunta_atual_idx = 0;
    resposta_jogador = ' ';
    resposta_enviada = false;
    resposta_certa = false;

    if (perguntas_usadas_indices != NULL) {
        free(perguntas_usadas_indices);
    }
    perguntas_usadas_indices = (int *)calloc(total_perguntas, sizeof(int));

    // Obter a primeira pergunta
    pergunta_atual = obter_pergunta_por_dificuldade(todas_perguntas, total_perguntas, dificuldades_jogo[pergunta_atual_idx], perguntas_usadas_indices);
    if (pergunta_atual == NULL) {
        // Tratar erro: não há perguntas suficientes ou para a dificuldade
        printf("Erro: Não foi possível iniciar o jogo. Verifique o banco de perguntas.\n");
    }
}

// Função para atualizar o jogo
void UpdateGame(GameScreen *currentScreen) {
    if (IsKeyPressed(KEY_A)) resposta_jogador = 'A';
    if (IsKeyPressed(KEY_B)) resposta_jogador = 'B';
    if (IsKeyPressed(KEY_C)) resposta_jogador = 'C';
    if (IsKeyPressed(KEY_D)) resposta_jogador = 'D';

    if (resposta_jogador != ' ' && !resposta_enviada) {
        resposta_enviada = true;
        if (resposta_jogador == pergunta_atual->letra_correta) {
            resposta_certa = true;
            pontuacao_atual = valores_perguntas[pergunta_atual_idx];

            // Marcos de segurança
            if (pergunta_atual_idx == 4) { // Pergunta 5 (índice 4)
                pontuacao_garantida = valores_perguntas[4];
            } else if (pergunta_atual_idx == 9) { // Pergunta 10 (índice 9)
                pontuacao_garantida = valores_perguntas[9];
            }

        } else {
            resposta_certa = false;
        }
    }

    if (resposta_enviada && IsKeyPressed(KEY_ENTER)) {
        if (resposta_certa) {
            pergunta_atual_idx++;
            if (pergunta_atual_idx < 15) {
                pergunta_atual = obter_pergunta_por_dificuldade(todas_perguntas, total_perguntas, dificuldades_jogo[pergunta_atual_idx], perguntas_usadas_indices);
                if (pergunta_atual == NULL) {
                    // Tratar erro: não há perguntas suficientes ou para a dificuldade
                    printf("Erro: Não foi possível carregar a próxima pergunta.\n");
                    *currentScreen = END; // Ir para tela final em caso de erro
                }
                resposta_jogador = ' ';
                resposta_enviada = false;
                resposta_certa = false;
            } else {
                *currentScreen = END; // Fim do jogo, todas as perguntas respondidas
            }
        } else {
            // Resposta incorreta, fim do jogo
            pontuacao_atual = pontuacao_garantida; // Atribui a pontuação garantida
            *currentScreen = END;
        }
    }
}

// Função para desenhar o jogo
void DrawGame() {
    
    ClearBackground(RAYWHITE);

    if (pergunta_atual != NULL) {
        // Definir a área para o enunciado da pergunta
        Rectangle enunciadoRec = { 100, 100, SCREEN_WIDTH - 200, 200 }; // x, y, largura, altura
        int fontSize = 25;
        int spacing = 2; // Espaçamento entre linhas

        // Desenhar o enunciado com quebra de linha
        DrawTextWrapped(GetFontDefault(), pergunta_atual->enunciado, enunciadoRec, fontSize, spacing, BLACK);

        // Calcular a altura do texto do enunciado para posicionar as alternativas
        // Uma estimativa pode ser feita, ou um valor fixo que seja suficiente.
        // Para simplificar, vamos usar um Y inicial fixo para as alternativas que seja seguro.
        int alternativasStartY = enunciadoRec.y + enunciadoRec.height + 20; // Começa abaixo da área do enunciado + um espaçamento

        for (int i = 0; i < NUM_ALTERNATIVAS; i++) {
            DrawText(TextFormat("%c: %s", (char)('A' + i), pergunta_atual->alternativas[i]), 100, alternativasStartY + i * 50, 25, BLACK);
        }

        if (resposta_enviada) {
            if (resposta_certa) {
                DrawText("CORRETO! Pressione ENTER para continuar.", 100, alternativasStartY + NUM_ALTERNATIVAS * 50 + 20, 25, GREEN);
            } else {
                DrawText(TextFormat("INCORRETO! A resposta correta era %c. Pressione ENTER para continuar.", pergunta_atual->letra_correta), 100, alternativasStartY + NUM_ALTERNATIVAS * 50 + 20, 25, RED);
            }
        }
    } else {
        DrawText("Carregando perguntas...", SCREEN_WIDTH/2 - MeasureText("Carregando perguntas...", 25)/2, SCREEN_HEIGHT/2, 25, BLACK);
    }

    DrawText(TextFormat("Premiação: R$ %d", pontuacao_atual), 100, SCREEN_HEIGHT - 100, 25, DARKGRAY);
    DrawText(TextFormat("Pontuação Garantida: R$ %d", pontuacao_garantida), 100, SCREEN_HEIGHT - 70, 25, DARKGREEN);
    DrawText(TextFormat("Pergunta %d de 15", pergunta_atual_idx + 1), SCREEN_WIDTH - 300, SCREEN_HEIGHT - 100, 25, DARKGRAY);
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo do Milhão");
    SetTargetFPS(60);

    carregar_perguntas_csv("bancoperguntas.csv", &todas_perguntas, &total_perguntas, &capacidade_perguntas);

    GameScreen tela_atual = MENU;

    while (!WindowShouldClose()) {
        switch (tela_atual) {
            case MENU: {
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawText("JOGO DO MILHÃO", SCREEN_WIDTH/2 - MeasureText("JOGO DO MILHÃO", 60)/2, 200, 60, BLACK);
                
                // Botão Iniciar Jogo
                Rectangle botao_start = { SCREEN_WIDTH/2 - 150, 400, 300, 70 };
                DrawRectangleRec(botao_start, LIGHTGRAY);
                DrawText("Iniciar Jogo", botao_start.x + 50, botao_start.y + 20, 30, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), botao_start) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (total_perguntas < 15) {
                        printf("Não há perguntas suficientes para iniciar o jogo. São necessárias 15 perguntas.\n");
                        // Poderíamos exibir uma mensagem na tela também
                    } else {
                        InitGame(todas_perguntas, total_perguntas);
                        tela_atual = GAMEPLAY;
                    }
                }

                // Botão Sair
                Rectangle botao_saida = { SCREEN_WIDTH/2 - 150, 500, 300, 70 };
                DrawRectangleRec(botao_saida, LIGHTGRAY);
                DrawText("Sair", botao_saida.x + 110, botao_saida.y + 20, 30, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), botao_saida) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    CloseWindow();
                }

                EndDrawing();
            } break;
            case GAMEPLAY: {
                UpdateGame(&tela_atual);
                BeginDrawing();
                DrawGame();
                EndDrawing();
            } break;
            case END: {
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawText("FIM DE JOGO!", SCREEN_WIDTH/2 - MeasureText("FIM DE JOGO!", 60)/2, 200, 60, BLACK);
                DrawText(TextFormat("Sua Premiação final: R$ %d", pontuacao_atual), SCREEN_WIDTH/2 - MeasureText(TextFormat("Sua pontuação final: R$ %d", pontuacao_atual), 30)/2, 300, 30, BLACK);
                
                // Botão Voltar ao Menu
                Rectangle botao_voltar_menu = { SCREEN_WIDTH/2 - 150, 400, 300, 70 };
                DrawRectangleRec(botao_voltar_menu, LIGHTGRAY);
                DrawText("Voltar ao Menu", botao_voltar_menu.x + 30, botao_voltar_menu.y + 20, 30, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), botao_voltar_menu) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    tela_atual = MENU;
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


