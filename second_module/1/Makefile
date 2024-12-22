.SILENT:

build:
	gcc -o chat chat.c

run:
	gcc -o chat chat.c && ./chat 8080

docker:
	docker build -t ubuntu-chat .

docker-network:
	docker network create --driver bridge --subnet 192.168.0.0/16 network-for-chat

docker-run:
	docker run --network=network-for-chat --rm -ti ubuntu-chat

help:
	echo "build - build the app via gcc"
	echo "run - start app with default 8080 port"
	echo "docker - build the app via docker, see README"
	echo "docker-network - create network for docker with name: network-for-chat"
	echo "docker-run - launch 1 docker container"
