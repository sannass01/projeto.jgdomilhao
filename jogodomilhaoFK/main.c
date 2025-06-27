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
        // Se não houver perguntas disponíveis para a dificuldade atual, tentar a próxima dificuldade
        // Isso evita um loop infinito se uma dificuldade específica não tiver perguntas suficientes
        return NULL;
    }

    srand(time(NULL));
    int indice_aleatorio = rand() % count_disponiveis;
    int indice_real = indices_disponiveis[indice_aleatorio];

    indice_pergunta_usada[indice_real] = 1;
    free(indices_disponiveis);
    return &perguntas[indice_real];
}

// constante para as dificuldaddes dos jogos
const char *dificuldades_jogo[] = {
    "muito fácil", "muito fácil",
    "fácil", "fácil",
    "médio", "médio", "médio", "médio",
    "difícil", "difícil", "difícil", "difícil",
    "muito difícil", "muito difícil", "muito difícil"
};
int valores_perguntas[] = {1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 300000, 400000, 500000, 600000, 700000, 800000, 1000000};

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
void Iniciar_jogo(Pergunta **todas_perguntas_ptr, int *total_perguntas_ptr, int *perguntas_respondidas_count_ptr, int *pontuacao_atual_ptr, int *pontuacao_garantida_ptr, int *pergunta_atual_idx_ptr, char *resposta_jogador_ptr, bool *resposta_enviada_ptr, bool *resposta_certa_ptr, Pergunta **pergunta_atual_ptr, int **perguntas_usadas_indices_ptr, int *skip_count_ptr) {
    *perguntas_respondidas_count_ptr = 0;
    *pontuacao_atual_ptr = 0;
    *pontuacao_garantida_ptr = 0; // varial usada para resetar a pontuação
    *pergunta_atual_idx_ptr = 0;
    *resposta_jogador_ptr = ' ';
    *resposta_enviada_ptr = false;
    *resposta_certa_ptr = false;
    *skip_count_ptr = 0; // Inicializa o contador de pulos

    if (*perguntas_usadas_indices_ptr != NULL) {
        free(*perguntas_usadas_indices_ptr);
    }
    *perguntas_usadas_indices_ptr = (int *)calloc(*total_perguntas_ptr, sizeof(int));

    // obter a primeira pergunta
    *pergunta_atual_ptr = obter_pergunta_por_dificuldade(*todas_perguntas_ptr, *total_perguntas_ptr, dificuldades_jogo[*pergunta_atual_idx_ptr], *perguntas_usadas_indices_ptr);
    // if para verificar se algo deu errrado 
    if (*pergunta_atual_ptr == NULL) {
        printf("Erro: Não foi possível iniciar o jogo. Verifique o banco de perguntas.\n");
    }
}

// funcao para atualizar 
void Atualizar_jogo(GameScreen *tela_atual, Pergunta **pergunta_atual_ptr, int *pergunta_atual_idx, int *pontuacao_atual, int *pontuacao_garantida, char *resposta_jogador, bool *resposta_enviada, bool *resposta_certa, Pergunta *todas_perguntas, int total_perguntas, int *perguntas_usadas_indices, Sound sfx_correct, Sound sfx_wrong, int *contador_pulo) {
    // Lógica para o botão Pular
    Rectangle botao_pular = { SCREEN_WIDTH - 200, 250, 150, 50 };
    if (CheckCollisionPointRec(GetMousePosition(), botao_pular) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && *contador_pulo < 3) {
        (*contador_pulo)++;
        *resposta_certa = true; // Simula resposta correta
        *resposta_enviada = true;
        PlaySound(sfx_correct);
    }

    // para ver qual tecla o jogador fez 
    if (IsKeyPressed(KEY_A)) *resposta_jogador = 'A';
    if (IsKeyPressed(KEY_B)) *resposta_jogador = 'B';
    if (IsKeyPressed(KEY_C)) *resposta_jogador = 'C';
    if (IsKeyPressed(KEY_D)) *resposta_jogador = 'D';

    // verificar se a resposta do jogador é valida 
    if (*resposta_jogador != ' ' && !*resposta_enviada) {
        *resposta_enviada = true;
        if (*resposta_jogador == (*pergunta_atual_ptr)->letra_correta) {
            *resposta_certa = true;
            *pontuacao_atual = valores_perguntas[*pergunta_atual_idx];
            PlaySound(sfx_correct);

            // paramentros para a segurança do codigo
            if (*pergunta_atual_idx == 4) { // pergunta 5 (tamanho 4)
                *pontuacao_garantida = valores_perguntas[4];
            } else if (*pergunta_atual_idx == 9) { // pergunta  10 (tamanho  9)
                *pontuacao_garantida = valores_perguntas[9];
            }

        } else {
            *resposta_certa = false;
            PlaySound(sfx_wrong);
        }
    }

    if (*resposta_enviada && IsKeyPressed(KEY_ENTER)) {
        if (*resposta_certa) {
            (*pergunta_atual_idx)++;
            if (*pergunta_atual_idx < 15) {
                *pergunta_atual_ptr = obter_pergunta_por_dificuldade(todas_perguntas, total_perguntas, dificuldades_jogo[*pergunta_atual_idx], perguntas_usadas_indices);
                if (*pergunta_atual_ptr == NULL) {
                    printf("Não há mais perguntas disponíveis para a dificuldade atual. Fim de jogo.\n");
                    *tela_atual = END; // Avança para a tela final se não houver mais perguntas
                }
                *resposta_jogador = ' ';
                *resposta_enviada = false;
                *resposta_certa = false;
            } else {
                *tela_atual = END; // quando todas as perguntas ja foram respondidas
                // fim do jogo
            }
        } else {
            // else para ver a resposta incorreta
            // se tiver incorreta fim do jogo
            *pontuacao_atual = *pontuacao_garantida; // salva a pontuação garantida 
            *tela_atual = END;
        }
    }
}

// essa funcao foi feita para escrever no jogo
void Escrever_jogo(Pergunta *pergunta_atual, int pontuacao_atual, int pontuacao_garantida, int pergunta_atual_idx, bool resposta_enviada, bool resposta_certa, int conta_pulo) {
    
    ClearBackground(RAYWHITE);

    if (pergunta_atual != NULL) {
        //usado para definir area do enunciado
        Rectangle enunciadoRec = { 100, 100, SCREEN_WIDTH - 200, 200 }; // x = largura //  altura = y
        int tamanho_fonte = 25;
        int espacamento = 2; // a separação entre as linhas

        // escrever o enunciado 
        Escrever_texto(GetFontDefault(), pergunta_atual->enunciado, enunciadoRec, tamanho_fonte, espacamento, BLACK);

    
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

    // Desenha o botão Pular
    Rectangle botao_pular = { SCREEN_WIDTH - 200, 250, 150, 50 };
    DrawRectangleRec(botao_pular, (conta_pulo < 3) ? BLUE : GRAY);
    DrawText("Pular", botao_pular.x + 45, botao_pular.y + 15, 20, WHITE);
    DrawText(TextFormat("Pulos restantes: %d", 3 - conta_pulo), SCREEN_WIDTH - 250, 320, 20, BLACK);

    // funcao para escrever a premiação, pontuação e as perguntas na tela
    DrawText(TextFormat("Premiação: R$ %d", pontuacao_atual), 100, SCREEN_HEIGHT - 100, 25, DARKGRAY);
    DrawText(TextFormat("Pontuação Garantida: R$ %d", pontuacao_garantida), 100, SCREEN_HEIGHT - 70, 25, DARKGREEN);
    DrawText(TextFormat("Pergunta %d de 15", pergunta_atual_idx + 1), SCREEN_WIDTH - 300, SCREEN_HEIGHT - 100, 25, DARKGRAY);
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo do Milhão");
    SetTargetFPS(60);
    InitAudioDevice();

    Pergunta *todas_perguntas = NULL;
    int total_perguntas = 0;
    int capacidade_perguntas = 0;

    int pontuacao_atual = 0;
    int pontuacao_garantida = 0; 
    int pergunta_atual_idx = 0;
    int perguntas_respondidas_count = 0;
    int *perguntas_usadas_indices = NULL;

    Pergunta *pergunta_atual = NULL;
    char resposta_jogador = ' ';
    bool resposta_enviada = false;
    bool resposta_certa = false;
    int conta_pulo = 0;

    // Carregar texturas das imagens de fundo
    Texture2D teladefundo_menu = LoadTexture("teladefundo_menu.png");
    Texture2D teladefundo_gameplay = LoadTexture("teladefundo_gameplay.png");
    Texture2D teladefundo_end = LoadTexture("teladefundo_end.png");

    // Carregar sons
    Sound sfx_correct = LoadSound("correct_answer.ogg");
    Sound sfx_wrong = LoadSound("wrong_answer.ogg");
    Music music_menu = LoadMusicStream("Watch_Dogs_Main_Menu_Theme.ogg");

    carregar_perguntas_csv("bancoperguntas.csv", &todas_perguntas, &total_perguntas, &capacidade_perguntas);

    GameScreen tela_atual = MENU;

    while (!WindowShouldClose()) {
        UpdateMusicStream(music_menu);

        switch (tela_atual) {
            case MENU: {
                if (!IsMusicStreamPlaying(music_menu)) {
                    PlayMusicStream(music_menu);
                }
                BeginDrawing();

                DrawTexture(teladefundo_menu, 0, 0, WHITE); // Desenha a imagem de fundo do menu
                // ClearBackground(RAYWHITE); // Removido para usar a imagem de fundo

                ClearBackground(RAYWHITE);
                DrawText("JOGO DO MILHÃO", SCREEN_WIDTH/2 - MeasureText("JOGO DO MILHÃO", 60)/2, 130, 60, YELLOW);
                
                // botão para iniciar o jogo
                Rectangle botao_start = { SCREEN_WIDTH/2 - 150, 400, 300, 70 };
                DrawRectangleRec(botao_start, YELLOW);
                DrawText("Iniciar Jogo", botao_start.x + 50, botao_start.y + 20, 30, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), botao_start) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (total_perguntas < 15) {
                        printf("Não há perguntas suficientes para iniciar o jogo. São necessárias 15 perguntas.\n");
                       
                    } else {
                        StopMusicStream(music_menu);
                        Iniciar_jogo(&todas_perguntas, &total_perguntas, &perguntas_respondidas_count, &pontuacao_atual, &pontuacao_garantida, &pergunta_atual_idx, &resposta_jogador, &resposta_enviada, &resposta_certa, &pergunta_atual, &perguntas_usadas_indices, &conta_pulo);
                        tela_atual = GAMEPLAY;
                    }
                }

                // o botão de sair do jogo
                Rectangle botao_saida = { SCREEN_WIDTH/2 - 150, 500, 300, 70 };
                DrawRectangleRec(botao_saida, YELLOW);
                DrawText("Sair", botao_saida.x + 110, botao_saida.y + 20, 30, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), botao_saida) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    CloseWindow();
                }

                EndDrawing();
            } break;

            case GAMEPLAY: {
                Atualizar_jogo(&tela_atual, &pergunta_atual, &pergunta_atual_idx, &pontuacao_atual, &pontuacao_garantida, &resposta_jogador, &resposta_enviada, &resposta_certa, todas_perguntas, total_perguntas, perguntas_usadas_indices, sfx_correct, sfx_wrong, &conta_pulo);
                BeginDrawing();

                DrawTexture(teladefundo_gameplay, 0, 0, WHITE); // Desenha a imagem de fundo do gameplay
                // ClearBackground(RAYWHITE); // Removido para usar a imagem de fundo

                Escrever_jogo(pergunta_atual, pontuacao_atual, pontuacao_garantida, pergunta_atual_idx, resposta_enviada, resposta_certa, conta_pulo);
                EndDrawing();
            } break;

            case END: {
                BeginDrawing();

                DrawTexture(teladefundo_end, 0, 0, WHITE); // Desenha a imagem de fundo do fim de jogo
                // ClearBackground(RAYWHITE); // Removido para usar a imagem de fundo

                ClearBackground(RAYWHITE);
                DrawText("FIM DE JOGO!", SCREEN_WIDTH/2 - MeasureText("FIM DE JOGO!", 60)/2, 200, 60, RED);
                DrawText(TextFormat("Sua Premiação final: R$ %d", pontuacao_atual), SCREEN_WIDTH/2 - MeasureText(TextFormat("Sua pontuação final: R$ %d", pontuacao_atual), 30)/2, 300, 30, RED);
                
                //botao de voltar para o menu inicial
                Rectangle botao_voltar_menu = { SCREEN_WIDTH/2 - 150, 900, 300, 70 };
                DrawRectangleRec(botao_voltar_menu, RED);
                DrawText("Voltar ao Menu", botao_voltar_menu.x + 30, botao_voltar_menu.y + 20, 30, BLACK);
                if (CheckCollisionPointRec(GetMousePosition(), botao_voltar_menu) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    tela_atual = MENU;
                }

                EndDrawing();
            } break;
            default: break;
        }
    }//while

    // Descarregar texturas ao fechar a janela
    UnloadTexture(teladefundo_menu);
    UnloadTexture(teladefundo_gameplay);
    UnloadTexture(teladefundo_end);
    UnloadSound(sfx_correct);
    UnloadSound(sfx_wrong);
    UnloadMusicStream(music_menu);
    CloseAudioDevice();

    free(todas_perguntas);
    if (perguntas_usadas_indices != NULL) {
        free(perguntas_usadas_indices);
    }
    CloseWindow();

    return 0;
}


