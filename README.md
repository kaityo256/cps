# Command Processor Scheduler

## Usage

Execute shell commands in parallel.

```sh
mpirun -np num_procs ./cps tasklist
```

## Example

To compile and execute with 4 processes:

```sh
make
mpirun -np 4 ./cps task.sh
```

The list of shell commands should be provided in a file, as shown below:

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

Lines beginning with # are treated as comments and are ignored during execution.

## Output

After execution, a log file named cps.log will be generated:

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

## Parallel Execution Details

One process is reserved for scheduling, so the number of processes available for executing tasks is `num_procs - 1`.

In the example above:

* The total execution time is 7.043 seconds.
* The elapsed time (wall time) is 2.613 seconds.
* The effective number of worker processes is 4 - 1 = 3.


Therefore, the parallel efficiency is calculated as:

```python
7.043 / (2.613 * 3) = 0.898456 ≈ 89.8%
```

## License

This software is released under the MIT License, see [LICENSE](LICENSE).
