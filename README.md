# Command Processor Scheduler

## Usage

Execute shell commands in parallel.

```sh
mpirun -np num_procs ./cps tasklist
```

For example,

```sh
make
mpirun -np 4 ./cps task.sh
```

The shell commands are given by list.

```sh
$ cat task.sh
# tasks
sleep 1.1
sleep 1.4
sleep 0.9
sleep 1.1
sleep 1.3
sleep 1.2
```

The lines beginning with `#` are considered to be comment.

It will generate a log file `cps.log` after execution.

```sh
$ cat cps.log
Number of tasks : 6
Number of processes : 4
Total execution time: 7.043 [s]
Elapsed time: 2.613 [s]
Parallel Efficiency : 0.898456

Task list:
Command : Elapsed time
sleep 1.1 : 1.11 [s]
sleep 1.4 : 1.408 [s]
sleep 0.9 : 0.907 [s]
sleep 1.1 : 1.106 [s]
sleep 1.3 : 1.308 [s]
sleep 1.2 : 1.204 [s]
```

One process is used for scheduler. Therefore, the number of processes to execute tasks is `num_procs - 1`.
In this case, the total execution time is 7.043 s and the elapsed time (wall time) is 2.613 s. The effective number of processes is 3 (4-1).

Therefore, the parallel efficiency for this case is

7.043 / (2.613 * 3) = 0.8984564357698686 ~ 89.8%

## License

This software is released under the MIT License, see [LICENSE](LICENSE).
