#include "config_rede.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

/* Alguns trechos de código foram inspirados nas aplicações da Playlist de Programação de Sockets
do Prof. Ítalo Cunha, e serão devidamente sinalizados.
Vídeo-aula disponível em: https://www.youtube.com/watch?v=tJ3qNtv0HVs&list=PLyrH0CFXIM5Wzmbv-lC-qvoBejsa803Qk&ab_channel=%C3%8DtaloCunha*/

void tratamento_args(int argc, char **argv){

    int found_nick = 0; // Flag para verificar se -nick foi encontrado

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-nick") == 0) {
            found_nick = 1;

            if(found_nick){
                if(argv[i] != argv[3]){
                found_nick = 0;
                }
            }
            break;
        }
    }

    if (!found_nick) {
        printf("Error: Expected '-nick' argument\n");
        exit(EXIT_FAILURE);
    }

    if (argc != 5) {
        printf("Error: Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }

    if (strlen(argv[4]) > 13) {
        printf("Error: Nickname too long (max 13)\n");
        exit(EXIT_FAILURE);
    }
}

int tratamento_jogo(char *entrada_start){

    if (strcasecmp(entrada_start, "Q") == 0) { // Escolheu sair
        return 0;
    }

    // Conversão da entrada string para um número float
    char *auxptr;
    float valor = strtof(entrada_start, &auxptr);

    if (*auxptr != '\0') {
        printf("Error: Invalid command\n");
        return -1;

    } else if(valor <= 0){
        printf("Error: Invalid bet value\n");
        return -1;

    } else{ // Aposta -> valor numérico
        return 1;
    }
}

int main(int argc, char **argv) {

    tratamento_args(argc, argv);

    char *server_ip = argv[1];
    char *port = argv[2];
    char *apelido = argv[4];

    struct sockaddr_storage server_addr;
    configura_addr(server_ip, port, &server_addr);

    int sock = socket(server_addr.ss_family, SOCK_STREAM, 0);
    connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr));

    struct aviator_msg msg_recebida_server, msg_client;
    recv(sock, &msg_recebida_server, sizeof(msg_recebida_server), 0); // Recebimento de mensagens vindas do servidor

    char entrada_start[STR_LEN], entrada_closed[STR_LEN];
    int validador = -1;

    while (validador == -1) {

        if (strcmp(msg_recebida_server.type, "start") == 0) {
            printf("Rodada aberta! Digite o valor da aposta ou digite [Q] para sair (%d segundos restantes):", (int)msg_recebida_server.value);
            scanf("%9s", entrada_start);
            validador = tratamento_jogo(entrada_start);

        } 
    }

    if (validador == 1) { // Se a aposta for válida
        strncpy(msg_client.type, "bet", STR_LEN);
        msg_client.value = strtof(entrada_start, NULL); // O valor da aposta
        send(sock, &msg_client, sizeof(msg_client), 0);

        recv(sock, &msg_recebida_server.value, sizeof(msg_recebida_server.value), 0);
        float bet = msg_recebida_server.value;
        printf("Aposta recebida: %.2f R$\n", bet);

    } else if (validador == 0) { // Se o usuário digitou 'Q' para sair
        strncpy(msg_client.type, "bye", STR_LEN);
        send(sock, &msg_client, sizeof(msg_client), 0);

        printf("Aposte com responsabilidade. A plataforma é nova e tá com horário bugado. Volte logo, %s.\n", apelido);
        close(sock);
    }

    if (strcmp(msg_recebida_server.type, "closed") == 0) {
        printf("Apostas encerradas! Não é mais possível apostar nesta rodada.\n");
        if(validador == 1){ // Pessoa apostou
            printf("Digite C para sacar:");
            scanf("%9s", entrada_closed);
            if (strcasecmp(entrada_closed, "C") == 0) { // Escolheu sacar
                printf("Escolheu sacar.\n");
                
            } else {
                printf("Error: Invalid command\n");
            }
        }
    }
    return 0;
}