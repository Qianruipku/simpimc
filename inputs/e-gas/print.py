import h5py
import numpy as np
import pyblock

burn_in = 100


def ratio_jackknife(num, den, n_block=20):
    """Estimate ratio mean(num)/mean(den) and stderr with leave-one-block-out jackknife."""
    n = min(len(num), len(den))
    if n < 4:
        return np.nan, np.nan, 0

    num = np.asarray(num[:n], dtype=float)
    den = np.asarray(den[:n], dtype=float)

    n_block = max(2, min(n_block, n // 2))
    block_size = n // n_block
    if block_size == 0:
        return np.nan, np.nan, 0

    use_n = block_size * n_block
    num = num[:use_n]
    den = den[:use_n]

    full_num = np.mean(num)
    full_den = np.mean(den)
    if np.isclose(full_den, 0.0):
        return np.nan, np.nan, n_block

    theta = full_num / full_den
    jk_vals = []
    for i in range(n_block):
        lo = i * block_size
        hi = (i + 1) * block_size
        num_loo = np.concatenate([num[:lo], num[hi:]])
        den_loo = np.concatenate([den[:lo], den[hi:]])
        den_mean_loo = np.mean(den_loo)
        if np.isclose(den_mean_loo, 0.0):
            return theta, np.nan, n_block
        jk_vals.append(np.mean(num_loo) / den_mean_loo)

    jk_vals = np.asarray(jk_vals)
    jk_mean = np.mean(jk_vals)
    jk_err = np.sqrt((n_block - 1) / n_block * np.sum((jk_vals - jk_mean) ** 2))
    return theta, jk_err, n_block

with h5py.File('e.0.0.h5', 'r') as f:
    # Read sign-weighted energy accumulator (<sE> when no importance weights are used).
    E = np.array(f['/Observables/Energy/total/x'])
    E = np.real(E[burn_in:])

    sign = None
    if '/Observables/Sign/x' in f:
        sign = np.real(np.array(f['/Observables/Sign/x'])[burn_in:])

    pressure_total = None
    pressure_ideal = None
    pressure_virial = None
    if '/Observables/Pressure/total/x' in f:
        pressure_total = np.real(np.array(f['/Observables/Pressure/total/x'])[burn_in:])
        pressure_ideal = np.real(np.array(f['/Observables/Pressure/ideal/x'])[burn_in:])
        pressure_virial = np.real(np.array(f['/Observables/Pressure/virial/x'])[burn_in:])

    print("Energy data length:", len(E))
    if sign is not None:
        print("Sign data length:", len(sign))
    if pressure_total is not None:
        print("Pressure data length:", len(pressure_total))

    # Print move acceptance for all moves present in file.
    if '/Moves' in f:
        for move in f['/Moves'].keys():
            move_path = f'/Moves/{move}'
            if f'{move_path}/n_accept' in f and f'{move_path}/n_attempt' in f:
                n_accept = np.array(f[f'{move_path}/n_accept'])
                n_attempt = np.array(f[f'{move_path}/n_attempt'])
                total_accept = np.sum(n_accept)
                total_attempt = np.sum(n_attempt)
                acc_rate = total_accept / total_attempt if total_attempt > 0 else float('nan')
                print(f"{move} acceptance: {total_accept}/{total_attempt} = {acc_rate:.4f}")

    # Reblock raw accumulator for reference.
    reblock_data = pyblock.blocking.reblock(E)
    opt = pyblock.blocking.find_optimal_block(len(E), reblock_data)

    energy_summary_lines = []
    if opt[0] != opt[0] or int(opt[0]) >= len(reblock_data):  # NaN or out of range
        print("Warning: optimal block is NaN or out of range, reblock failed.")
        energy_summary_lines.append("reblock=failed")
    else:
        reblock_best = reblock_data[int(opt[0])]
        print(str(reblock_best))
        energy_summary_lines.append(f"reblock={reblock_best}")

    # If Sign is present, compute the physically correct fermion energy E = <sE>/<s>.
    if sign is not None and len(sign) > 0:
        n_pair = min(len(E), len(sign))
        E_pair = E[:n_pair]
        s_pair = sign[:n_pair]
        s_mean = np.mean(s_pair)
        sE_mean = np.mean(E_pair)
        print(f"Average sign: {s_mean}")

        if np.isclose(s_mean, 0.0):
            print("Reweighted energy: undefined because average sign is ~0.")
            energy_summary_lines.append(f"n_samples={n_pair}")
            energy_summary_lines.append(f"mean_sE={sE_mean}")
            energy_summary_lines.append(f"mean_s={s_mean}")
            energy_summary_lines.append("total_energy=nan")
            energy_summary_lines.append("total_energy_stderr=nan")
        else:
            E_reweighted = sE_mean / s_mean
            E_reweighted_jk, E_reweighted_err, n_block = ratio_jackknife(E_pair, s_pair)
            print(f"Reweighted energy <sE>/<s>: {E_reweighted}")
            print(f"Jackknife error ({n_block} blocks): {E_reweighted_err}")

            energy_summary_lines.append(f"n_samples={n_pair}")
            energy_summary_lines.append(f"mean_sE={sE_mean}")
            energy_summary_lines.append(f"mean_s={s_mean}")
            energy_summary_lines.append(f"total_energy={E_reweighted_jk}")
            energy_summary_lines.append(f"total_energy_stderr={E_reweighted_err}")

            with open("energy_reweighted.txt", 'w') as out:
                out.write(f"n_samples={n_pair}\n")
                out.write(f"mean_sE={sE_mean}\n")
                out.write(f"mean_s={s_mean}\n")
                out.write(f"total_energy={E_reweighted_jk}\n")
                out.write(f"total_energy_stderr={E_reweighted_err}\n")
    else:
        print("Sign observable not found: using raw Energy/total/x only.")
        energy_summary_lines.append(f"n_samples={len(E)}")
        raw_mean = np.mean(E)
        raw_stderr = np.std(E, ddof=1) / np.sqrt(len(E)) if len(E) > 1 else np.nan
        energy_summary_lines.append(f"total_energy={raw_mean}")
        energy_summary_lines.append(f"total_energy_stderr={raw_stderr}")

    with open("energy.txt", 'w') as out:
        out.write("\n".join(energy_summary_lines) + "\n")

    if pressure_total is not None and len(pressure_total) > 0:
        print(f"Pressure mean: {np.mean(pressure_total)}")
        print(f"Pressure ideal mean: {np.mean(pressure_ideal)}")
        print(f"Pressure virial mean: {np.mean(pressure_virial)}")