import sys, os
from math import sqrt, pi

# Exact location of PAGEN scripts
PAGEN_HOME = '../../../scripts/pagen'
sys.path.append(PAGEN_HOME)
from GenPairAction import *

# Units
units = {'energy':'H', 'distance':'A'}

# Settings for e-gas
M = 128 # number of time slices
N = 2 # number electrons
pol = 0 # if polarized or not
theta = 1. # T/T_F
rs = 1. # Wigner Seits Radius

# Calculate things
# Fermi Temp (see Martin pg 103 (only he uses atomic units))
if pol:
  TF = 0.5 * (9.*pi/2.)**(2./3.) / (rs**2)
else:
  TF = 0.5 * (9.*pi/4.)**(2./3.) / (rs**2)
T = theta*TF # temperature in Hartree
tau = (1/T)/M # used tau
a0 = 1. # Bohr radius (.52917721092 Angstroms)
a = rs*a0
from math import pi, sqrt
L = pow(N*(4./3.)*pi*a*a*a, 1.0/3.0) # Length of box
k_cut = 14./(L/2.)
print('tau', tau, 'k_cut', k_cut, 'L', L)

# Particles
e = {'type': 'e', 'lambda': 0.5, 'Z': -1.0}

# Potential
potential = {}
potential['function'] = lambda Z1,Z2,r: Z1*Z2/r
potential['r_min'] = 0.0001 # first grid point
potential['r_max'] = 100. # last grid point
potential['n_grid'] = 1000 # number grid points
potential['grid_type'] = "OPTIMIZED" # grid type (LINEAR, LOG, OPTIMIZED (Ilkka only!))

# Squarer
squarer = {}
squarer['type'] = "Ilkka" # Ilkka or David
squarer['tau'] = tau # desired timestep of PIMC simulation
squarer['n_d'] = 3 # dimension
squarer['r_max'] = 100.0 # maximum distance on grid
squarer['n_grid'] = 100 # number of grid points
squarer['grid_type'] = "OPTIMIZED" # grid type (LINEAR, LOG, OPTIMIZED (Ilkka only!))
squarer['n_square'] = 33 # total number of squarings to reach lowest temperature
squarer['n_order'] = -1 # order of off-diagonal PA fit: -1 = no fit (direct spline, Ilkka only!), 0 = only diagonal, 1-3 = fit off-diagonal to 1-3 order
squarer['n_temp'] = 1 # number of temperatures for which to calculate the pair action (David only!)

# Long-range breakup
breakup = {}
breakup['type'] = 'OptimizedEwald' # OptimizedEwald, StandardEwald, or None
breakup['n_d'] = 3 # dimension
breakup['L'] = L # length of box
breakup['r_min'] = 0.0001 # first grid point
breakup['r_max'] = sqrt(breakup['n_d'])*breakup['L']/2. # last grid point
breakup['r_cut'] = breakup['L']/2. # r cutoff for ewald
breakup['k_cut'] = k_cut # k cutoff for ewald
breakup['n_grid'] = 1000 # number of grid points
breakup['grid_type'] = "OPTIMIZED" # grid type (LINEAR, LOG, OPTIMIZED (Ilkka only!))
breakup['n_knots'] = 10 # number of knots in spline (probably fine)
breakup['n_images'] = 10 # Naive check

# Pair action objects
pa_objects = [
{'species_a': e, 'species_b': e, 'potential': potential, 'breakup': breakup, 'squarer': squarer},
]

# Run
run(pa_objects)
