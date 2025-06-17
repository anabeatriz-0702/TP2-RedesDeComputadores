#include "config_rede.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define MAXIMO_CLIENTS 10

/* Alguns trechos de código foram inspirados nas aplicações da Playlist de Programação de Sockets
do Prof. Ítalo Cunha, e serão devidamente sinalizados.
Vídeo-aula disponível em: https://www.youtube.com/watch?v=tJ3qNtv0HVs&list=PLyrH0CFXIM5Wzmbv-lC-qvoBejsa803Qk&ab_channel=%C3%8DtaloCunha*/

time_t timer_coleta = 0; // Timer da coleta zerado, declarado fora da função controle_cliente,
// para que o timer seja compartilhado para todas as threads

int n_clients = 0; // Variável global para o número de clientes conectados
float v_total = 0; // Variável global para o valor acumulado de apostas

void logs_server(const char *event, int id, int n, float v, float m, float me, float payout, float player_profit, float house_profit){

    if (strcmp(event, "start") == 0) {
        printf("event=start | id=%d | N=%d\n", id, n);

    } else if (strcmp(event, "closed") == 0) {
        printf("event=closed | id=%d | N=%d | V=%.2f\n", id, n, v);

    } else if (strcmp(event, "bet") == 0) {
        printf("event=bet | id=%d | N=%d | V=%.2f\n", id, n, v);

    } else if (strcmp(event, "cashout") == 0) {
        printf("event=\n");

    } else if (strcmp(event, "multiplier") == 0) {
        printf("event=\n");

    } else if (strcmp(event, "explode") == 0) {
        printf("event=\n");

    } else if (strcmp(event, "payout") == 0) {
        printf("event=\n");

    } else if (strcmp(event, "profit") == 0) {
        printf("event=\n");

    } else if (strcmp(event, "bye") == 0) {
        printf("event=bye | id=%d\n", id);

    }
}

/*int calculo_explosao(int n_jogadores, float total_apostas){
    // Função auxiliar para calcular o ponto de explosão, com base no número de jogadores
    // e na soma dos valores das apostas feitas
    float m_e = sqrt(1 + n_jogadores + 0.01*total_apostas);
    return m_e;
}

int calculo_profit(int saque, float payout, float bet, float total_apostas){
    // Função auxiliar para calcular o lucro do jogador (player_profit) e da casa (house_profit)
    float player_profit, house_profit;

    if(saque){ // cliente sacou antes da explosão
        player_profit =+ (payout - bet);

    } else{
        player_profit =- bet;
    }
    house_profit == total_apostas - payout;

}*/

struct client_data{
    int client_sock;
    int player_id;
    struct sockaddr_storage addr;
};

void* controle_cliente(void* args) {
    struct client_data *args_client = (struct client_data *)args;

    // Envio da mensagem de start, quando o primeiro cliente se conecta
    struct aviator_msg msg_server, msg_recebida_client;

    time_t timer_corrido = time(NULL);

    if(timer_coleta == 0){ // Quando o primeiro cliente se conectar, timer_coleta == 0 e executa o if. Para os demais clientes, timer_coleta != 0
        timer_coleta = timer_corrido; // Inicia a contagem de tempo
    }

    int timer_restante = 60 - (int)(timer_corrido - timer_coleta); // Calcula quanto tempo resta para aceitação das apostas
    if (timer_restante >= 0){ // Aceitando apostas
            //Chamada da função logs_server: logs_server(event, id, N, V, m ,me, payout, player_profit, house_profit)
            logs_server("start", args_client->player_id, n_clients, -1, -1, -1, -1, -1, -1);

            strncpy(msg_server.type, "start", STR_LEN);
            msg_server.value = timer_restante;
            send(args_client->client_sock, &msg_server, sizeof(msg_server), 0); // Envio da mensagem com o evento (start) e o tempo restante (timer_restante)

            recv(args_client->client_sock, &msg_recebida_client, sizeof(msg_recebida_client) - 1, 0); // Recebimento da mensagem do cliente

        if (strcmp(msg_recebida_client.type, "bet") == 0){ // Se o client apostou, event = bet
            float bet = msg_recebida_client.value; // Valor da aposta é coletado
            v_total += bet; // Valor cumulativo de apostas: v_total é sempre incrementado com bet

            logs_server("bet", args_client->player_id, n_clients, v_total, -1, -1, -1, -1, -1);

            msg_server.value = bet;
            send(args_client->client_sock, &msg_server.value, sizeof(msg_server.value), 0); // Envio do valor para impressão da mensagem "Aposta recebida"

        } else if(strcmp(msg_recebida_client.type, "bye") == 0){ // Se o cliente decidiu sair (Q), event = bye
            logs_server("bye", args_client->player_id, -1, -1, -1, -1, -1, -1, -1);
            close(args_client->client_sock);
        }

    } else{ // nao aceita mais apostas
        logs_server("closed", args_client->player_id, n_clients, v_total, -1, -1, -1, -1, -1);
        strncpy(msg_server.type, "closed", STR_LEN);
        send(args_client->client_sock, &msg_server, sizeof(msg_server), 0);
    }

    free(args_client); // Libera a memória alocada para o cliente
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    if (argc != 3) {
        printf("Entrada para utilização: %s <server IP> <server port>\n", argv[0]);
    }

    struct sockaddr_storage server_addr; // Armazena o endereço do servidor (suporta tanto v4 quanto v6)

    char *protocol = argv[1];
    char *port = argv[2];

    // Configurar um endereço de servidor para determinada rede
    inicializar_addr_server(protocol, port, &server_addr);

    int sock;
    sock = socket(server_addr.ss_family, SOCK_STREAM, 0); // Criação de um socket do tipo SOCK_STREAM

    int opt = 1; // Ativação
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // Permitir reutilização do endereço IP e porta

    struct sockaddr *addr = (struct sockaddr *)(&server_addr); // Faz um cast de sockaddr_storage para sockaddr

    bind(sock, addr, sizeof(server_addr)); // Associa o socket ao endereço do servidor
    listen(sock, MAXIMO_CLIENTS); // Socket em escuta para 10 clientes

    int incr_player_id = 1;

    // Loop aceitando conexões
    while (1) {
        /* Trecho inspirado na aula do Prof. Ítalo Cunha: servidor aceitar conexões do cliente */
        struct sockaddr_storage addr_client; // Armazenar endereço do cliente
        socklen_t addrlen_client = sizeof(addr_client); // Tamanho da estrutura que armazena o endereço do cliente

        int client_sock = accept(sock, (struct sockaddr *)(&addr_client), &addrlen_client);

        pthread_t t_client; // Criação de uma thread para atender este cliente de forma independente
        struct client_data *args_client = malloc(sizeof(*args_client)); // Alocar memória para passar o socket para a thread

        args_client -> client_sock = client_sock;
        args_client -> player_id = incr_player_id++; // Atribui e incrementa o ID
        memcpy(&(args_client -> addr), &addr, sizeof(addr));

        pthread_create(&t_client, NULL, controle_cliente, args_client);
        pthread_detach(t_client);
        n_clients++;
    }

    return 0;

}