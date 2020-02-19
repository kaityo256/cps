#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <string>
#include <unistd.h>
#include <vector>

std::vector<std::string> command_list;
std::vector<int> assign_list;
std::vector<std::chrono::system_clock::time_point> start_time;
std::vector<double> ellapsed_time;

double get_time(std::chrono::system_clock::time_point start) {
  auto end = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void manager(const int procs) {
  const int num_tasks = command_list.size();
  assign_list.resize(procs, -1);
  start_time.resize(procs);
  ellapsed_time.resize(num_tasks);
  int task_index = 0;
  auto timer_start = std::chrono::system_clock::now();
  // Distribute Tasks
  while (task_index < num_tasks) {
    MPI_Status st;
    int dummy = 0;
    int isReady = 0;
    for (int i = 1; i < procs && task_index < num_tasks; i++) {
      // Polling
      MPI_Iprobe(i, 0, MPI_COMM_WORLD, &isReady, &st);
      if (!isReady) continue;
      if (assign_list[i] != -1) {
        auto start = start_time[i];
        double elapsed = get_time(start);
        ellapsed_time[assign_list[i]] = elapsed;
        printf("task %d assigned to %d is finished at %f\n", task_index, i, get_time(timer_start));
        assign_list[i] = -1;
      }
      // Assign task_index-th task to the i-th process
      assign_list[i] = task_index;
      start_time[i] = std::chrono::system_clock::now();
      printf("task %d is assignd to %d at %f\n", task_index, i, get_time(timer_start));
      MPI_Recv(&dummy, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &st);
      int len = command_list[task_index].length() + 1;
      MPI_Send(&len, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(command_list[task_index].data(), len, MPI_CHAR, i, 0, MPI_COMM_WORLD);
      task_index++;
    }
  }
  // Complete Notification
  std::vector<int> vf;
  for (int i = 0; i < procs; i++) {
    vf.push_back(false);
  }
  int finish_check = procs - 1;

  while (finish_check > 0) {
    MPI_Status st;
    int dummy = 0;
    int recv = 0;
    int isReady = 0;
    for (int i = 1; i < procs; i++) {
      if (vf[i]) break;
      isReady = false;
      MPI_Iprobe(i, 0, MPI_COMM_WORLD, &isReady, &st);
      if (isReady) {
        MPI_Recv(&recv, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &st);
        MPI_Send(&dummy, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        finish_check--;
        vf[i] = false;
        if (assign_list[i] != -1) {
          auto start = start_time[i];
          auto end = std::chrono::system_clock::now();
          double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
          printf("%d: %f\n", i, elapsed);
          ellapsed_time[assign_list[i]] = elapsed;
          assign_list[i] = -1;
        }
      }
    }
  }
  printf("----------\n");
  for (int i = 0; i < num_tasks; i++) {
    printf("%d: %f\n", i, ellapsed_time[i]);
  }
}

void worker(const int rank) {
  while (true) {
    int send = 10;
    int len = 0;
    MPI_Status st;
    // Sends dummy data to notify that communication is ready.
    MPI_Send(&send, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    // Recieve the length of a command
    MPI_Recv(&len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &st);
    // If the length is zero, all tasks are completed.
    if (len == 0) {
      printf("Finish OK: %d\n", rank);
      break;
    }
    std::unique_ptr<char> buf(new char[len]);
    MPI_Recv(buf.get(), len, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &st);
    std::string recv_string = buf.get();
    printf("%d: Recieved %s\n", rank, recv_string.c_str());
    std::system(recv_string.c_str());
  }
}

// Load commands list from a file
// Success: return 1
// Error  : return 0
int loadfile(int argc, char **argv) {
  int rank, procs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  if (rank != 0) {
    return 1;
  }
  if (argc < 2) {
    std::cout << "Usage: cps commandlist" << std::endl;
    return 0;
  }
  std::string filename = argv[1];
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    std::cerr << "Could not open " << filename << std::endl;
    return 0;
  }
  std::string line;
  while (getline(ifs, line)) {
    if (line.length() > 0 && line[0] == '#') {
      continue;
    }
    command_list.push_back(line);
  }
  return 1;
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank, procs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  int is_ready = loadfile(argc, argv);
  int all_ready = 0;
  MPI_Allreduce(&is_ready, &all_ready, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
  if (all_ready) {
    if (rank == 0) {
      manager(procs);
    } else {
      worker(rank);
    }
  }
  MPI_Finalize();
}
