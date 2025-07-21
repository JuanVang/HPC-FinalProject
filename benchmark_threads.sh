#!/bin/bash

# Parámetros
ROWS=1000
COLS=1000
GENS=100
REPS=3
CSV=benchmark_threads.csv

# Combinaciones óptimas para 8 núcleos físicos
COMBOS="1 8
2 4
4 2
8 1"

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

# 2. OpenMP pura (mismos hilos que en las combinaciones óptimas)
echo "Ejecutando versión OpenMP pura..."
for threads in 8 4 2 1; do
    for rep in $(seq 1 $REPS); do
        t=$(apptainer exec gol.sif /gol_omp $ROWS $COLS $GENS --threads $threads 2>&1 | grep "Tiempo de simulación" | awk '{print $(NF-1)}')
        speedup=$(echo "$T1 / $t" | bc -l)
        efficiency=$(echo "$speedup / $threads" | bc -l)
        echo "omp,1,$threads,$ROWS,$COLS,$GENS,$rep,$t,$speedup,$efficiency" >> $CSV
    done
done

# 3. MPI+OpenMP solo combinaciones óptimas
while read procs threads; do
    if [ -z "$procs" ]; then continue; fi
    echo "Ejecutando: $procs procesos, $threads hilos por proceso..."
    for rep in $(seq 1 $REPS); do
        t=$(apptainer exec gol.sif mpirun -np $procs /gol_mpi_omp $ROWS $COLS $GENS --threads $threads 2>&1 | grep "Tiempo de simulación" | awk '{print $(NF-1)}')
        total_cores=$((procs * threads))
        speedup=$(echo "$T1 / $t" | bc -l)
        efficiency=$(echo "$speedup / $total_cores" | bc -l)
        echo "mpi_omp,$procs,$threads,$ROWS,$COLS,$GENS,$rep,$t,$speedup,$efficiency" >> $CSV
    done
done <<< "$COMBOS"

echo "\n¡Pruebas completadas! Resultados en $CSV" 