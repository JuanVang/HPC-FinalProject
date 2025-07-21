import pandas as pd
import matplotlib.pyplot as plt

# Leer el CSV
df = pd.read_csv("benchmark_threads.csv")

# Filtrar versiones
omp = df[df['version'] == 'omp']
mpi_omp = df[(df['version'] == 'mpi_omp') & (df['procs'] == 2)]

# Agrupar por número de hilos y calcular promedio
omp_summary = omp.groupby('threads').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()
mpi_omp_summary = mpi_omp.groupby('threads').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()

# Graficar Speed-up
plt.figure(figsize=(8,5))
plt.plot(omp_summary['threads'], omp_summary['speedup'], marker='o', label='OpenMP')
plt.plot(mpi_omp_summary['threads'], mpi_omp_summary['speedup'], marker='s', label='MPI+OpenMP (np=2)')
plt.plot(omp_summary['threads'], omp_summary['threads'], 'k--', label='Ideal')
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Speed-up')
plt.title('Speed-up vs. Número de hilos')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('speedup.png')
plt.show()

# Graficar Eficiencia
plt.figure(figsize=(8,5))
plt.plot(omp_summary['threads'], omp_summary['efficiency'], marker='o', label='OpenMP')
plt.plot(mpi_omp_summary['threads'], mpi_omp_summary['efficiency'], marker='s', label='MPI+OpenMP (np=2)')
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Eficiencia')
plt.title('Eficiencia vs. Número de hilos')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('efficiency.png')
plt.show() 