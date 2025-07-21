#!/bin/bash

echo "=== PRUEBA DE CONSISTENCIA DE INICIALIZACIÓN ==="
echo "Comparando tableros iniciales de las 3 versiones..."
echo

# Parámetros de prueba
ROWS=10
COLS=10
GENS=1

echo "Configuración: ${ROWS}x${COLS}, ${GENS} generación"
echo

echo "1. VERSIÓN SERIAL:"
apptainer exec gol.sif /gol_serial ${ROWS} ${COLS} ${GENS} --print 2>/dev/null | head -${ROWS}

echo
echo "2. VERSIÓN OpenMP:"
OMP_NUM_THREADS=1 apptainer exec gol.sif /gol_omp ${ROWS} ${COLS} ${GENS} --print 2>/dev/null | head -${ROWS}

echo
echo "3. VERSIÓN MPI+OpenMP:"
OMP_NUM_THREADS=1 apptainer exec gol.sif mpirun -np 1 /gol_mpi_omp ${ROWS} ${COLS} ${GENS} --print 2>/dev/null | head -${ROWS}

echo
echo "=== FIN DE PRUEBA ===" 