#!/bin/bash

# Parámetros
ROWS=1000
COLS=1000
GENS=100
PROCS=4
THREADS_LIST="1 2 4 8 16"
REPS=3
CSV=benchmark_threads.csv

# Encabezado CSV
echo "version,procs,threads,rows,cols,gens,run,time_sec,speedup,efficiency" > $CSV

# 1. Serial (referencia)
echo "Ejecutando versión serial..."
T1=0
for rep in $(seq 1 $REPS); do
    t=$(apptainer exec gol.sif /gol_serial $ROWS $COLS $GENS 2>&1 | grep "Tiempo de simulación" | awk '{print $(NF-1)}')
    echo "serial,1,1,$ROWS,$COLS,$GENS,$rep,$t,1,1" >> $CSV
    T1=$(echo "$T1 + $t" | bc)
done
T1=$(echo "$T1 / $REPS" | bc -l)
echo "Tiempo serial promedio: $T1 s"

# 2. MPI+OpenMP variando hilos
echo "Ejecutando versión híbrida MPI+OpenMP con $PROCS procesos..."
for threads in $THREADS_LIST; do
    for rep in $(seq 1 $REPS); do
        t=$(apptainer exec gol.sif mpirun -np $PROCS /gol_mpi_omp $ROWS $COLS $GENS --threads $threads 2>&1 | grep "Tiempo de simulación" | awk '{print $(NF-1)}')
        speedup=$(echo "$T1 / $t" | bc -l)
        efficiency=$(echo "$speedup / ($PROCS * $threads)" | bc -l)
        echo "mpi_omp,$PROCS,$threads,$ROWS,$COLS,$GENS,$rep,$t,$speedup,$efficiency" >> $CSV
    done
done

echo "\n¡Pruebas completadas! Resultados en $CSV" 