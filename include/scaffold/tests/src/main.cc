#include "main.h"

int main(int argc, char** argv)
{
  COMM::Init(argc, argv);

  // Get input file
  std::string in_file = "";
  if( argc == 2 )
    in_file = argv[1];
  else {
    std::cout << "Usage: ./test InputFile\n";
    return 1;
  }

  Simulation sim;
  sim.SetupIO(in_file);
  sim.BuildMPIModel();
  sim.Run();

  COMM::Finalize();

  return 0;
}
