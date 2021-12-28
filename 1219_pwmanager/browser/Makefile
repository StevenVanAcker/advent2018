challenge := browser

build:
	docker build -t $(challenge) .

run:
	docker run --rm -ti -v $$PWD:/data $(challenge)

uninstall:
	docker rmi $(challenge)

