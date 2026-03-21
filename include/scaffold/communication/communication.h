#ifndef SCAFFOLD_COMMUNICATION_H_
#define SCAFFOLD_COMMUNICATION_H_

#if USE_MPI
  #include <mpi.h>
#endif
#if USE_OPENMP
  #include <omp.h>
#endif
#include "mpi_datatype.h"

namespace scaffold { namespace parallel {

namespace COMM {
  #ifdef USE_MPI // Parallel version
    inline void Init (int argc, char **argv)
    {
      MPI_Init(&argc, &argv);
      int proc;
      MPI_Comm_rank(MPI_COMM_WORLD, &proc);
    }

    inline void Finalize() { MPI_Finalize(); }
    inline void BarrierSync() { MPI_Barrier(MPI_COMM_WORLD); }

    // Return process number
    inline int WorldProc()
    {
      int proc;
      MPI_Comm_rank(MPI_COMM_WORLD, &proc);
      return (proc);
    }

  #else // Serial version
    inline void Init (int argc, char **argv) {}
    inline void Finalize () {}
    inline int WorldProc() {return (0);}
  #endif
}

class Communicator
{
public:

  Communicator()
  {
    SetWorld();
  }

#if USE_MPI // Parallel version

  MPI_Comm MPIComm;

  /// Sets this communicator to be that of all the processes (i.e. MPI_WORLD)
  void SetWorld()
  {
    MPIComm = MPI_COMM_WORLD;
  }

  int MyProc()
  {
    int rank;
    MPI_Comm_rank(MPIComm, &rank);
    return rank;
  }

  int NumProcs()
  {
    int n_procs;
    MPI_Comm_size(MPIComm, &n_procs);
    return n_procs;
  }

  std::string MyHost()
  {
    int len;
    char hostname[MPI_MAX_PROCESSOR_NAME];
    MPI_Get_processor_name(hostname, &len);
    return hostname;
  }

  void BarrierSync()
  {
    MPI_Barrier(MPIComm);
  }

  void Split(int color, Communicator &new_comm)
  {
    MPI_Comm_split(MPIComm, color, 0, &(new_comm.MPIComm));
  }

  void Subset(matrix::vec<int> &ranks, Communicator &new_comm)
  {
    MPI_Group my_group, new_group;
    MPI_Comm_group (MPIComm, &my_group);
  #ifdef USE_ARMADILLO
    MPI_Group_incl(my_group, ranks.size(), ranks.memptr(), &new_group);
  #endif
  #ifdef USE_EIGEN
    MPI_Group_incl(my_group, ranks.size(), ranks.data(), &new_group);
  #endif
    MPI_Comm_create(MPIComm, new_group, &(new_comm.MPIComm));
  }

  /** Gathers vectors from subprocesses and puts them in a matrix
   * @param to_proc ID for receiving processor
   * @param from_buff Reference to sending vector
   * @param to_buff Reference to receiving matrix
   * return the MPI status
   */
  template <typename T>
  int AllGatherCols(matrix::mat<T> &buff)
  {
    int n_procs = NumProcs();
    int my_proc = MyProc();
  #ifdef USE_ARMADILLO
    int rows = buff.n_rows;
    int cols = buff.n_cols;
  #endif
  #ifdef USE_EIGEN
    int rows = buff.rows();
    int cols = buff.cols();
  #endif
    int displacements[n_procs];
    int receive_counts[n_procs];
    int sendCount;
    void *sendBuf, *receiveBuf;
    int curr_col = 0;
    for (int proc=0; proc<n_procs; proc++) {
      int proc_cols = cols/n_procs + ((cols%n_procs)>proc);
      displacements[proc] = rows*curr_col;
      receive_counts[proc] = rows*proc_cols;
      if (proc == my_proc) {
        sendBuf = &(buff(0,curr_col));
        sendCount = rows*proc_cols;
      }
      curr_col += proc_cols;
    }
  #ifdef USE_ARMADILLO
    receiveBuf = buff.memptr();
  #endif
  #ifdef USE_EIGEN
    receiveBuf = buff.data();
  #endif
    return MPI_Allgatherv(sendBuf, sendCount, MPI_DOUBLE, receiveBuf, receive_counts, displacements, MPI_DOUBLE, MPIComm);
  }

  // Send
  template<class T>
  inline int Send(int to_proc, T &val)
  {
    return MPI_Send(MPITypeTraits<T>::GetAddr(val), MPITypeTraits<T>::GetSize(val), MPITypeTraits<T>::GetType(val), to_proc, 0, MPIComm);
  }

  // Receive
  template<class T>
  inline int Receive(int from_proc, T &val)
  {
    return MPI_Recv(MPITypeTraits<T>::GetAddr(val), MPITypeTraits<T>::GetSize(val), MPITypeTraits<T>::GetType(val), from_proc, 0, MPIComm, MPI_STATUS_IGNORE);
  }

  // Sendrecv
  template<class T>
  inline int SendReceive (int from_proc, T &from_buff, int to_proc, T &to_buff)
  {
    return MPI_Sendrecv(MPITypeTraits<T>::GetAddr(from_buff), MPITypeTraits<T>::GetSize(from_buff), MPITypeTraits<T>::GetType(from_buff), from_proc, 1, MPITypeTraits<T>::GetAddr(to_buff), MPITypeTraits<T>::GetSize(to_buff), MPITypeTraits<T>::GetType(to_buff), to_proc, 1, MPIComm, MPI_STATUS_IGNORE);
  }

  // Broadcast
  template<class T>
  inline int Broadcast(int from_proc, T &val)
  {
    return MPI_Bcast(MPITypeTraits<T>::GetAddr(val), MPITypeTraits<T>::GetSize(val), MPITypeTraits<T>::GetType(val), from_proc, MPIComm);
  }

  // Reduce
  template<class T>
  inline int Reduce(int to_proc, T &from_buff, T &to_buff, MPI_Op Op)
  {
    return MPI_Reduce(MPITypeTraits<T>::GetAddr(from_buff), MPITypeTraits<T>::GetAddr(to_buff), MPITypeTraits<T>::GetSize(from_buff), MPITypeTraits<T>::GetType(from_buff), Op, to_proc, MPIComm);
  }

  // AllReduce
  template<class T>
  inline int AllReduce(T &from_buff, T &to_buff, MPI_Op Op)
  {
    return MPI_Allreduce(MPITypeTraits<T>::GetAddr(from_buff), MPITypeTraits<T>::GetAddr(to_buff), MPITypeTraits<T>::GetSize(from_buff), MPITypeTraits<T>::GetType(from_buff), Op, MPIComm);
  }

  // Sum
  template<class T>
  inline int Sum(int to_proc, T &from_buff, T &to_buff) { return Reduce(to_proc,from_buff,to_buff,MPI_SUM); }

  // AllSum
  template<class T>
  inline int AllSum(T &from_buff, T &to_buff) { return AllReduce(from_buff,to_buff,MPI_SUM); }

  // Product
  template<class T>
  inline int Product(int to_proc, T &from_buff, T &to_buff) { return Reduce(to_proc,from_buff,to_buff,MPI_PROD); }

  // AllProduct
  template<class T>
  inline int AllProduct(T &from_buff, T &to_buff) { return AllReduce(from_buff,to_buff,MPI_PROD); }

  // Gather
  template<class T>
  inline int Gather(int to_proc, T &from_buff, T &to_buff)
  {
    return MPI_Gather(MPITypeTraits<T>::GetAddr(from_buff), MPITypeTraits<T>::GetSize(from_buff), MPITypeTraits<T>::GetType(from_buff), MPITypeTraits<T>::GetAddr(to_buff), MPITypeTraits<T>::GetSize(from_buff), MPITypeTraits<T>::GetType(to_buff), to_proc, MPIComm);
  }

  // Gatherv
  template<class T>
  inline int Gatherv(int to_proc, T &from_buff, T &to_buff, int* recvCounts, int* displacements)
  {
     return MPI_Gatherv(MPITypeTraits<T>::GetAddr(from_buff), MPITypeTraits<T>::GetSize(from_buff), MPITypeTraits<T>::GetType(from_buff), MPITypeTraits<T>::GetAddr(to_buff), recvCounts, displacements, MPITypeTraits<T>::GetType(to_buff), to_proc, MPIComm);
  }

  // GatherCols
  template<class T>
  int GatherCols(int to_proc, T &from_buff, T &to_buff)
  {
    int n_procs = NumProcs();
    int my_proc = MyProc();
#ifdef USE_ARMADILLO
    int rows = from_buff.n_rows;
    int cols = from_buff.n_cols;
#elif defined USE_EIGEN
    int rows = from_buff.rows();
    int cols = from_buff.cols();
#endif
    int displacements[n_procs];
    int recvCounts[n_procs];
    int curr_col = 0;
    for (int proc=0; proc<n_procs; proc++) {
      int proc_cols = cols/n_procs + ((cols%n_procs)>proc);
      displacements[proc] = rows*curr_col;
      recvCounts[proc] = rows*proc_cols;
      if (proc == my_proc) {
        to_buff.set_size(rows,proc_cols);
      }
      curr_col += proc_cols;
    }
    return Gather(to_proc, from_buff, to_buff, recvCounts, displacements);
  }

  // AllGather
  template<class T>
  inline int AllGather(T &from_buff, T &to_buff)
  {
    return MPI_Gather(MPITypeTraits<T>::GetAddr(from_buff), MPITypeTraits<T>::GetSize(from_buff), MPITypeTraits<T>::GetType(from_buff), MPITypeTraits<T>::GetAddr(to_buff), MPITypeTraits<T>::GetSize(from_buff), MPITypeTraits<T>::GetType(to_buff), MPIComm);
  }

  // Scatter
  template<class T>
  inline int Scatter(int from_proc, T &from_buff, T &to_buff)
  {
    return MPI_Scatter(MPITypeTraits<T>::GetAddr(from_buff), MPITypeTraits<T>::GetSize(from_buff), MPITypeTraits<T>::GetType(from_buff), MPITypeTraits<T>::GetAddr(to_buff), MPITypeTraits<T>::GetSize(to_buff), MPITypeTraits<T>::GetType(to_buff), from_proc, MPIComm);
  }

  // Scatter
  template<class T>
  inline int Scatterv(int from_proc, T &from_buff, T &to_buff, int* send_counts, int* displacements)
  {
     return MPI_Scatterv(MPITypeTraits<T>::GetAddr(from_buff), send_counts, displacements, MPITypeTraits<T>::GetType(from_buff), MPITypeTraits<T>::GetAddr(to_buff), MPITypeTraits<T>::GetSize(to_buff), MPITypeTraits<T>::GetType(to_buff), from_proc, MPIComm);
  }

  // ScatterCols
  template<class T>
  int ScatterCols(int from_proc, T &from_buff, T &to_buff)
  {
    int n_procs = NumProcs();
    int my_proc = MyProc();
#ifdef USE_ARMADILLO
    int rows = from_buff.n_rows;
    int cols = from_buff.n_cols;
#elif defined USE_EIGEN
    int rows = from_buff.rows();
    int cols = from_buff.cols();
#endif
    int displacements[n_procs];
    int send_counts[n_procs];
    int curr_col = 0;
    for (int proc=0; proc<n_procs; proc++) {
      int proc_cols = cols/n_procs + ((cols%n_procs)>proc);
      displacements[proc] = rows*curr_col;
      send_counts[proc] = rows*proc_cols;
      if (proc == my_proc) {
        to_buff.set_size(rows,proc_cols);
      }
      curr_col += proc_cols;
    }
    return Scatterv(from_proc, from_buff, to_buff, send_counts, displacements);
  }

#else   // Serial version
  inline void SetWorld(){}
  inline int MyProc() {return 0;}
  inline int NumProcs() {return 1;}
  inline std::string MyHost() {return "0";}
  inline void BarrierSync() {}
  inline void Split(int color, Communicator &new_comm) {}
  inline void Subset(matrix::vec<int> &ranks, Communicator &new_comm)
  {
    if (ranks.size() != 1) {
      std::cerr << "Serial verion of code does not support nontrivial subsets. Exiting.\n";
      abort();
    }
  }

  template<class T>
  inline int Send(int to_proc, T &val) {}
  template<class T>
  inline int Receive(int from_proc, T &val) {}
  template<class T>
  inline int SendReceive (int from_proc, T &from_buff, int to_proc, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int Broadcast(int from_proc, T &val) {}
  template<class T>
  inline int Sum(int to_proc, T &from_buff, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int AllSum(T &from_buff, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int Product(int to_proc, T &from_buff, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int AllProduct(T &from_buff, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int Gather(int to_proc, T &from_buff, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int Gatherv(int to_proc, T &from_buff, T &to_buff, int* recvCounts, int* displacements) {to_buff = from_buff;}
  template<class T>
  inline int GatherCols(int to_proc, T &from_buff, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int AllGather(T &from_buff, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int Scatter(int from_proc, T &from_buff, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int Scatterv(int from_proc, T &from_buff, T &to_buff, int* send_counts, int* displacements) {to_buff = from_buff;}
  template<class T>
  inline int ScatterCols(int from_proc, T &from_buff, T &to_buff) {to_buff = from_buff;}
  template<class T>
  inline int AllGatherCols (matrix::mat<T> &buff) {}
#endif
};

}}

#endif // SCAFFOLD_COMMUNICATION_H_
