#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VETOR 100
#define TAMANHO_TABELA 13
#define TAMANHO_REGISTRO 134
#define TAMANHO_BUCKET 2

typedef struct metadados {
    int inseridos;
    int lidos;
} METADADOS;

typedef struct segurado {
    char codigo[4];
    char nome[50];
    char seguradora[50];
    char tipo_seguro[30];
} SEGURADO;

typedef struct balde {
    int bucket[TAMANHO_BUCKET];
    int enderecos[TAMANHO_BUCKET];
} CELULA_TABELA_HASH;

void print_hash_table() {
    FILE *fp = fopen("tabela_hash.bin", "rb");

    if (fp == NULL) {
        printf("Erro ao abrir o arquivo da tabela hash!\n");
        exit(1);
    }

    // print hash table in table format
    printf("\nTabela Hash:\n");
    printf("Indice |");
    for (int i = 0; i < TAMANHO_BUCKET; i++) {
        printf("  Slot%d   | Endereco%d |", i + 1, i + 1);
    }
    printf("\n");
    for (int i = 0; i < 60; i++) {
        printf("-");
    }
    printf("\n");

    CELULA_TABELA_HASH bucket;

    for (int i = 0; i < TAMANHO_TABELA; i++) {
        fseek(fp, i * sizeof(CELULA_TABELA_HASH), SEEK_SET);
        fread(&bucket, sizeof(CELULA_TABELA_HASH), 1, fp);

        printf("%6d | ", i);

        for (int j = 0; j < TAMANHO_BUCKET; j++) {
            printf("%8d | %8d | ", bucket.bucket[j], bucket.enderecos[j]);
        }

        printf("\n");
    }

    fclose(fp);
}

int string_to_int(char * str) {
    int numero = 0;
    int sinal_numero = 1;
    int i = 0;

    if (str[0] == '-') {
        sinal_numero = -1;
        i = 1;
    }

    while (str[i] != '\0') {
        if (str[i] >= '0' && str[i] <= '9') {
            numero = numero * 10 + (str[i] - '0');
        } else return 0;
        i++;
    }

    return numero * sinal_numero;
}

int calcular_hash(int chave) {
    return chave % TAMANHO_TABELA;
}

void overflow_progressivo(int hash, int chave, int offset) {
    printf("Executando overflow progressivo...\n");
    FILE * fp = fopen("tabela_hash.bin", "r+b");

    if(fp == NULL) {
        printf("Erro ao abrir o arquivo da tabela hash!\n");
        exit(1);
    }

    int posicao_inicial = hash;
    while (1) {
        if (hash == posicao_inicial) {
            printf("Tabela hash cheia!\n");
            exit(1);
        }

        hash = (hash + 1) % TAMANHO_TABELA;

        CELULA_TABELA_HASH bucket;
        fseek(fp, hash * sizeof(CELULA_TABELA_HASH), SEEK_SET);
        fread(&bucket, sizeof(CELULA_TABELA_HASH), 1, fp);

        int overflow = 1;
        for (int i = 0; i < TAMANHO_BUCKET; i++) {
            printf("Tentando inserir no bucket %d - Indice %d\n", hash, i);
            if(bucket.bucket[i] == -1) {
                bucket.bucket[i] = chave;
                bucket.enderecos[i] = offset;
                overflow = 0;
                break;
            }
            printf("Colisão!\n\n");
        }

        if (overflow) continue;

        fseek(fp, hash * sizeof(CELULA_TABELA_HASH), SEEK_SET);
        fwrite(&bucket, sizeof(CELULA_TABELA_HASH), 1, fp);
        break;
    }
    fclose(fp);
}

void adicionar_chave_tabela(int posicao, int chave, int offset) {
    FILE * fp = fopen("tabela_hash.bin", "r+b");

    if(fp == NULL) {
        printf("Erro ao abrir o arquivo da tabela hash!\n");
        exit(1);
    }

    CELULA_TABELA_HASH bucket;
    fseek(fp, posicao * sizeof(CELULA_TABELA_HASH), SEEK_SET);
    fread(&bucket, sizeof(CELULA_TABELA_HASH), 1, fp);

    int overflow = 1, colisao = 0;
    for (int i = 0; i < TAMANHO_BUCKET; i++) {
        printf("\nTentando inserir no bucket %d - Indice %d\n", posicao, i);
        if(bucket.bucket[i] == -1) {
            bucket.bucket[i] = chave;
            bucket.enderecos[i] = offset;
            overflow = 0;
            break;
        }
        printf("Colisão!\n\n");
    }

    if(overflow) overflow_progressivo(posicao, chave, offset);

    fseek(fp, posicao * sizeof(CELULA_TABELA_HASH), SEEK_SET);
    fwrite(&bucket, sizeof(CELULA_TABELA_HASH), 1, fp);
    fclose(fp);
}

void inicializar_tabela_hash() {
    FILE * fp;
    fp = fopen("tabela_hash.bin", "r+b");

    if(fp == NULL) {
        fp = fopen("tabela_hash.bin", "wb");

        if(fp == NULL) {
            printf("Erro ao criar o arquivo da tabela hash!\n");
            exit(1);
        }

        CELULA_TABELA_HASH bucket_vazio[TAMANHO_TABELA];

        for(int i = 0; i < TAMANHO_TABELA; i++) {
            for(int j = 0; j < TAMANHO_BUCKET; j++) {
                bucket_vazio[i].bucket[j] = -1;
                bucket_vazio[i].enderecos[j] = -1;
            };
        }

        for(int i = 0; i < TAMANHO_TABELA; i++) fwrite(&bucket_vazio[i], sizeof(CELULA_TABELA_HASH), 1, fp);
    }

    fclose(fp);
}

void inserir_tabela_hash(int chave, int offset) {
    FILE * fp = fopen("tabela_hash.bin", "r+b");

    int posicao_tabela = calcular_hash(chave);
    adicionar_chave_tabela(posicao_tabela, chave, offset);

    printf("Chave %d inserida com sucesso!\n\n", chave);

    fclose(fp);
}

SEGURADO * lerArquivoInsere(int * numero_segurados) {
    FILE * fp = fopen("insere.bin", "rb");

    if(fp == NULL) {
        printf("Erro ao abrir o arquivo de inserção!\n");
        exit(1);
    }

    SEGURADO * segurados = (SEGURADO *) malloc(sizeof(SEGURADO) * MAX_VETOR);

    int i = 0;
    while(fread(&segurados[i], sizeof(SEGURADO), 1, fp) == 1) {
        i++;
    }

    fclose(fp);

    *numero_segurados = i;
    return segurados;
}

int * lerArquivoBusca(int * numero_segurados) {
    FILE * fp = fopen("busca.bin", "rb");

    if(fp == NULL) {
        printf("Erro ao abrir o arquivo de busca!\n");
        exit(1);
    }

    int * codigos = (int *) malloc(sizeof(int) * MAX_VETOR);

    char aux[4];
    int i = 0;
    while(1) {
        int lido = fread(aux, sizeof(char), 4, fp);
        if (lido == 0) break;
        codigos[i++] = string_to_int(aux);
    }

    fclose(fp);

    *numero_segurados = i;
    return codigos;
}

void inserir(METADADOS * metadados, SEGURADO * segurados, FILE * fp_metadados, int numero_segurados) {
    // if (metadados.inseridos == numero_segurados) {
    //     printf("Todos os segurados já foram inseridos!\n");
    //     continue;
    // }

    FILE * fp_dados;
    fp_dados = fopen("segurados.bin", "r+b");

    if(fp_dados == NULL) {
        fp_dados = fopen("segurados.bin", "w+b");
    }

    SEGURADO segurado = segurados[metadados->inseridos];

    fseek(fp_dados, 0, SEEK_END);
    int offset = ftell(fp_dados);
    fwrite(&segurado, sizeof(SEGURADO), 1, fp_dados);

    int cod_int = string_to_int(segurado.codigo);
    inserir_tabela_hash(cod_int, offset);

    metadados->inseridos = (metadados->inseridos + 1) % numero_segurados;

    rewind(fp_metadados);
    fwrite(metadados, sizeof(METADADOS), 1, fp_metadados);
    fflush(fp_metadados);

    fclose(fp_dados);
}

void buscar(METADADOS * metadados, FILE * fp_metadados) {
    FILE * fp_hash;
    fp_hash = fopen("tabela_hash.bin", "rb");

    if(fp_hash == NULL) {
        printf("Erro ao abrir o arquivo da tabela hash!\n");
        exit(1);
    }

    int numero_segurados;
    int * codigos = lerArquivoBusca(&numero_segurados);

    // if (metadados->lidos == numero_segurados) {
    //     printf("Todos os segurados já foram buscados!\n");
    //     return;
    // }

    int chave = codigos[metadados->lidos];
    int posicao_tabela = calcular_hash(chave);

    printf("Buscando chave %d...\n", chave);

    int acessos = 0;
    int posicao_inicial = posicao_tabela;
    while (1) {
        if (posicao_tabela == posicao_inicial && acessos > 0) {
            printf("Chave %d não encontrada!\n", chave);

            metadados->lidos = (metadados->lidos + 1) % numero_segurados;

            rewind(fp_metadados);
            fwrite(metadados, sizeof(METADADOS), 1, fp_metadados);
            fflush(fp_metadados);

            return;
        }

        CELULA_TABELA_HASH bucket;
        fseek(fp_hash, posicao_tabela * sizeof(CELULA_TABELA_HASH), SEEK_SET);
        fread(&bucket, sizeof(CELULA_TABELA_HASH), 1, fp_hash);
        acessos++;

        for (int i = 0; i < TAMANHO_BUCKET; i++) {
            if (bucket.bucket[i] == chave) {
                printf("Chave %d encontrada, endereço %d, %d acesso%c!\n", chave, bucket.enderecos[i], acessos, acessos == 1 ? '\0' : 's');

                metadados->lidos = (metadados->lidos + 1) % numero_segurados;

                rewind(fp_metadados);
                fwrite(metadados, sizeof(METADADOS), 1, fp_metadados);
                fflush(fp_metadados);

                return;
            }
        }
        posicao_tabela = (posicao_tabela + 1) % TAMANHO_TABELA;
    }


}

int main() {
    inicializar_tabela_hash();

    FILE * fp_metadados;
    fp_metadados = fopen("metadados.bin", "r+b");

    if (fp_metadados == NULL) {
        fp_metadados = fopen("metadados.bin", "w+b");
        METADADOS metadados;
        metadados.inseridos = 0;
        metadados.lidos = 0;
        fwrite(&metadados, sizeof(METADADOS), 1, fp_metadados);
    }

    rewind(fp_metadados);

    METADADOS metadados;
    fread(&metadados, sizeof(METADADOS), 1, fp_metadados);

    int numero_segurados = 0;
    SEGURADO * segurados = lerArquivoInsere(&numero_segurados);

    int opcao = 0;

    do {
        printf("\nEscolha uma opção:\n");
        printf("1 - Inserir segurado\n");
        printf("2 - Remover segurado\n");
        printf("3 - Buscar segurado\n");
        printf("0 - Sair\n");
        scanf("%d", &opcao);

        if(opcao == 1) {
            inserir(&metadados, segurados, fp_metadados, numero_segurados);
            print_hash_table();
        } else if(opcao == 2) {
            printf("Remover segurado\n");
        } else if(opcao == 3) {
            buscar(&metadados, fp_metadados);
        } else if(opcao == 0) {
            printf("Saindo...\n");
            break;
        } else {
            printf("Opção inválida!\n");
        }
    } while (opcao != 0);
}