import h5py
import numpy as np
import pyblock

burn_in = 100

with h5py.File('e.0.0.h5', 'r') as f:
    # 读取能量数据
    E = np.array(f['/Observables/Energy/total/x'])
    E = np.real(E[burn_in:])

    pressure_total = None
    pressure_ideal = None
    pressure_virial = None
    if '/Observables/Pressure/total/x' in f:
        pressure_total = np.real(np.array(f['/Observables/Pressure/total/x'])[burn_in:])
        pressure_ideal = np.real(np.array(f['/Observables/Pressure/ideal/x'])[burn_in:])
        pressure_virial = np.real(np.array(f['/Observables/Pressure/virial/x'])[burn_in:])

    # print("Raw energy data:", E)
    print("Energy data length:", len(E))
    if pressure_total is not None:
        print("Pressure data length:", len(pressure_total))

    # 打印 move 接受率
    for move in ["BisecteU", "BisecteD"]:
        n_accept = np.array(f[f"/Moves/{move}/n_accept"])
        n_attempt = np.array(f[f"/Moves/{move}/n_attempt"])
        total_accept = np.sum(n_accept)
        total_attempt = np.sum(n_attempt)
        acc_rate = total_accept / total_attempt if total_attempt > 0 else float('nan')
        print(f"{move} acceptance: {total_accept}/{total_attempt} = {acc_rate:.4f}")

    reblock_data = pyblock.blocking.reblock(E)
    opt = pyblock.blocking.find_optimal_block(len(E), reblock_data)

    if opt[0] != opt[0] or int(opt[0]) >= len(reblock_data):  # NaN or out of range
        print("Warning: optimal block is NaN or out of range, reblock failed.")
    else:
        with open("energy.txt", 'w') as out:
            out.write(str(reblock_data[int(opt[0])]))
            print(str(reblock_data[int(opt[0])]))

    if pressure_total is not None and len(pressure_total) > 0:
        print(f"Pressure mean: {np.mean(pressure_total)}")
        print(f"Pressure ideal mean: {np.mean(pressure_ideal)}")
        print(f"Pressure virial mean: {np.mean(pressure_virial)}")