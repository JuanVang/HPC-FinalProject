import pandas as pd
import matplotlib.pyplot as plt

# Leer el CSV
df = pd.read_csv("benchmark_threads.csv")

# OpenMP: total_threads = threads
omp = df[df['version'] == 'omp'].copy()
omp['total_threads'] = omp['threads']

# MPI+OpenMP: total_threads = procs * threads
mpi_omp = df[df['version'] == 'mpi_omp'].copy()
mpi_omp['total_threads'] = mpi_omp['procs'] * mpi_omp['threads']

# Agrupar por total_threads y calcular promedio
omp_summary = omp.groupby('total_threads').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()
mpi_omp_summary = mpi_omp.groupby('total_threads').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()

# Graficar Speed-up
total_threads = sorted(set(omp_summary['total_threads']).union(mpi_omp_summary['total_threads']))
plt.figure(figsize=(8,5))
plt.plot(omp_summary['total_threads'], omp_summary['speedup'], marker='o', label='OpenMP')
plt.plot(mpi_omp_summary['total_threads'], mpi_omp_summary['speedup'], marker='s', label='MPI+OpenMP')
plt.plot(total_threads, total_threads, 'k--', label='Ideal')
plt.xlabel('Total de hilos (procesos × hilos)')
plt.ylabel('Speed-up')
plt.title('Speed-up vs. Total de hilos')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('speedup_total_threads.png')
plt.show()

# Graficar Eficiencia
plt.figure(figsize=(8,5))
plt.plot(omp_summary['total_threads'], omp_summary['efficiency'], marker='o', label='OpenMP')
plt.plot(mpi_omp_summary['total_threads'], mpi_omp_summary['efficiency'], marker='s', label='MPI+OpenMP')
plt.xlabel('Total de hilos (procesos × hilos)')
plt.ylabel('Eficiencia')
plt.title('Eficiencia vs. Total de hilos')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('efficiency_total_threads.png')
plt.show() 