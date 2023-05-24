#!/bin/bash
#SBATCH --time=00:00:10
#SBATCH --partition=atesting
#SBATCH --qos=testing
#SBATCH --ntasks=6 
#SBATCH --job-name=cars
#SBATCH --output=cars.%j.out


module purge
module load loadbalance
source /curc/sw/anaconda/default

mpirun lb cmd_file
