# Tutorial: A Quick Start to Profiling and Scaling 

## Objectives: 

_To gain a beginners understanding of:_

* What code __profiling__ is and how to get started
* What code __scaling__ is and how to get started

---

## Profiling

### _Profiling is measuring the performance of your code_


#### _Goal_: identify issues and rectify them so that code runs faster:

<img src="images/allinea_profiling.png" width="50%" />
 
###### Source: Allinea



#### Profiling is part of the coding cycle:


<img src="images/allinea_coding_cycle.png" width="50%" />

###### Source: Allinea

### Incredibly Simple Profiling

Time the program externally with the Linux `time` function (_Example code can be found in `simple_examples` subdirectory_): 

```
module load python
time python external_timing.py
```


* _Pros:_ fast, easy
* _Cons:_ Doesn't provide information about where or why bottlenecks occur
* Use cases 
  * when you already know where a bottleneck is within the code. 
  * code is relatively simple (e.g., a single-file script)

### Simple profiling 

Add timing wrappers around subsections of code (internal) (_Example code can be found in `simple_examples` subdirectory_): 

```
module load python
time python internal_timing.py
```

let's inspect the code...it uses the ___python___ `time` package and some simple calls. 


* _Pros:_ also relatively fast, easy. Helps isolate where bottlenecks occur.
* _Cons:_ Doesn't provide information about why bottlenecks occur
* Use cases: 
  * when you do not know where bottleneck is
  * code is relatively simple 

### Software-based profiling

* Vendor-based software. CURC has:
  * [Arm-Forge](https://developer.arm.com/Tools%20and%20Software/Arm%20Forge) (formerly Allinea)
  * [Intel VTune](https://www.intel.com/content/www/us/en/develop/documentation/vtune-help/top.html) (basic profiling), [Advisor](https://www.intel.com/content/www/us/en/develop/documentation/get-started-with-advisor/top.html) (vectorization/threading), [Trace Analyzer](https://www.intel.com/content/www/us/en/develop/documentation/get-started-with-itac/top.html) (MPI)
* Community-based software that you can download for free, install on your own
  * [GNU Profile](https://ftp.gnu.org/old-gnu/Manuals/gprof-2.9.1/html_mono/gprof.html) (`gprof`)
  * [AMD uprof](https://developer.amd.com/amd-uprof)
  * [HPC Toolkit](http://hpctoolkit.org)
  * Lots of others: [Oprofile](https://oprofile.sourceforge.io/news/), [perf](https://perf.wiki.kernel.org/index.php/Tutorial#Introduction) [TAU](https://www.cs.uoregon.edu/research/tau/home.php), [Scalasca](https://www.scalasca.org)

#### Example

Let's use _Vtune_ to profile an mpi program written in ___c___ called `wave.c`. You'll find all of the files you need in this repository.

Step 1: Login

_Option A:_ If you have a CURC account:
```
ssh -X <yourusername>@login.rc.colorado.edu
```

_Option B:_ If you do not have a CURC account, get a temporary username and password from the instructor:
```
ssh -X userNNNN@tlogin1.rc.colorado.edu
```

Step 2: Start an interactive job with 4 cores. 

```
module load slurm/alpine
sinteractive -N 1 -n 4 -t 60 --reservation=tutorial
```

Step 3: Load the modules you need:

```
module load intel impi vtune
```

Step 4: In the directory where you downloaded `wave.c`, compile it with `make`:

```
make
```

Step 5: If successful, this will result in the executable `wave_c`. Run vtune on the executable:

```
vtune -collect hotspot mpirun -n 4 ./wave_c
```

_this will take about 1 minute and will create a directory called something like r000hs that contains results_

Step 6: Now view the output to identify bottlenecks (requires X11-forwarding)

```
vtune-gui r000hs
```

_which will produce an interactive screen that looks like this:_
 
<img src="images/vtune_map_results_screen.png" width="90%" />

Now, with Vtune graphical user interface (gui), you can find areas of the code that took the longest to run. If you identify bottlenecks, you would then iteratively work to improve them (not covered in this tutorial). 

---

## Scaling
 
### _Scaling is the trial-and-error process of running multiple iterations of a parallel application, with each iteration using a different number of threads (cores), and then comparing the results to determine how the _efficiency_ of the code changes with the number of threads._ 
 
#### Efficiency

##### `Efficiency = Expected Time /  Actual Time` where: 
* _Expected_ ("ideal") time is often taken relative to application’s serial performance, or performance at a low core count.
* _Actual_ time is the result of your scaling run.

##### In the real world, parallel processes are not 100% efficient due to:
* communications overhead
  * “fabric” latency 
  * processes waiting on each other 
* limitations of problem size
* limitations of what parts of the code can be parallelized (Amdahl’s law)

_Therefore, the efficiency will typically decrease as the number of threads increases._
 
##### The "Optimal" efficiency for a particular workflow depends on the context
   * If you have an operational application, say a weather forecast that needs to be produced in a specific timeframe, you'll want to find the optimal efficiency that enables the job to be completed in time for the product deadline.  You may be satisfied to run with a relatively low efficiency as long as the job is done in time. 
   * If you have a research application, whereby you aren't constrained by near-term deadlines, then you typically will strive to achieve the maximum efficiency possible. This will almost always mean running with the *fewest* cores possible while remaining under the wall clock limits on HPC (24 hours typically, on our system)
 
__Rule of thumb:__ >=80% efficiency is pretty good. 
 
##### Types of Scaling

___Strong___

Keep the problem size constant and keep increasing the number of threads (cores).

 <img src="images/strong_scaling.png" width="90%" />

___Weak___

Increase both the problem size and the number of threads (cores) in a consistent manner (e.g., double the problem size if you double the cores).

  <img src="images/weak_scaling.png" width="90%" />


---

### Final items:

* Questions?  Email rc-help@colorado.edu

* Please consider providing feedback on this tutorial via this brief survey: http://tinyurl.com/curc-survey18

* Links to resources that informed this presentation
  * [Intel Vtune docs](https://www.intel.com/content/www/us/en/docs/vtune-profiler/user-guide/2023-0)
  * [SDSC profiling tutorial](https://education.sdsc.edu/training/interactive/hpc_user_training_2021/week7/) 
  * [U. Utah CHPC profiling slides](https://www.chpc.utah.edu/presentations/images-and-pdfs/Profiling20.pdf)
  * [Arm-Forge/Allinea profiling and debugging slides](https://www.alcf.anl.gov/files/Allinea.pdf)

* Acknowledgements
  * Alpine is jointly funded by the University of Colorado Boulder, the University of Colorado Anschutz, Colorado State University, and the National Science Foundation.
  * Blanca is jointly funded by computing users and the University of Colorado Boulder.
  * XDMoD is funded under NSF grant numbers ACI 1025159 and ACI 1445806.
