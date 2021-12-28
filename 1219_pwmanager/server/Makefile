challenge := server

build:
	docker build -t $(challenge) .

run:
	mkdir -p ./ads/
	chmod a+rwx ./ads/
	docker run --rm -ti -v $$PWD/ads:/opt/pwmanager/server/ads.dir/ads -p 5580:5580 $(challenge)

clean:
	rm -rf ./ads/ *~
uninstall:
	docker rmi $(challenge)

