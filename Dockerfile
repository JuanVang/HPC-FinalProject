FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y build-essential openmpi-bin libopenmpi-dev

WORKDIR /app

COPY gol_mpi_omp.cpp .

RUN mpic++ -fopenmp -o game_of_life gol_mpi_omp.cpp

CMD ["mpirun", "-np", "4", "./game_of_life", "8", "8", "5"]

