import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Leer el CSV
df = pd.read_csv("benchmark_threads.csv")

# Filtrar versiones
omp = df[df['version'] == 'omp']
mpi = df[df['version'] == 'mpi']
mpi_omp2 = df[(df['version'] == 'mpi_omp') & (df['procs'] == 2)]
mpi_omp4 = df[(df['version'] == 'mpi_omp') & (df['procs'] == 4)]
mpi_omp8 = df[(df['version'] == 'mpi_omp') & (df['procs'] == 8)]

# Agrupar por número de hilos y calcular promedio
omp_summary = omp.groupby('threads').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()
mpi_summary = mpi.groupby('procs').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()
mpi_omp_summary2 = mpi_omp2.groupby('threads').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()
mpi_omp_summary4 = mpi_omp4.groupby('threads').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()
mpi_omp_summary8 = mpi_omp8.groupby('threads').agg({'speedup':'mean', 'efficiency':'mean'}).reset_index()

# Graficar Speed-up OMP
plt.figure(figsize=(8,5))
plt.plot(omp_summary['threads'], omp_summary['speedup'], marker='o', label='OpenMP')
plt.plot(omp_summary['threads'], omp_summary['threads'], 'k--', label='Ideal')
plt.xlabel('Número de hilos')
plt.ylabel('Speed-up')
plt.title('OMP speed-up')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('omp_speedup.png')
plt.show()

# Graficar Speed-up MPI
plt.figure(figsize=(8,5))
plt.plot(mpi_summary['procs'], mpi_summary['speedup'], marker='o', label='MPI')
plt.plot(mpi_summary['procs'], mpi_summary['procs'], 'k--', label='Ideal')
plt.xlabel('Número de procesos')
plt.ylabel('Speed-up')
plt.title('MPI speed-up')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('mpi_speedup.png')
plt.show()

# Graficar Speed-up Híbrido 2 proc
plt.figure(figsize=(8,5))
plt.plot(mpi_omp_summary2['threads'], mpi_omp_summary2['speedup'], marker='s', label='MPI+OpenMP')
plt.plot(mpi_omp_summary2['threads'], mpi_omp_summary2['threads'], 'k--', label='Ideal')
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Speed-up')
plt.title('Hybrid speed-up (nproc=2)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('hybrid2_speedup.png')
plt.show()

# Graficar Speed-up Híbrido 4 proc
plt.figure(figsize=(8,5))
plt.plot(mpi_omp_summary4['threads'], mpi_omp_summary4['speedup'], marker='s', label='MPI+OpenMP')
plt.plot(mpi_omp_summary4['threads'], mpi_omp_summary4['threads'], 'k--', label='Ideal')
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Speed-up')
plt.title('Hybrid speed-up (nproc=4)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('hybrid4_speedup.png')
plt.show()

# Graficar Speed-up Híbrido 8 proc
plt.figure(figsize=(8,5))
plt.plot(mpi_omp_summary8['threads'], mpi_omp_summary8['speedup'], marker='s', label='MPI+OpenMP')
plt.plot(mpi_omp_summary8['threads'], mpi_omp_summary8['threads'], 'k--', label='Ideal')
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Speed-up')
plt.title('Hybrid speed-up (nproc=8)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('hybrid8_speedup.png')
plt.show()

x=np.linspace(1, 16, 4)
y=np.ones_like(x)
z = np.full_like(x, 0.6)

# Graficar Eficiencia OMP
plt.figure(figsize=(8,5))
plt.plot(omp_summary['threads'], omp_summary['efficiency'], marker='o', label='OpenMP')
plt.plot(x, y)
plt.plot(x, z)
plt.xlabel('Número de hilos')
plt.ylabel('Eficiencia')
plt.title('Eficiencia OMP')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('omp_efficiency.png')
plt.show() 

# Graficar Eficiencia MPI
plt.figure(figsize=(8,5))
plt.plot(mpi_summary['procs'], mpi_summary['efficiency'], marker='o', label='MPI')
plt.plot(x, y)
plt.plot(x, z)
plt.xlabel('Número de procesos')
plt.ylabel('Eficiencia')
plt.title('Eficiencia MPI')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('mpi_efficiency.png')
plt.show() 

# Graficar Eficiencia híbrida 2 proc
plt.figure(figsize=(8,5))
plt.plot(mpi_omp_summary2['threads'], mpi_omp_summary2['efficiency'], marker='s', label='MPI+OpenMP')
plt.plot(x, y)
plt.plot(x, z)
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Eficiencia')
plt.title('Eficiencia híbrida (nproc=2)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('hybrid2_efficiency.png')
plt.show() 

# Graficar Eficiencia híbrida 4 proc
plt.figure(figsize=(8,5))
plt.plot(mpi_omp_summary4['threads'], mpi_omp_summary4['efficiency'], marker='s', label='MPI+OpenMP')
plt.plot(x, y)
plt.plot(x, z)
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Eficiencia')
plt.title('Eficiencia híbrida (nproc=4)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('hybrid4_efficiency.png')
plt.show() 

# Graficar Eficiencia híbrida 8 proc
plt.figure(figsize=(8,5))
plt.plot(mpi_omp_summary8['threads'], mpi_omp_summary8['efficiency'], marker='s', label='MPI+OpenMP')
plt.plot(x, y)
plt.plot(x, z)
plt.xlabel('Número de hilos por proceso')
plt.ylabel('Eficiencia')
plt.title('Eficiencia híbrida (nproc=8)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('hybrid8_efficiency.png')
plt.show() 