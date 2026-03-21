#ifndef SCAFFOLD_TESTS_H
#define SCAFFOLD_TESTS_H

#include <scaffold.h>
#include <iostream>
#include <assert.h>

using namespace scaffold::matrix;
using namespace scaffold::algorithm;
using namespace scaffold::parallel;
using namespace scaffold::io;
using namespace scaffold::rand;

class Simulation
{
public:
  // Constructor
#if USE_MPI
  Simulation() : rng((int)time(0)*(COMM::WorldProc()+1)) {}
#else
  Simulation() : rng((int)time(0)) {}
#endif

  // Simulation Random Number Generator
  RNG rng;

  // Inputs
  Input in;
  IO out;
  void SetupIO(std::string in_file);

  // MPI setup
  Communicator world_comm; // This is the global MPIWORLD communicator.
  Communicator inter_comm; // This is for communication between the rank 0 procs of each walker group.
  Communicator intra_comm; // This is for commmunication between procs within a walker group.
  void BuildMPIModel();
  int procs_per_group;

  // Algorithm
  void Run() { Tests(); };

  // Tests
  void Tests() { HDF5Tests(); MPITests(); };
  void HDF5Tests();
  void MPITests();
  void ReturnSync() { world_comm.BarrierSync(); };
  void TestInverse(Communicator &my_comm);
  void TestAllGatherCols(Communicator &my_comm);
  void TestBroadcast(Communicator &my_comm);
  void TestSendReceive(Communicator &my_comm);
  void TestSendrecv(Communicator &my_comm);
  void TestSums(Communicator &my_comm);

};

void Simulation::SetupIO(std::string in_file)
{
  // Input
  in.Load(in_file);

  // Output
  std::stringstream tmp_ss;
  tmp_ss << in.GetChild("IO").GetAttribute<std::string>("output_prefix") << ".h5";
  std::string output = tmp_ss.str();
  out.Load(output);
  out.Create();
}

void Simulation::BuildMPIModel()
{
  procs_per_group = in.GetChild("Parallel").GetAttribute<int>("procs_per_group");
  int N = world_comm.NumProcs();
  assert ((N % procs_per_group) == 0 && (procs_per_group > 1));
  int n_groups = N/procs_per_group;
  int my_group = world_comm.MyProc()/procs_per_group;
  world_comm.Split(my_group, intra_comm);
  vec<int> ranks(n_groups);
  for (int group=0; group<n_groups; group++)
    ranks(group) = group*procs_per_group;
  world_comm.Subset(ranks, inter_comm);
  int n_threads = 1;
#if USE_OPENMP
  #pragma omp parallel
  {
    n_threads = omp_get_num_threads();
  }
#endif
  if (world_comm.MyProc() == 0)
    std::cout <<"# Processes: "<<N<< ", # Groups: "<<n_groups
              <<", # Processes/Group: "<<procs_per_group
              <<", # Threads/Process: "<<n_threads<<std::endl;
}

void Simulation::HDF5Tests()
{
  // HDF5 Tests
  if (world_comm.MyProc() == 0) {
    std::cout <<"Writing to HDF5 file..."<<std::endl;
    mat<double> A = random<mat<double>>(2,2);
    std::cout << A << std::endl;
    out.Write("doubleArray", A);
    std::cout <<"Reading from HDF5 file..."<<std::endl;
    mat<double> B = random<mat<double>>(2,2);
    out.Read("doubleArray", B);
    std::cout << B << std::endl;
  }
  world_comm.BarrierSync(); // Sync procs within each clone.

  if (world_comm.MyProc() == 0) {
    std::cout <<"Creating extendable dataset..."<<std::endl;
    out.CreateGroup("Data");
    mat<double> A = random< mat<double> >(3,2);
    std::cout << A << std::endl;
    out.CreateExtendableDataSet("/Data/", "test", A);
    std::cout <<"Reading extendable dataset..."<<std::endl;
    cube<double> B(3,2,1);
    out.Read("/Data/test",B);
    std::cout << B << std::endl;
    std::cout <<"Appending extendable dataset..."<<std::endl;
    mat<double> C = random< mat<double> >(3,2);
    std::cout << C << std::endl;
    out.AppendDataSet("/Data/", "test", C);
    std::cout <<"Reading extendable dataset..."<<std::endl;
    cube<double> D(3,2,2);
    out.Read("/Data/test",D);
    std::cout << D << std::endl;
  }
  world_comm.BarrierSync(); // Sync procs within each clone.

}

void Simulation::MPITests()
{
  // Run MPI Tests
  TestInverse(intra_comm);
  TestAllGatherCols(intra_comm);
  TestBroadcast(intra_comm);
  TestSendReceive(intra_comm);
  TestSendrecv(intra_comm);
  TestSums(intra_comm);
}

void Simulation::TestInverse(Communicator &my_comm)
{
  int my_proc = my_comm.MyProc();
  int n = 4;

  // Set up initial matrices
  mat<double> A = random<mat<double>>(n,n);
  my_comm.Broadcast(0,A);
  mat<double> In = identity<mat<double>>(n,n);

  // Divide up problem
  mat<double> B;
  my_comm.ScatterCols(0,In,B);

  // Compute partial inverses
  mat<double> X = solve(A,B);

  // Gather full inverse
  mat<double> AI = zeros<mat<double>>(n,n);
  my_comm.Gather(0,X,AI);
  if (my_proc == 0) {
    if (abs(sum(A*AI-In)) < 1.e-9)
      std::cout << "Inverse test ... passed." << std::endl;
    else {
      std::cout << "Inverse test ... failed." << std::endl;
      exit(1);
    }
  }

  ReturnSync();
}

void Simulation::TestAllGatherCols(Communicator &my_comm)
{
  int my_proc = my_comm.MyProc();
  int n_procs = my_comm.NumProcs();
  int n_cols = 2*n_procs+2;
  int n_rows = 2;
  mat<double> A = my_proc*ones<mat<double>>(n_rows,n_cols);
  my_comm.AllGatherCols(A);
  int it_worked = 0;
  if (my_proc==A(0,int(n_cols*float(my_proc)/float(n_procs)+1.)))
    it_worked = 1;
  int tot = 0;
  my_comm.Sum(0, it_worked, tot);
  if (my_proc == 0) {
    if (tot == my_comm.NumProcs())
      std::cout << "AllGatherCols test ... passed." << std::endl;
    else {
      std::cout << "AllGatherCols test ... failed." << std::endl;
      exit(1);
    }
  }

  ReturnSync();
}

void Simulation::TestBroadcast(Communicator &my_comm)
{
  int my_proc = my_comm.MyProc();

  // int test
  int my_value = my_proc;
  my_comm.Broadcast(0, my_value);
  if (my_proc == 1) {
    if (my_value == 0)
      std::cout << "Broadcast int test ... passed." << std::endl;
    else {
      std::cout << "Broadcast int test ... failed." << std::endl;
      exit(1);
    }
  }

  // mat<int> test
  mat<int> my_mat = my_proc*ones<mat<int>>(2,2);
  my_comm.Broadcast(0, my_mat);
  mat<int> known_mat = 0*ones<mat<int>>(2,2);
  if (my_proc == 1) {
    if(sum(my_mat-known_mat)==0)
      std::cout << "Broadcast mat<int> test ... passed." << std::endl;
    else {
      std::cout << "Broadcast mat<int> test ... failed." << std::endl;
      exit(1);
    }
  }

  ReturnSync();
}

void Simulation::TestSendReceive(Communicator &my_comm)
{
  int my_proc = my_comm.MyProc();

  // int test
  if (my_proc == 0) {
    int value = 17;
    my_comm.Send(1, value);
  } else if (my_proc == 1) {
    int value = 0;
    int tmp_value = 17;
    my_comm.Receive(0, value);
    if (value-tmp_value == 0)
      std::cout << "Send/Receive int test ... passed." << std::endl;
    else {
      std::cout << "Send/Receive int test ... failed." << std::endl;
      exit(1);
    }
  }

  // mat<int> test
  if (my_proc == 0) {
    mat<int> value = ones<mat<int>>(2,2);
    my_comm.Send(1, value);
  } else if (my_proc == 1) {
    mat<int> value = zeros<mat<int>>(2,2);
    mat<int> tmp_value = ones<mat<int>>(2,2);
    my_comm.Receive(0, value);
    if (sum(value-tmp_value)==0)
      std::cout << "Send/Receive mat<int> test ... passed." << std::endl;
    else {
      std::cout << "Send/Receive mat<int> test ... failed." << std::endl;
      exit(1);
    }
  }

  ReturnSync();
}

void Simulation::TestSendrecv(Communicator &my_comm)
{
  int n_procs = my_comm.NumProcs();
  int my_proc = my_comm.MyProc();
  int send_proc = (my_proc+1) % n_procs;
  int recv_proc = ((my_proc-1) + n_procs) % n_procs;

  // int test
  int val1 = my_proc;
  int val2 = recv_proc;
  my_comm.SendReceive(send_proc, val1, recv_proc, val1);
  if (my_proc == 0) {
    if (val1==val2)
      std::cout << "SendReceive int test ... passed." << std::endl;
    else {
      std::cout << "SendReceive int test ... failed." << std::endl;
      exit(1);
    }
  }

  // mat<int> test
  mat<int> mat1 = my_proc*ones<mat<int>>(2,2);
  mat<int> mat2 = recv_proc*ones<mat<int>>(2,2);
  my_comm.SendReceive(send_proc, mat1, recv_proc, mat1);
  if (my_proc == 0) {
    if(sum(mat1-mat2)==0)
      std::cout << "SendReceive mat<int> test ... passed." << std::endl;
    else {
      std::cout << "SendReceive mat<int> test ... failed." << std::endl;
      exit(1);
    }
  }

  ReturnSync();
}

void Simulation::TestSums(Communicator &my_comm)
{
  int my_proc = my_comm.MyProc();

  // int test
  int my_value = 1;
  int tot = 0;
  my_comm.Sum(0, my_value, tot);
  if (my_proc == 0) {
    if(tot == my_comm.NumProcs())
      std::cout << "Sum int test ... passed." << std::endl;
    else {
      std::cout << "Sum int test ... failed." << std::endl;
      exit(1);
    }
  }

  // mat<int> test
  mat<int> my_mat = ones<mat<int>>(2,2);
  mat<int> my_sum = zeros<mat<int>>(2,2);
  my_comm.Sum(0, my_mat, my_sum);
  mat<int> known_sum = my_comm.NumProcs()*ones<mat<int>>(2,2);
  if (my_proc == 0) {
    if(sum(my_sum-known_sum)==0)
      std::cout << "Sum mat<int> test ... passed." << std::endl;
    else {
      std::cout << "Sum mat<int> test ... failed." << std::endl;
      exit(1);
    }
  }
  ReturnSync();
}



#endif
