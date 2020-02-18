#include <cstdio>
#include <mpi.h>
#include <unistd.h>
#include <vector>

const int DATA = 7;                     //タスクの総数
int data[DATA] = {3, 1, 1, 1, 1, 1, 1}; //タスクの重さ

void manager(const int procs) {
  int count = 0;
  // Distribute Tasks
  while (count < DATA) {
    MPI_Request req;
    MPI_Status st;
    int dummy = 0;
    int isReady = 0;
    for (int i = 1; i < procs && count < DATA; i++) {
      // Polling
      MPI_Iprobe(i, 0, MPI_COMM_WORLD, &isReady, &st);
      if (isReady == 1) {
        // プロセスiが通信準備完了しているので、ダミーデータを受信してから仕事を割り当てる。
        MPI_Recv(&dummy, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &st);
        MPI_Send(&data[count], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
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
    int recv = 0;
    MPI_Status st;
    //通信準備完了を知らせるため、ダミーデータを送信
    MPI_Send(&send, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    //お仕事データを受け取る
    MPI_Recv(&recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &st);
    if (recv == 0) {
      // 0を受け取ったら、お仕事終了と判断
      printf("Finish OK: %d\n", rank);
      break;
    }
    printf("%d: Recieved %d\n", rank, recv);
    usleep(recv * 100000);
  }
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank;
  int procs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  if (rank == 0) {
    manager(procs);
  } else {
    worker(rank);
  }
  MPI_Finalize();
}
