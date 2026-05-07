import argparse
import xml.etree.ElementTree as ET
import subprocess
from math import pi, sqrt


def get_config():
    parser = argparse.ArgumentParser(description='Generate e-gas XML input')
    parser.add_argument('--M', type=int, help='number of time slices (n_bead)')
    parser.add_argument('--N', type=int, help='number of particles (n_part)')
    parser.add_argument('--pol', type=int, choices=(0, 1), help='polarization flag (0 or 1)')
    parser.add_argument('--theta', type=float, help='T/T_F')
    parser.add_argument('--rs', type=float, help='Wigner-Seitz radius')
    parser.add_argument('--seed', type=int, help='RNG seed')
    parser.add_argument('--lambda_e', type=float, help='electron lambda')
    parser.add_argument('--enable_pressure', type=int, choices=(0, 1), help='enable pressure observable (0 or 1)')
    parser.add_argument('--use_fixed_node', type=int, choices=(0, 1), help='enable fixed-node approximation (0 or 1)')
    parser.add_argument('--nodal_action_type', type=str, choices=('FreeNodal', 'OptimizedFreeNodal', 'OptimizedSHONodal'), help='type of nodal action')
    args = parser.parse_args()

    return {
        'M': args.M if args.M is not None else globals().get('M', 128),
        'N': args.N if args.N is not None else globals().get('N', 2),
        'pol': args.pol if args.pol is not None else globals().get('pol', 0),
        'theta': args.theta if args.theta is not None else globals().get('theta', 1.0),
        'rs': args.rs if args.rs is not None else globals().get('rs', 1.0),
        'seed': args.seed if args.seed is not None else globals().get('seed', 1428586593),
        'lambda_e': args.lambda_e if args.lambda_e is not None else globals().get('lambda_e', 0.5),
        'enable_pressure': bool(args.enable_pressure) if args.enable_pressure is not None else globals().get('enable_pressure', False),
        'use_fixed_node': bool(args.use_fixed_node) if args.use_fixed_node is not None else globals().get('use_fixed_node', True),
        'nodal_action_type': args.nodal_action_type if args.nodal_action_type is not None else globals().get('nodal_action_type', 'FreeNodal'),
    }

# Parameter section (editable as needed). Prefer externally injected globals (e.g., via runpy.run_path init_globals).
config = get_config()
M = config['M']  # n_bead
N = config['N']  # n_part
pol = config['pol']
theta = config['theta']
rs = config['rs']
seed = config['seed']
lambda_e = config['lambda_e']
enable_pressure = config['enable_pressure']
use_fixed_node = config['use_fixed_node']
nodal_action_type = config['nodal_action_type']
print('Running gen_input.py with:', {
    'M': M,
    'N': N,
    'pol': pol,
    'theta': theta,
    'rs': rs,
    'seed': seed,
    'lambda_e': lambda_e,
    'enable_pressure': enable_pressure,
    'use_fixed_node': use_fixed_node,
    'nodal_action_type': nodal_action_type,
})

# Physical quantity calculations
if pol:
    TF = 0.5 * (9.*pi/2.)**(2./3.) / (rs**2)
else:
    TF = 0.5 * (9.*pi/4.)**(2./3.) / (rs**2)
T = theta * TF
beta = 1.0 / T
n_d = 3
n_images = 100
n_level = 6
L = pow(N*(4./3.)*pi*(rs**3), 1.0/3.0)
k_cut = 14.0/(L/2.)

# Generate XML

root = ET.Element('Input')
ET.SubElement(root, 'RNG', seed=str(seed))
ET.SubElement(root, 'IO', output_prefix='e.0')
ET.SubElement(root, 'Parallel', procs_per_group='1')
ET.SubElement(root, 'System', n_d=str(n_d), n_bead=str(M), beta=str(beta), L=str(L), PBC='1', k_cut=str(k_cut))

# Auto-generate spin distribution based on pol
particles = ET.SubElement(root, 'Particles')
species_list = []
if pol == 0:
    n_up = N // 2
    n_down = N - n_up
    species_list.append({'name': 'eU', 'n_part': n_up})
    species_list.append({'name': 'eD', 'n_part': n_down})
else:
    species_list.append({'name': 'eU', 'n_part': N})
for sp in species_list:
    ET.SubElement(
        particles,
        'Species',
        name=sp['name'],
        type='e',
        n_part=str(sp['n_part']),
        fermi='1',
        fixed_node='1' if use_fixed_node else '0',
        init_type='Random',
        **{'lambda': str(lambda_e)},
    )

# Actions
actions = ET.SubElement(root, 'Actions')
for sp in species_list:
    ET.SubElement(actions, 'Action', name=f'Kinetic{sp["name"]}', type='Kinetic', species=sp['name'], n_images=str(n_images))
    if use_fixed_node:
        ET.SubElement(actions, 'Action', name=f'Nodal{sp["name"]}', type=nodal_action_type, species=sp['name'], n_images=str(n_images))
# Pair actions: include Coulomb interactions for same-species and cross-species pairs
for i, spa in enumerate(species_list):
    for j, spb in enumerate(species_list):
        if j < i: continue  # avoid duplicates
        name = f'Coulomb{spa["name"]}{spb["name"]}'
        ET.SubElement(actions, 'Action', name=name, type='IlkkaPairAction', file='./e_e.h5', n_images='0', species_a=spa['name'], species_b=spb['name'], max_level='0', use_long_range='1')

# Moves
moves = ET.SubElement(root, 'Moves')
for sp in species_list:
    ET.SubElement(moves, 'Move', name=f'Bisect{sp["name"]}', type='PermBisectIterative', n_images='1', species=sp['name'], n_level=str(n_level), adaptive='1', target_ratio='0.1')

observables = ET.SubElement(root, 'Observables')
ET.SubElement(observables, 'Observable', name='Energy', type='Energy')
# For direct fermion sampling (no fixed-node), collect sign for reweighting:
#   E = <sE>/<s>
if not use_fixed_node:
    ET.SubElement(observables, 'Observable', name='Sign', type='Sign')
if enable_pressure:
    ET.SubElement(observables, 'Observable', name='Pressure', type='Pressure')
ET.SubElement(observables, 'Observable', name='PathDump', type='PathDump', skip='40000')
ET.SubElement(observables, 'Observable', name='Time', type='Time')

algorithm = ET.SubElement(root, 'Algorithm')
loop_outer = ET.SubElement(algorithm, 'Loop', n_step='40000')
loop_inner = ET.SubElement(loop_outer, 'Loop', n_step='100')
for sp in species_list:
    ET.SubElement(loop_inner, 'Move', name=f'Bisect{sp["name"]}')
ET.SubElement(loop_inner, 'Observable', name='Energy')
if not use_fixed_node:
    ET.SubElement(loop_inner, 'Observable', name='Sign')
if enable_pressure:
    ET.SubElement(loop_inner, 'Observable', name='Pressure')
ET.SubElement(loop_outer, 'Write')

tree = ET.ElementTree(root)

# Pretty-print XML output
xml_path = './e-gas.xml'
import xml.dom.minidom
raw_xml = ET.tostring(root, encoding='utf-8')
parsed = xml.dom.minidom.parseString(raw_xml)
pretty_xml = parsed.toprettyxml(indent='  ', encoding='utf-8')
with open(xml_path, 'wb') as f:
    f.write(pretty_xml)
print(f"Generated {xml_path}")

# Call gen_e_pa.py to generate .h5
print("Running gen_e_pa.py ...")
# subprocess.run(['python3', 'gen_e_pa.py'])
print("gen_e_pa.py finished")
