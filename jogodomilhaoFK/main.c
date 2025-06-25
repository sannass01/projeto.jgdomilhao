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

// os estados do jogo
typedef enum GameScreen {
    MENU = 0,
    GAMEPLAY,
    END
} GameScreen;

// funcao limpar o buffer
void limpar_buffer() {
    int c;
    while ((c = GetCharPressed()) != 0);
}

// funcao para embaralhar o array
void embaralhar_perguntas(Pergunta *perguntas, int num_perguntas) {
    srand(time(NULL));
    for (int i = num_perguntas - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Pergunta temp = perguntas[i];
        perguntas[i] = perguntas[j];
        perguntas[j] = temp;
    }
}

// funcao para obter o nivel da dificuldade 
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


Pergunta *todas_perguntas = NULL;
int total_perguntas = 0;
int capacidade_perguntas = 0;

int pontuacao_atual = 0;
int pontuacao_garantida = 0; 
int pergunta_atual_idx = 0;
int perguntas_respondidas_count = 0;
int *perguntas_usadas_indices = NULL;


// constante para as dificuldaddes dos jogos
const char *dificuldades_jogo[] = {
    "muito fácil", "muito fácil",
    "fácil", "fácil",
    "médio", "médio", "médio", "médio",
    "difícil", "difícil", "difícil", "difícil",
    "muito difícil", "muito difícil", "muito difícil"
};
int valores_perguntas[] = {1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 300000, 400000, 500000, 600000, 700000, 800000, 1000000};

// passagem por parametro
Pergunta *pergunta_atual = NULL;
char resposta_jogador = ' ';
bool resposta_enviada = false;
bool resposta_certa = false;

// funçao para escrever no jogo 
void Escrever_texto(Font font, const char *texto, Rectangle rec, float fontSize, float spacing, Color tint)
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

// funcao para iniciar o jogo
void Iniciar_jogo(Pergunta *perguntas, int num_perguntas) {
    todas_perguntas = perguntas;
    total_perguntas = num_perguntas;
    perguntas_respondidas_count = 0;
    pontuacao_atual = 0;
    pontuacao_garantida = 0; // varial usada para resetar a pontuação
    pergunta_atual_idx = 0;
    resposta_jogador = ' ';
    resposta_enviada = false;
    resposta_certa = false;

    if (perguntas_usadas_indices != NULL) {
        free(perguntas_usadas_indices);
    }
    perguntas_usadas_indices = (int *)calloc(total_perguntas, sizeof(int));

    // obter a primeira pergunta
    pergunta_atual = obter_pergunta_por_dificuldade(todas_perguntas, total_perguntas, dificuldades_jogo[pergunta_atual_idx], perguntas_usadas_indices);
    // if para verificar se algo deu errrado 
    if (pergunta_atual == NULL) {
        printf("Erro: Não foi possível iniciar o jogo. Verifique o banco de perguntas.\n");
    }
}

// funcao para atualizar 
void Atualizar_jogo(GameScreen *currentScreen) {
    // para ver qual tecla o jogador fez 
    if (IsKeyPressed(KEY_A)) resposta_jogador = 'A';
    if (IsKeyPressed(KEY_B)) resposta_jogador = 'B';
    if (IsKeyPressed(KEY_C)) resposta_jogador = 'C';
    if (IsKeyPressed(KEY_D)) resposta_jogador = 'D';

    // verificar se a resposta do jogador é valida 
    if (resposta_jogador != ' ' && !resposta_enviada) {
        resposta_enviada = true;
        if (resposta_jogador == pergunta_atual->letra_correta) {
            resposta_certa = true;
            pontuacao_atual = valores_perguntas[pergunta_atual_idx];

            // paramentros para a segurança do codigo
            if (pergunta_atual_idx == 4) { // pergunta 5 (tamanho 4)
                pontuacao_garantida = valores_perguntas[4];
            } else if (pergunta_atual_idx == 9) { // pergunta  10 (tamanho  9)
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
                    printf("Erro: Não foi possível carregar a próxima pergunta.\n");
                    *currentScreen = END; //caso de erro, ele vai para a tela final
                }
                resposta_jogador = ' ';
                resposta_enviada = false;
                resposta_certa = false;
            } else {
                *currentScreen = END; // quando todas as perguntas ja foram respondidas
                // fim do jogo
            }
        } else {
            // else para ver a resposta incorreta
            // se tiver incorreta fim do jogo
            pontuacao_atual = pontuacao_garantida; // salva a pontuação garantida 
            *currentScreen = END;
        }
    }
}

// essa funcao foi feita para escrever no jogo
void Escrever_jogo() {
    
    ClearBackground(RAYWHITE);

    if (pergunta_atual != NULL) {
        //usado para definir area do enunciado
        Rectangle enunciadoRec = { 100, 100, SCREEN_WIDTH - 200, 200 }; // x = largura //  altura = y
        int fontSize = 25;
        int spacing = 2; // a separação entre as linhas

        // escrever o enunciado 
        Escrever_texto(GetFontDefault(), pergunta_atual->enunciado, enunciadoRec, fontSize, spacing, BLACK);

    
        // calcular a posição da altura no enunciado
        // com isso posiciona as alternativas certo
        int alternativasStartY = enunciadoRec.y + enunciadoRec.height + 20; 

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
    // funcao para escrever a premiação, pontuação e as perguntas na tela
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
                
                // botão para iniciar o jogo
                Rectangle botao_start = { SCREEN_WIDTH/2 - 150, 400, 300, 70 };
                DrawRectangleRec(botao_start, LIGHTGRAY);
                DrawText("Iniciar Jogo", botao_start.x + 50, botao_start.y + 20, 30, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), botao_start) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (total_perguntas < 15) {
                        printf("Não há perguntas suficientes para iniciar o jogo. São necessárias 15 perguntas.\n");
                       
                    } else {
                        Iniciar_jogo(todas_perguntas, total_perguntas);
                        tela_atual = GAMEPLAY;
                    }
                }

                // o botão de sair do jogo
                Rectangle botao_saida = { SCREEN_WIDTH/2 - 150, 500, 300, 70 };
                DrawRectangleRec(botao_saida, LIGHTGRAY);
                DrawText("Sair", botao_saida.x + 110, botao_saida.y + 20, 30, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), botao_saida) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    CloseWindow();
                }

                EndDrawing();
            } break;
            case GAMEPLAY: {
                Atualizar_jogo(&tela_atual);
                BeginDrawing();
                Escrever_jogo();
                EndDrawing();
            } break;
            case END: {
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawText("FIM DE JOGO!", SCREEN_WIDTH/2 - MeasureText("FIM DE JOGO!", 60)/2, 200, 60, BLACK);
                DrawText(TextFormat("Sua Premiação final: R$ %d", pontuacao_atual), SCREEN_WIDTH/2 - MeasureText(TextFormat("Sua pontuação final: R$ %d", pontuacao_atual), 30)/2, 300, 30, BLACK);
                
                //botao de voltar para o menu inicial
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