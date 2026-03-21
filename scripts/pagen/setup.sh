#!/bin/bash

cd davidSquarer && ./machconfig && make && cd ..
cd ilkkaSquarer && make && cd ..
cd exact && f2py -c coulcc.pyf coulcc36-f90.f90 --f90flags="-ffixed-form" --f77flags="-ffixed-form" && cd ..
