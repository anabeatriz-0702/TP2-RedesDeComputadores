all:
# mkdir -> Criar diretório bin
	mkdir -p ./bin 
	gcc -Wall client.c config_rede.c -o ./bin/client
	gcc -Wall -lpthread server.c config_rede.c -o ./bin/server

clean:
	rm -f ./bin/client ./bin/server
