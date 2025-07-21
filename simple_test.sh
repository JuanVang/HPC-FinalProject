#!/bin/bash

echo "=== PRUEBA SIMPLE DE CONSISTENCIA ==="
echo

echo "1. VERSIÓN SERIAL (solo primera generación):"
apptainer exec gol.sif /gol_serial 10 10 1 --print 2>/dev/null | grep -A 10 "Generación: 0"

echo
echo "2. VERSIÓN OpenMP (solo primera generación):"
OMP_NUM_THREADS=1 apptainer exec gol.sif /gol_omp 10 10 1 --print 2>/dev/null | grep -A 10 "Generación: 0"

echo
echo "3. VERSIÓN MPI+OpenMP (solo primera generación):"
OMP_NUM_THREADS=1 apptainer exec gol.sif mpirun -np 1 /gol_mpi_omp 10 10 1 --print 2>/dev/null | grep -A 10 "Paso 0" 