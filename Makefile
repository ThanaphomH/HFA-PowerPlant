DOCKER_IMAGE_NAME=thanaphomh/hpa-powerplant-solver
INPUT_FILE=grid-12-17
OUTPUT_FILE=grid-12-17.out

publish:
	@echo "Building docker image"
	docker build -t $(DOCKER_IMAGE_NAME) .
	@echo "Publishing docker image to Docker Hub"
	docker push $(DOCKER_IMAGE_NAME)
	@echo "Docker image published to Docker Hub"

pull:
	@echo "Pulling docker image from Docker Hub"
	docker pull $(DOCKER_IMAGE_NAME)
	@echo "Docker image pulled from Docker Hub"

check:
	@echo "Checking answers"
	python iscovered.py < check_answer.txt

run-docker:
	docker run --rm -v ./data/input:/input  -v ./data/output:/output ${DOCKER_IMAGE_NAME} /input/{INPUT_FILE} /output/{OUTPUT_FILE}

# time docker run --rm -v ./data/input:/input  -v ./data/output:/output thanaphomh/hpa-powerplant-solver /input/grid-12-17 /output/grid-12-17.out

build-docker:
	docker build --platform=linux/amd64 -t ${DOCKER_IMAGE_NAME} .


TRY_DOCKER_NAME=

run-other:
	docker pull ${TRY_DOCKER_NAME}
	time docker run --rm -v ./data/input:/input  -v ./data/output:/output ${TRY_DOCKER_NAME} /input/ring-25000-25000 /output/ring-25000-25000.out
