#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//define a estrutura Pergunta
typedef struct Pergunta{
    char enunciado[256];
    char alternativas[4][128];
    char alternativaCorreta;
    int dificuldade;
} Pergunta;

void carregarPerguntasDoCSV(const char *nomeArquivo, Pergunta **colecao, int *numPerguntas){
    FILE *arquivo = fopen(nomeArquivo, "r"); //tenta abrir o arquivo para leitura
    if(arquivo == NULL){
        printf("Erro ao abrir o arquivo %s. Nenhuma pergunta pre-carregada.\n", nomeArquivo); // ver se conseguiu validar
        return;
    }

    char linha[1024];//buffer para ler cada linha do CSV
    char *token;//tokens
    Pergunta tempPergunta;//variável temporária para montar a pergunta antes de adicionar
    int linhaLida = 0;//varivel para contar linha lida

    printf("\nCarregando perguntas do arquivo %s...\n", nomeArquivo);
    fgets(linha, sizeof(linha), arquivo);

    while(fgets(linha, sizeof(linha), arquivo) != NULL){
        linhaLida++;
        linha[strcspn(linha, "\n")] = 0;

        *colecao = (Pergunta *)realloc(*colecao, (*numPerguntas + 1) * sizeof(Pergunta));
        if(*colecao == NULL){
            printf("Erro ao alocar memória para carregar perguntas do CSV! Carregamento interrompido.\n");
            fclose(arquivo);
            return;
        }

        //variavel usada para validar se entrou no if
        int valido = 1;

        token = strtok(linha, ",");
        if(token != NULL){
            strncpy(tempPergunta.enunciado, token, sizeof(tempPergunta.enunciado) - 1);
            tempPergunta.enunciado[sizeof(tempPergunta.enunciado) - 1] = '\0';
        } else{
            valido = 0; //validador 
        }

        for(int i = 0; i < 4; i++){
            if(valido) {
                token = strtok(NULL, ",");
                if(token != NULL){
                    strncpy(tempPergunta.alternativas[i], token, sizeof(tempPergunta.alternativas[i]) - 1);
                    tempPergunta.alternativas[i][sizeof(tempPergunta.alternativas[i]) - 1] = '\0';
                } else{
                    valido = 0; //validador
                }
            }
        }

        if(valido){
            token = strtok(NULL, ",");
            if(token != NULL){
                tempPergunta.alternativaCorreta = toupper(token[0]);
            }else{
                valido = 0; // validador
            }
        }

        if(valido){
            token = strtok(NULL, ",");
            if(token != NULL){
                tempPergunta.dificuldade = atoi(token);
                //converte string para inteiro

                if (tempPergunta.dificuldade < 1 || tempPergunta.dificuldade > 5) {
                    printf("Aviso: Dificuldade invalida na linha %d do CSV. Definindo para 3 (Medio).\n", linhaLida);
                    tempPergunta.dificuldade = 3;
                    //define um valor padrão se for inválido
                }
            }else{
                valido = 0;  //validador

            }
        }

        if(valido){
            (*colecao)[*numPerguntas] = tempPergunta;
            (*numPerguntas)++;
        }else{
            //se a linha foi inválida, avisamos e precisamos desfazer a alocação extra
            printf("Aviso: Erro de formato na linha %d do CSV. Ignorando esta linha.\n", linhaLida);
            if(*numPerguntas > 0){ //se já havia perguntas válidas, redimensiona para o tamanho anterior
                 *colecao = (Pergunta *)realloc(*colecao, (*numPerguntas) * sizeof(Pergunta));
                 if(*colecao == NULL && *numPerguntas > 0){
                     printf("Erro critico ao realocar memória apos erro de linha no CSV.\n");
                     fclose(arquivo);
                     return;
                 }
            }else{ //se não havia perguntas e a primeira linha foi inválida
                free(*colecao);
                *colecao = NULL;
            }
        }
    }

    fclose(arquivo);
    printf("Carregamento concluido. %d perguntas carregadas.\n", *numPerguntas);
}

int main(){
    int opcao;
    Pergunta *colecaoDeTodasAsPerguntas = NULL;
    int quantidadeDePerguntas = 0;

    carregarPerguntasDoCSV("bancoperguntas.csv", &colecaoDeTodasAsPerguntas, &quantidadeDePerguntas);


    do{
        //menu das perguntas
        printf("\nMENU DE GERENCIAMENTO DE PERGUNTAS\n");
        printf("1. Cadastrar nova pergunta\n");
        printf("2. Listar todas as perguntas\n");
        printf("3. Pesquisar pergunta\n");
        printf("4. Alterar pergunta\n");
        printf("5. Excluir pergunta\n");
        printf("0. Sair\n");

        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        //switch das cases(opções do menu)
        switch(opcao){
            case 1: {
                printf("\n--- CADASTRO DE PERGUNTA ---\n");

                colecaoDeTodasAsPerguntas = (Pergunta *)realloc(colecaoDeTodasAsPerguntas, (quantidadeDePerguntas + 1) * sizeof(Pergunta));
                if(colecaoDeTodasAsPerguntas == NULL){
                    printf("Erro ao alocar memoria para a nova pergunta!\n");
                    break;
                }

                Pergunta novaPergunta;

                printf("Enunciado da pergunta: ");
                getchar();
                fgets(novaPergunta.enunciado, sizeof(novaPergunta.enunciado), stdin);
                novaPergunta.enunciado[strcspn(novaPergunta.enunciado, "\n")] = 0;

                for(int i = 0; i < 4; i++){
                    printf("Alternativa %c: ", 'A' + i);
                    fgets(novaPergunta.alternativas[i], sizeof(novaPergunta.alternativas[i]), stdin);
                    novaPergunta.alternativas[i][strcspn(novaPergunta.alternativas[i], "\n")] = 0;
                }

                do{
                    printf("Letra da alternativa correta (A, B, C ou D): ");
                    scanf(" %c", &novaPergunta.alternativaCorreta);
                    novaPergunta.alternativaCorreta = toupper(novaPergunta.alternativaCorreta);
                    if(novaPergunta.alternativaCorreta < 'A' || novaPergunta.alternativaCorreta > 'D'){
                        printf("Opcao invalida. Por favor, digite A, B, C ou D.\n");
                    }
                } while(novaPergunta.alternativaCorreta < 'A' || novaPergunta.alternativaCorreta > 'D');

                do{
                    printf("Nivel de dificuldade (1 - Muito Facil, 2 - Facil, 3 - Medio, 4 - Dificil, 5 - Muito Dificil): ");
                    scanf("%d", &novaPergunta.dificuldade);
                    if(novaPergunta.dificuldade < 1 || novaPergunta.dificuldade > 5){
                        printf("Nivel de dificuldade invalido. Digite um valor entre 1 e 5.\n");
                    }
                } while(novaPergunta.dificuldade < 1 || novaPergunta.dificuldade > 5);

                //adiciona a nova pergunta ao final do array(vetor) dinâmico
                colecaoDeTodasAsPerguntas[quantidadeDePerguntas] = novaPergunta;
                quantidadeDePerguntas++; //incrementa o contador total de perguntas
                printf("Pergunta cadastrada com sucesso!\n");
                break;
            }

            case 2: {
                if(quantidadeDePerguntas == 0){
                    printf("\nNenhuma pergunta cadastrada ainda.\n");
                    break;
                }
                    // mostrar as perguntas
                printf("\n--- LISTA DE PERGUNTAS ---\n");
                for(int i = 0; i < quantidadeDePerguntas; i++){
                    printf("\nPergunta %d:\n", i + 1);
                    printf("Enunciado: %s\n", colecaoDeTodasAsPerguntas[i].enunciado);
                    for(int j = 0; j < 4; j++){
                        printf("  %c) %s\n", 'A' + j, colecaoDeTodasAsPerguntas[i].alternativas[j]);
                    }
                    printf("Alternativa Correta: %c\n", colecaoDeTodasAsPerguntas[i].alternativaCorreta); // CORRIGIDO AQUI
                    printf("Dificuldade: %d\n", colecaoDeTodasAsPerguntas[i].dificuldade);
                }
                break;
            }

            case 3: {
                if (quantidadeDePerguntas == 0) {
                    printf("\nNenhuma pergunta cadastrada para pesquisar.\n");
                    break;
                }
                    //pesquisar pergunta
                char termoPesquisa[100];
                printf("\n--- PESQUISAR PERGUNTA ---\n");
                printf("Digite o termo para pesquisar no enunciado: ");
                getchar();
                fgets(termoPesquisa, sizeof(termoPesquisa), stdin);
                termoPesquisa[strcspn(termoPesquisa, "\n")] = 0;

                int encontradas = 0;
                for (int i = 0; i < quantidadeDePerguntas; i++){
                    if(strstr(colecaoDeTodasAsPerguntas[i].enunciado, termoPesquisa) != NULL){
                        printf("\n--- PERGUNTA ENCONTRADA (ID: %d) ---\n", i + 1);
                        printf("Enunciado: %s\n", colecaoDeTodasAsPerguntas[i].enunciado);
                        for(int j = 0; j < 4; j++){
                            printf("  %c) %s\n", 'A' + j, colecaoDeTodasAsPerguntas[i].alternativas[j]);
                        }
                        printf("Alternativa Correta: %c\n", colecaoDeTodasAsPerguntas[i].alternativaCorreta);
                        printf("Dificuldade: %d\n", colecaoDeTodasAsPerguntas[i].dificuldade);
                        encontradas++; //verificar se encontrou algo
                    }
                }

                if(encontradas == 0){
                    printf("Nenhuma pergunta encontrada com o termo '%s'.\n", termoPesquisa);
                }
                break;
            }

            case 4: {
                if(quantidadeDePerguntas == 0){
                    printf("\nNenhuma pergunta cadastrada para alterar.\n");
                    break;
                }

                int idPergunta;
                //lista as perguntas para o usuário escolher qual alterar
                printf("\nLISTA DE PERGUNTAS\n");
                for(int i = 0; i < quantidadeDePerguntas; i++){
                    printf("\nPergunta %d:\n", i + 1);
                    printf("Enunciado: %s\n", colecaoDeTodasAsPerguntas[i].enunciado);

                    for(int j = 0; j < 4; j++){
                        printf("  %c) %s\n", 'A' + j, colecaoDeTodasAsPerguntas[i].alternativas[j]);
                    }
                    printf("Alternativa Correta: %c\n", colecaoDeTodasAsPerguntas[i].alternativaCorreta);
                    printf("Dificuldade: %d\n", colecaoDeTodasAsPerguntas[i].dificuldade);
                }

                printf("\nALTERAR PERGUNTA\n");
                printf("Digite o ID da pergunta que deseja alterar (1 a %d): ", quantidadeDePerguntas);
                scanf("%d", &idPergunta);

                if(idPergunta < 1 || idPergunta > quantidadeDePerguntas){
                    printf("ID de pergunta invalido!\n");
                    break;
                }

                int indice = idPergunta - 1;

                printf("\nEditando Pergunta %d:\n", idPergunta);
                printf("Enunciado atual: %s\n", colecaoDeTodasAsPerguntas[indice].enunciado);
                printf("Novo enunciado (deixe em branco para manter o atual): ");
                getchar();
                char novoEnunciado[256];
                fgets(novoEnunciado, sizeof(novoEnunciado), stdin);
                novoEnunciado[strcspn(novoEnunciado, "\n")] = 0;
                if(strlen(novoEnunciado) > 0){
                    strcpy(colecaoDeTodasAsPerguntas[indice].enunciado, novoEnunciado);
                }

                for(int i = 0; i < 4; i++){
                    printf("Alternativa %c atual: %s\n", 'A' + i, colecaoDeTodasAsPerguntas[indice].alternativas[i]);
                    printf("Nova alternativa %c (deixe em branco para manter a atual): ", 'A' + i);
                    char novaAlternativa[128];
                    fgets(novaAlternativa, sizeof(novaAlternativa), stdin);
                    novaAlternativa[strcspn(novaAlternativa, "\n")] = 0;
                    if(strlen(novaAlternativa) > 0){
                        strcpy(colecaoDeTodasAsPerguntas[indice].alternativas[i], novaAlternativa);
                    }
                }

                printf("Alternativa correta atual: %c\n", colecaoDeTodasAsPerguntas[indice].alternativaCorreta);
                char novaAlternativaCorreta;
                do{
                    printf("Nova letra da alternativa correta (A, B, C, D ou 0 para manter): ");
                    scanf(" %c", &novaAlternativaCorreta);
                    novaAlternativaCorreta = toupper(novaAlternativaCorreta);
                    if(novaAlternativaCorreta == '0'){
                        break;
                    }
                    if(novaAlternativaCorreta < 'A' || novaAlternativaCorreta > 'D'){
                        printf("Opcao invalida. Por favor, digite A, B, C, D ou 0.\n");
                    }
                } while(novaAlternativaCorreta < 'A' || novaAlternativaCorreta > 'D');

                if(novaAlternativaCorreta != '0'){
                    colecaoDeTodasAsPerguntas[indice].alternativaCorreta = novaAlternativaCorreta;
                }

                printf("Dificuldade atual: %d\n", colecaoDeTodasAsPerguntas[indice].dificuldade);
                int novaDificuldade;
                do{
                    printf("Novo nivel de dificuldade (1-5 ou 0 para manter): ");
                    scanf("%d", &novaDificuldade);
                    if(novaDificuldade == 0){
                        break;
                    }
                    if(novaDificuldade < 1 || novaDificuldade > 5){
                        printf("Nivel de dificuldade invalido. Digite um valor entre 1 e 5 ou 0.\n");
                    }
                } while(novaDificuldade < 1 || novaDificuldade > 5);

                if(novaDificuldade != 0){
                    colecaoDeTodasAsPerguntas[indice].dificuldade = novaDificuldade;
                }

                printf("Pergunta alterada com sucesso!\n");
                break;
            }

            case 5: { 
                if(quantidadeDePerguntas == 0){
                    printf("\nNenhuma pergunta cadastrada para excluir.\n");
                    break;
                }

                int idPergunta;
                //lista as perguntas para o usuário escolher qual excluir
                printf("\nLISTA DE PERGUNTAS\n");
                for(int i = 0; i < quantidadeDePerguntas; i++){
                    printf("\nPergunta %d:\n", i + 1);
                    printf("Enunciado: %s\n", colecaoDeTodasAsPerguntas[i].enunciado);

                    for(int j = 0; j < 4; j++){
                        printf("  %c) %s\n", 'A' + j, colecaoDeTodasAsPerguntas[i].alternativas[j]);
                    }
                    printf("Alternativa Correta: %c\n", colecaoDeTodasAsPerguntas[i].alternativaCorreta);
                    printf("Dificuldade: %d\n", colecaoDeTodasAsPerguntas[i].dificuldade);
                }

                printf("\nEXCLUIR PERGUNTA\n");
                printf("Digite o ID da pergunta que deseja excluir (1 a %d): ", quantidadeDePerguntas);
                scanf("%d", &idPergunta);

                if(idPergunta < 1 || idPergunta > quantidadeDePerguntas){
                    printf("ID de pergunta invalido!\n");
                    break;
                }

                int indice = idPergunta - 1;

                //move as perguntas que virão para preencher o espaço da pergunta excluída
                for(int i = indice; i < quantidadeDePerguntas - 1; i++){
                    colecaoDeTodasAsPerguntas[i] = colecaoDeTodasAsPerguntas[i + 1];
                }

                quantidadeDePerguntas--;//decrementa o contador de perguntas
                colecaoDeTodasAsPerguntas = (Pergunta *)realloc(colecaoDeTodasAsPerguntas, quantidadeDePerguntas * sizeof(Pergunta));
                if(quantidadeDePerguntas == 0){
                    free(colecaoDeTodasAsPerguntas);
                    colecaoDeTodasAsPerguntas = NULL;
                } else if(colecaoDeTodasAsPerguntas == NULL){
                    printf("Erro ao realocar memória apos exclusao.\n");
                }

                printf("Pergunta excluida com sucesso!\n");
                break;
            }

            case 0:
                printf("\nSaindo do programa. Ate mais!\n");
                break;
            default:
                printf("\nOpcao invalida. Por favor, tente novamente.\n");
                break;
        }
    } while(opcao != 0);

    //libera a memória alocada
    free(colecaoDeTodasAsPerguntas);
    colecaoDeTodasAsPerguntas = NULL;

    return 0;
}