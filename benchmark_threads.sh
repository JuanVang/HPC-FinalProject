#!/bin/bash

# Parámetros
ROWS=840
COLS=840
GENS=500
REPS=5
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

# 3. MPI
echo "Ejecutando versión MPI..."
PROCS_LIST=(1 2 3 4 5 6 7 8)
for i in ${!PROCS_LIST[@]}; do
    procs=${PROCS_LIST[$i]}
    for rep in $(seq 1 $REPS); do
        echo "Comando: apptainer exec gol.sif mpirun -np $procs /gol_mpi $ROWS $COLS $GENS"
        output=$(apptainer exec gol.sif mpirun -np $procs /gol_mpi $ROWS $COLS $GENS 2>&1)
        echo "$output"
        t=$(echo "$output" | grep "Tiempo de simulación" | awk '{print $(NF-1)}')
        echo "[DEBUG] Tiempo medido para mpi con $procs procesos, rep $rep: '$t'"
        if [ -z "$t" ]; then
            echo "[WARN] Tiempo vacío para mpi con $procs procesos, rep $rep, saltando..."
            continue
        fi
        speedup=$(echo "$T1 / $t" | bc -l)
        efficiency=$(echo "$speedup / $procs" | bc -l)
        echo "mpi,$procs,1,$ROWS,$COLS,$GENS,$rep,$t,$speedup,$efficiency" >> $CSV
    done
done

# 4. MPI+OpenMP con combinaciones balanceadas para 16 hilos totales
echo "Ejecutando versión MPI+OpenMP con combinaciones balanceadas (16 hilos totales)..."
PROCS_LIST=(2 4 8)
THREADS_LIST=(1 2 4 6 8 10 12 14 16)
for i in ${!PROCS_LIST[@]}; do
    procs=${PROCS_LIST[$i]}
    for j in ${!THREADS_LIST[@]}; do
        threads=${THREADS_LIST[$j]}
        for rep in $(seq 1 $REPS); do
            echo "Comando: apptainer exec gol.sif mpirun -np $procs /gol_mpi_omp $ROWS $COLS $GENS --threads $threads"
            output=$(apptainer exec gol.sif mpirun -np $procs /gol_mpi_omp $ROWS $COLS $GENS --threads $threads 2>&1)
            echo "$output"
            t=$(echo "$output" | grep "Tiempo de simulación" | awk '{print $(NF-1)}')
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
done

echo "\n¡Pruebas completadas! Resultados en $CSV" 