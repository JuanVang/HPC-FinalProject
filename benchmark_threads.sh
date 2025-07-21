#!/bin/bash

# Parámetros
ROWS=1000
COLS=1000
GENS=100
REPS=3
CSV=benchmark_threads.csv
MAX_THREADS=16

# Encabezado CSV
echo "version,procs,threads,rows,cols,gens,run,time_sec,speedup,efficiency" > $CSV

# 1. Serial (referencia)
echo "Ejecutando versión serial..."
T1=0
for rep in $(seq 1 $REPS); do
    t=$(apptainer exec gol.sif /gol_serial $ROWS $COLS $GENS 2>&1 | grep "Tiempo de simulación" | awk '{print $(NF-1)}')
    if [ -z "$t" ]; then
        echo "[WARN] Tiempo vacío para serial, rep $rep, saltando..."
        continue
    fi
    echo "serial,1,1,$ROWS,$COLS,$GENS,$rep,$t,1,1" >> $CSV
    T1=$(echo "$T1 + $t" | bc)
done
T1=$(echo "$T1 / $REPS" | bc -l)
echo "Tiempo serial promedio: $T1 s"

# 2. OpenMP pura (threads 1 a 16)
echo "Ejecutando versión OpenMP pura..."
for threads in $(seq 1 $MAX_THREADS); do
    for rep in $(seq 1 $REPS); do
        t=$(apptainer exec gol.sif /gol_omp $ROWS $COLS $GENS --threads $threads 2>&1 | grep "Tiempo de simulación" | awk '{print $(NF-1)}')
        echo "[DEBUG] Tiempo medido para omp con $threads hilos, rep $rep: '$t'"
        if [ -z "$t" ]; then
            echo "[WARN] Tiempo vacío para omp con $threads hilos, rep $rep, saltando..."
            continue
        fi
        speedup=$(echo "$T1 / $t" | bc -l)
        efficiency=$(echo "$speedup / $threads" | bc -l)
        echo "omp,1,$threads,$ROWS,$COLS,$GENS,$rep,$t,$speedup,$efficiency" >> $CSV
    done
done

# 3. MPI+OpenMP con combinaciones balanceadas para 16 hilos totales
echo "Ejecutando versión MPI+OpenMP con combinaciones balanceadas (16 hilos totales)..."
COMBOS="1 16
2 8
4 4
8 2
16 1"
for combo in $COMBOS; do
    set -- $combo
    procs=$1
    threads=$2
    for rep in $(seq 1 $REPS); do
        t=$(apptainer exec gol.sif mpirun -np $procs /gol_mpi_omp $ROWS $COLS $GENS --threads $threads 2>&1 | grep "Tiempo de simulación" | awk '{print $(NF-1)}')
        echo "[DEBUG] Tiempo medido para mpi_omp con $procs procesos, $threads hilos, rep $rep: '$t'"
        if [ -z "$t" ]; then
            echo "[WARN] Tiempo vacío para mpi_omp con $procs procesos, $threads hilos, rep $rep, saltando..."
            continue
        fi
        total_cores=$((procs * threads))
        speedup=$(echo "$T1 / $t" | bc -l)
        efficiency=$(echo "$speedup / $total_cores" | bc -l)
        echo "mpi_omp,$procs,$threads,$ROWS,$COLS,$GENS,$rep,$t,$speedup,$efficiency" >> $CSV
    done
done

echo "\n¡Pruebas completadas! Resultados en $CSV" 