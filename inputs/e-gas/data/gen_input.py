import xml.etree.ElementTree as ET
import subprocess
from math import pi, sqrt

# Parameter section (editable as needed). Prefer externally injected globals (e.g., via runpy.run_path init_globals).
M = globals().get('M', 128)  # n_bead
N = globals().get('N', 2)    # n_part
pol = globals().get('pol', 0)
theta = globals().get('theta', 1.0)
rs = globals().get('rs', 1.0)
seed = globals().get('seed', 1428586593)
lambda_e = globals().get('lambda_e', 0.5)
enable_pressure = globals().get('enable_pressure', False)
print('Running gen_input.py with:', {'M': M, 'N': N, 'pol': pol, 'theta': theta, 'rs': rs, 'seed': seed, 'lambda_e': lambda_e, 'enable_pressure': enable_pressure})

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
    ET.SubElement(particles, 'Species', name=sp['name'], type='e', n_part=str(sp['n_part']), fermi='0', fixed_node='0', init_type='Random', **{'lambda': str(lambda_e)})

# Actions
actions = ET.SubElement(root, 'Actions')
for sp in species_list:
    ET.SubElement(actions, 'Action', name=f'Kinetic{sp["name"]}', type='Kinetic', species=sp['name'], n_images=str(n_images))
# Pair actions: include Coulomb interactions for same-species and cross-species pairs
for i, spa in enumerate(species_list):
    for j, spb in enumerate(species_list):
        if j < i: continue  # avoid duplicates
        name = f'Coulomb{spa["name"]}{spb["name"]}'
        ET.SubElement(actions, 'Action', name=name, type='IlkkaPairAction', file='data/e_e.h5', n_images='0', species_a=spa['name'], species_b=spb['name'], max_level='0', use_long_range='1')

# Moves
moves = ET.SubElement(root, 'Moves')
for sp in species_list:
    ET.SubElement(moves, 'Move', name=f'Bisect{sp["name"]}', type='PermBisectIterative', n_images='1', species=sp['name'], n_level=str(n_level), adaptive='1', target_ratio='0.1')

observables = ET.SubElement(root, 'Observables')
ET.SubElement(observables, 'Observable', name='Energy', type='Energy', skip='1')
if enable_pressure:
    ET.SubElement(observables, 'Observable', name='Pressure', type='Pressure', skip='1')
ET.SubElement(observables, 'Observable', name='PathDump', type='PathDump', skip='1')
ET.SubElement(observables, 'Observable', name='Time', type='Time')

algorithm = ET.SubElement(root, 'Algorithm')
loop_outer = ET.SubElement(algorithm, 'Loop', n_step='4000')
loop_inner = ET.SubElement(loop_outer, 'Loop', n_step='100')
for sp in species_list:
    ET.SubElement(loop_inner, 'Move', name=f'Bisect{sp["name"]}')
ET.SubElement(loop_inner, 'Observable', name='Energy')
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
