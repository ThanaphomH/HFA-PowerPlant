DOCKER_IMAGE_NAME=thanaphomh/hpa-powerplant
publish:
	@echo "Building docker image"
	docker build -t $(DOCKER_IMAGE_NAME):latest .
	@echo "Publishing docker image to Docker Hub"
	docker push $(DOCKER_IMAGE_NAME):latest
	@echo "Docker image published to Docker Hub"

pull:
	@echo "Pulling docker image from Docker Hub"
	docker pull $(DOCKER_IMAGE_NAME):latest
	@echo "Docker image pulled from Docker Hub"

check:
	@echo "Checking answers"
	python iscovered.py < check_answer.txt

run:
	python run.py < input.txt
