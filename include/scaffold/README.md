scaffold
========

Utilities for fast parallel communication and IO wrapped neatly in C++.

master: ![master build status](https://travis-ci.org/etano/scaffold.svg?branch=master)

dev: ![dev build status](https://travis-ci.org/etano/scaffold.svg?branch=dev)

## Dependencies

* HDF5 (with c++ API) - http://www.hdfgroup.org/HDF5/
* Armadillo c++ - http://arma.sourceforge.net/ OR Eigen - http://eigen.tuxfamily.org/

These dependencies will automatically be installed by cmake. However, if you'd like to install them yourself, be sure to set the environmental variables:

    export HDF5_HOME=/hdf5/install/directory
    export ARMADILLO_HOME=/armadillo/install/directory
    export EIGEN_HOME=/eigen/install/directory

## Compiling

Scaffold is a header-only library, though the simplest way to compile to use the supplied CMakeLists.txt.

## Tests

For an example build, look in the `tests` folder and do the following:

    cd tests
    mkdir build && cd build
    cmake ..
    make

To run simpimc simply do the following:

    ../bin/scaffold_test ../inputs/test.xml

Or if you wish to run in parallel:

    mpiexec -np 2 ../bin/scaffold_test ../inputs/test.xml

## Troubleshooting

Note that occassionally (depending on your version of cmake), loading ExternalProjects fails the first time around. This is easily remedied by running

    cmake ..
    make

a second time.

If you have any other questions, feel free to contact me.
