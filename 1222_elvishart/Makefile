challenge := elvishart

build:
	docker build -t $(challenge) .

run:
	docker run --rm -ti -p 1222:1222 $(challenge)

uninstall:
	docker rmi $(challenge)

