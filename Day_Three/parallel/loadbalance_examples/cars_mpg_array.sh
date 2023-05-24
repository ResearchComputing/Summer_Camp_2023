#!/bin/bash
#SBATCH --time=00:00:10
#SBATCH --partition=amilan
#SBATCH --qos=normal
#SBATCH --ntasks=1 
#SBATCH --job-name=cars
#SBATCH --output=cars.%A_%a.out
#SBATCH --array=1-5


source /curc/sw/anaconda/default
python cars_mpg.py $(sed -n "${SLURM_ARRAY_TASK_ID}p" cars_mpg_input_args)
