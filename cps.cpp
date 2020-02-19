#include <cstdio>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <string>
#include <unistd.h>
#include <vector>

std::vector<std::string> command_list;

void manager(const int procs) {
  const int num_tasks = command_list.size();
  int count = 0;
  // Distribute Tasks
  while (count < num_tasks) {
    MPI_Status st;
    int dummy = 0;
    int isReady = 0;
    for (int i = 1; i < procs && count < num_tasks; i++) {
      // Polling
      MPI_Iprobe(i, 0, MPI_COMM_WORLD, &isReady, &st);
      if (isReady == 1) {
        // プロセスiが通信準備完了しているので、ダミーデータを受信してから仕事を割り当てる。
        MPI_Recv(&dummy, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &st);
        int len = command_list[count].length() + 1;
        MPI_Send(&len, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Send(command_list[count].data(), len, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        count++;
      }
    }
    usleep(100);
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
      }
      usleep(100);
    }
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
