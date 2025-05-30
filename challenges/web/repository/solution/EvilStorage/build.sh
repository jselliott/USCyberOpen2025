docker build -t evil-storage .
CONTAINER=$(docker run -d evil-storage)
docker cp "${CONTAINER}:/opt/target/EvilStorage-1.0.0.jar" ../files/EvilStorage.jar
docker kill "${CONTAINER}"