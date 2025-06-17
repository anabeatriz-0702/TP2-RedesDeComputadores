#include "config_rede.h"

#include <netdb.h>
#include <string.h>
#include <stdio.h>

/* Alguns trechos de código foram inspirados nas aplicações da Playlist de Programação de Sockets
do Prof. Ítalo Cunha, e serão devidamente sinalizados. 
Vídeo-aula disponível em: https://www.youtube.com/watch?v=tJ3qNtv0HVs&list=PLyrH0CFXIM5Wzmbv-lC-qvoBejsa803Qk&ab_channel=%C3%8DtaloCunha */

// Função para configurar o endereço de rede (v4 ou v6) em uma estrutura do tipo sockaddre_storage
int configura_addr(const char *str_addr, const char *porta_str, struct sockaddr_storage *server_addr){

    struct addrinfo tipo_protocolo, *resultados_gerados;
    memset(&tipo_protocolo, 0, sizeof(tipo_protocolo));
    tipo_protocolo.ai_family = AF_UNSPEC;  // Campo ai_family como AF_UNSPEC (aceita IPv4 e IPv6)
    tipo_protocolo.ai_socktype = SOCK_STREAM;  // Tipo de socket (TCP)

    getaddrinfo(str_addr, porta_str, &tipo_protocolo, &resultados_gerados);  // getaddrinfo resolve str_addr e porta_str com os critérios de tipo_protocolo,
    // e preenche a lista de resultados em resultados_gerados.

    memcpy(server_addr, resultados_gerados->ai_addr, resultados_gerados->ai_addrlen);  // Cópia do endereço resolvido
    freeaddrinfo(resultados_gerados);  // Libera a memória alocada por getaddrinfo
    return 0;
}

// Função para configurar um endereço de servidor para determinada rede 
/* Trecho inspirado na aula do Prof. Ítalo Cunha */
int inicializar_addr_server(const char *protocolo, const char* porta_str, struct sockaddr_storage *server_addr){
    
    uint16_t porta_int = htons((uint16_t)atoi(porta_str)); 

    memset(server_addr, 0, sizeof(*server_addr)); // Zera estrutura de armazenamento evitar lixo de memória

    // Configura endereço para IPv4
    if(strcmp(protocolo, "v4") == 0){ // compara se as strings são iguais
        struct sockaddr_in *addr_v4 = (struct sockaddr_in *)server_addr; 
        addr_v4 -> sin_family = AF_INET; // Definição da família do protocolo (AF_INET)
        addr_v4 -> sin_addr.s_addr = INADDR_ANY; // Configura o endereço IP como qualquer
        addr_v4 -> sin_port = porta_int; // Definição da porta do servidor 
        return 0;    
    } 
    
    // Configura endereço para IPv6
    else if (strcmp(protocolo, "v6") == 0){
        struct sockaddr_in6 *addr_v6 = (struct sockaddr_in6 *)server_addr;
        addr_v6 -> sin6_family = AF_INET6;
        addr_v6 -> sin6_addr = in6addr_any;
        addr_v6 -> sin6_port = porta_int;
        return 0;
    } 

    return -1; // Protocolo inválido
}