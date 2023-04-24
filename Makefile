


.PHONY: help build \
	docker_start docker_stop \
	docker_image_create docker_image_remove \
	docker_container_create docker_container_remove \
	docker_init \
	all

help:
	@echo ""
	@echo "== Ostentus Makefile help =="
	@echo ""
	@echo "Common usage:"
	@echo \
		'\tmake docker_init|run once to prepare Docker image and container\n'\
		'\tmake|default command will output firmware.uf2 in this directory'\
		| column -s "|" -t
	@echo ""
	@echo "All available commands:"
	@awk '/^#/{c=substr($$0,3);next}c&&/^[[:alpha:]][[:alnum:]_-]+:/{print "\t"substr($$1,1,index($$1,":")),c}1{c=0}' $(MAKEFILE_LIST) | column -s: -t
	@echo ""

.DEFAULT_GOAL := all

# Build inside the Docker container and output to ./firmware.uf2
build:
	@docker exec -i ostentus bash < utility/build_firmware.sh

# Start the ostentus docker container
docker_start:
	@docker start ostentus

# Stop the ostentus docker container
docker_stop:
	@docker stop ostentus

# Create the ostentus_build docker image
docker_image_create:
	@docker build -t ostentus_build .

# Remove the ostentus_build docker image
docker_image_remove:
	@docker image rm ostentus_build


# Create the ostentus docker container (and build the cross compiler)
docker_container_create:
	@docker run -dt \
	 --user $(shell id -u):$(shell id -g) \
	 --name ostentus \
	 --mount type=bind,source=$(shell pwd)/,target=/ostentus \
	 ostentus_build
	@docker exec -w /ostentus/submodules/micropython ostentus make -C mpy-cross

# Remove the ostentus docker container
docker_container_remove:
	@docker rm ostentus

# Create ostentus_build image and use it to create the ostentus container
docker_init: docker_image_create docker_container_create

# Clear all ostenus Docker files (Stop the container, remove the container, remove the image)
docker_fullclean: docker_stop docker_container_remove docker_image_remove

# Start container, build firmware, stop container
all: docker_start build docker_stop
