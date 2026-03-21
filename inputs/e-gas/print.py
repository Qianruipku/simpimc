import h5py
import numpy as np
import pyblock

burn_in = 100

with h5py.File('e.0.0.h5', 'r') as f:
    # 读取能量数据
    E = np.array(f['/Observables/Energy/total/x'])
    E = np.real(E[burn_in:])

    # print("Raw energy data:", E)
    print("Energy data length:", len(E))

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