/*------------------------------------------------------------------------
 *
 * Adapted for use as a profiling example from the original code found at:
 * http://www.new-npac.org/projects/cdroms/cewes-1999-06-vol2/cps615course
 * 
 * This program implements the concurrent wave equation described  
 * in Chapter 5 of Fox et al., 1988, Solving Problems on Concurrent
 * Processors, vol 1.   
 *
 * A vibrating string is decomposed into points.  Each task is  
 * responsible for updating the amplitude of a number of points over
 * time.
 *       
 * At each iteration, each task exchanges boundary points with
 * nearest neighbors.  This version uses low level sends and receives
 * to exchange boundary points.
 *
 *-----------------------------------------------------------------------*/

/* Increase or decrease these to change the total running time
 * and the balance of computation vs communication.
 * The defaults are fine for a quad-core workstation */
#define MAXPOINTS 1000000
/* default to 30 seconds duration; this can be overridden on the command line */
#define DURATION 30

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "mpi.h"
#define PI 3.14159265

int E_RtoL = 10;
int E_LtoR = 20;
int E_OUT1 = 30;
int E_OUT2 = 40;

void get_data(void);
void init_line(void);
int update (int left, int right);
void output_master(void);
void output_workers(void);
MPI_Request request;
MPI_Status status;
int rank,                  /* rank */
    ntask,                   /* number of tasks */
    tpoints,                 /* total points along string */
    npoints,                 /* number of points handled by this task */
    first,                   /* index of 1st point handled by this task */
    rcode,                   /* generic return code */
    runtime;                 /* number of seconds to run for */
double values[MAXPOINTS+2],  /* values at time t */
       oldval[MAXPOINTS+2],  /* values at time (t-dt) */
       newval[MAXPOINTS+2];  /* values at time (t+dt) */

struct timespec tv1, start, end, finish_time;
unsigned long long communication_usec = 0;

void time_mpi_start()
{
    clock_gettime(CLOCK_MONOTONIC, &tv1);
}

void time_mpi_stop()
{
    struct timespec tv2;

    clock_gettime(CLOCK_MONOTONIC, &tv2);
    communication_usec += (tv2.tv_sec - tv1.tv_sec) * 1000000
                        + (tv2.tv_nsec - tv1.tv_nsec) / 1000;
}

/*  ------------------------------------------------------------------------
 *      Obtain input values from user
 *  ------------------------------------------------------------------------ */

void get_data(void)
{
    char tchar[8];
    int buffer[2];

    if (rank == 0)
    {
        tpoints = MAXPOINTS;
        printf("points = %d, running for %d seconds\n", tpoints, runtime);
        buffer[0] = tpoints;
    }

    /* broadcast number of points */
    time_mpi_start();
    MPI_Bcast(buffer, 1, MPI_INT, 0, MPI_COMM_WORLD);
    time_mpi_stop();
    tpoints = buffer[0];
}

/*  ------------------------------------------------------------------------
 *     Initialize points on line
 *  --------------------------------------------------------------------- */

void init_line(void)
{
    int nmin, nleft, npts, i, j, k;
    double x, fac;

    /* calculate initial values based on sine curve */
    nmin = tpoints/ntask;
    nleft = tpoints%ntask;
    fac = 2.0 * PI;
    for (i = 0, k = 0; i < ntask; i++)
    {
        npts = (i < nleft) ? nmin + 1 : nmin;
        if (rank == i)
        {
            first = k + 1;
            npoints = npts;

            for (j = 1; j <= npts; j++, k++)
            {
                x = (double)k/(double)(tpoints - 1);
                values[j] = sin (fac * x);
            }  
        }
        else
            k += npts;
    }
    for (i = 1; i <= npoints; i++)
        oldval[i] = values[i];
}

/*---------------------------------------------------------------------
 *      Calculate new values using wave equation
 *---------------------------------------------------------------------*/

void do_math(int i)
{
    const double dtime = 0.3;
    const double c = 1.0;
    const double dx = 1.0; 
    double tau, sqtau;

    tau = (c * dtime / dx);
    sqtau = tau * tau;
    newval[i] = (2.0 * values[i]) - oldval[i]  
        + (sqtau * (values[i-1] - (2.0 * values[i]) + values[i+1]));
}


void reduce_print(const char *format_string, int value)
{
    int min_val, sum_val, max_val;
    MPI_Reduce(&value, &min_val, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&value, &max_val, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&value, &sum_val, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0)
        printf(format_string, min_val, sum_val / ntask, max_val);
}

/*---------------------------------------------------------------------
 *      All tasks update their points a specified number of times  
 *---------------------------------------------------------------------*/

int update(int left, int right)
{
    int i, j;
    int iterations = 0;

    /* update values for each point along string */
    while (1)
    {
        int stop;
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (rank == 0)
        {
            stop = now.tv_sec  >= finish_time.tv_sec && 
                   now.tv_nsec >= finish_time.tv_nsec;
        }
        time_mpi_start();
        MPI_Bcast(&stop, 1, MPI_INT, 0, MPI_COMM_WORLD);
        time_mpi_stop();
        if (stop) break;

        for(i=0;i<10;++i)
        {
            iterations += 1;

            time_mpi_start();
            /* Exchange data with "left-hand" neighbor */
            if (first != 1) {
                MPI_Send(&values[1], 1, MPI_DOUBLE, left, E_RtoL, MPI_COMM_WORLD);
                MPI_Recv(&values[0], 1, MPI_DOUBLE, left, E_LtoR, MPI_COMM_WORLD,
                        &status);
            }
            /* Exchange data with "right-hand" neighbor */
            if (first + npoints -1 != tpoints) {
                MPI_Send(&values[npoints], 1, MPI_DOUBLE, right, E_LtoR, MPI_COMM_WORLD);
                MPI_Recv(&values[npoints+1], 1, MPI_DOUBLE, right, E_RtoL,
                        MPI_COMM_WORLD, &status);
            }
            time_mpi_stop();

            /* update points along line */
            for (j = 1; j <= npoints; j++)
            {
                /* global endpoints */
                if ((first + j - 1 == 1) || (first + j - 1 == tpoints))
                    newval[j] = 0.0;
                else
                    do_math(j); 
            }
            for (j = 1; j <= npoints; j++)
            {
                oldval[j] = values[j];
                values[j] = newval[j];
            }
        }
    }

    return iterations;
}

/*------------------------------------------------------------------
 *      Master receives results from workers and prints
 *------------------------------------------------------------------ */

void output_master(void)  
{
    int i, start, npts, buffer[2], istep, source;
#if defined (MPC_MODULE_MPC_MPI)
    /* MPC can have problems with stack overflow on single core machines, so use
       heap allocation here */
    double* results;
    results = (double*)malloc(MAXPOINTS * sizeof(double));
#else
    double results[MAXPOINTS];
#endif
    time_mpi_start();
    /* store worker's results in results array */
    for (i = 1; i < ntask; i++)
    {
        /* receive number of points and first point */
        MPI_Recv(buffer, 2, MPI_INT, MPI_ANY_SOURCE, E_OUT1,
                MPI_COMM_WORLD, &status);

        start = buffer[0];
        npts = buffer[1];

        /* receive results */
        source = status.MPI_SOURCE;
        MPI_Recv(&results[start-1], npts, MPI_DOUBLE, source,
                E_OUT2, MPI_COMM_WORLD, &status);
    }
    time_mpi_stop();

    /* store master's results in results array */
    for (i = first; i < first + npoints; i++)
        results[i-1] = values[i];

    istep = (tpoints <= 5) ? 1: tpoints/5;
#ifdef PRINT_VALIDATION
    printf ("\nPoints for validation:\n");
    for (i = 0; i < tpoints; i+=istep)
        printf ("%d:%4.2f  ", i, results[i]);
    if (i-istep != tpoints - 1) 
        printf ("%d:%4.2f  ", tpoints-1, results[tpoints-1]);
    printf("\n");
#endif
#if defined (MPC_MODULE_MPC_MPI)
    free(results);
#endif
}

/*----------------------------------------------------------------
 *      Workers send the updated values to the master
 *----------------------------------------------------------------*/

void output_workers(void)
{
    int buffer[2];

    /* send first point and number of points handled to master */
    buffer[0] = first;
    buffer[1] = npoints;
    time_mpi_start();
    MPI_Isend(buffer, 2, MPI_INT, 0, E_OUT1, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);

    /* send results to master */
    MPI_Isend(&values[1], npoints, MPI_DOUBLE, 0, E_OUT2, MPI_COMM_WORLD,
            &request);
    MPI_Wait(&request, &status);
    time_mpi_stop();
}

/*------------------------------------------------------------------
 *      Main program
 *------------------------------------------------------------------*/

int main(int argc, char **argv)
{
    int left, right, i, iterations;
    unsigned long long allt;

    /* learn number of tasks and rank in MPI_COMM_WORLD */
    rcode = MPI_Init(&argc, &argv);

    clock_gettime(CLOCK_MONOTONIC, &start);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &ntask);
    if (rcode >= 0 && rank == 0) printf ("Wave solution with %d processes\n", ntask);

    /* determine left and right neighbors */
    if (rank == ntask-1) right = MPI_PROC_NULL;
    else right = rank + 1;
    if (rank == 0) left = MPI_PROC_NULL;
    else left = rank - 1;

    if (argc > 1 && atoi(argv[1]) > 0)
        runtime = atoi(argv[1]);
    else
        runtime = DURATION;

    clock_gettime(CLOCK_MONOTONIC, &finish_time);
    finish_time.tv_sec += runtime;

    /* get program parameters and initialize wave values */
    get_data();
    init_line();

    /* update values along the line for nstep time steps */
    iterations = update(left, right);

    /*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*/
    /* collect results and print */

    if (rank == 0) output_master();
    else output_workers();

    time_mpi_start();
    MPI_Barrier(MPI_COMM_WORLD);
    time_mpi_stop();

    clock_gettime(CLOCK_MONOTONIC, &end);

    allt = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    if (rank == 0) printf("\n               min\tmean\tmax\n");
    reduce_print("All time (ms): %d\t%d\t%d\n", (int)(allt/1000.0 + 0.5));
    reduce_print("CPU time (ms): %d\t%d\t%d\n", (int)((allt - communication_usec)/1000.0 + 0.5));
    reduce_print("MPI time (ms): %d\t%d\t%d\n", (int)(communication_usec/1000.0 + 0.5));
    if (rank == 0) printf("\n");
    reduce_print("CPU time (%) : %d\t%d\t%d\n", (int)(100.0 * (allt - communication_usec) / allt + 0.5));
    reduce_print("MPI time (%) : %d\t%d\t%d\n", (int)(100.0 * communication_usec          / allt + 0.5));
    if (rank == 0) printf("\n");
    reduce_print("Iterations   : %d\t%d\t%d\n", iterations);
    double calculation_rate = ((double)tpoints / (double)allt) * iterations; /* in million points per second */
    if (rank == 0) printf("points/second: %.1fM (%.1fM per process)\n", calculation_rate, calculation_rate / ntask);

    if (rank == 0) printf("wave finished\n");
    MPI_Finalize();
    return(0);
}
