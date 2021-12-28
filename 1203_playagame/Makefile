challenge := playagame

build:
	docker build -t $(challenge) .

run:
	docker run --rm -ti -p 1203:1203 $(challenge)

uninstall:
	docker rmi $(challenge)

