#!/usr/bin/env python3
"""Run gen_input.py and gen_e_pa.py with configurable parameters.

Usage example:
  python gen_all.py --M 128 --N 2 --pol 0 --theta 1.0 --rs 1.0
"""

import argparse
import os
import runpy
import sys


def main():
    parser = argparse.ArgumentParser(description='Generate input XML and pair-action HDF5')
    parser.add_argument('--M', type=int, default=128, help='number of time slices (n_bead)')
    parser.add_argument('--N', type=int, default=2, help='number of particles (n_part)')
    parser.add_argument('--pol', type=int, choices=(0, 1), default=0, help='polarization flag (0 or 1)')
    parser.add_argument('--theta', type=float, default=1.0, help='T/T_F')
    parser.add_argument('--rs', type=float, default=1.0, help='Wigner-Seitz radius')
    parser.add_argument('--seed', type=int, default=1428586593, help='RNG seed passed to gen_input')
    parser.add_argument('--lambda_e', type=float, default=0.5, help='electron lambda passed to gen_input')
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    gen_input_path = os.path.join(script_dir, 'gen_input.py')
    gen_e_pa_path = os.path.join(script_dir, 'gen_e_pa.py')

    if not os.path.exists(gen_input_path):
        print(f'Error: {gen_input_path} not found', file=sys.stderr)
        sys.exit(2)
    if not os.path.exists(gen_e_pa_path):
        print(f'Error: {gen_e_pa_path} not found', file=sys.stderr)
        sys.exit(2)

    # run in the data directory so relative paths inside the scripts resolve
    cwd = os.getcwd()
    try:
        os.chdir(script_dir)

        init_globals = {
            'M': args.M,
            'N': args.N,
            'pol': args.pol,
            'theta': args.theta,
            'rs': args.rs,
            'seed': args.seed,
            'lambda_e': args.lambda_e,
        }

        print('Running gen_input.py with:', init_globals)
        runpy.run_path(gen_input_path, init_globals=init_globals, run_name='__main__')

        print('Running gen_e_pa.py with:', init_globals)
        runpy.run_path(gen_e_pa_path, init_globals=init_globals, run_name='__main__')

        print('Done.')

    finally:
        os.chdir(cwd)


if __name__ == '__main__':
    main()
