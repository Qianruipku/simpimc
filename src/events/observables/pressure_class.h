#ifndef SIMPIMC_OBSERVABLES_PRESSURE_CLASS_H_
#define SIMPIMC_OBSERVABLES_PRESSURE_CLASS_H_

#include "observable_class.h"

/// Measure pressure using an ideal-gas term plus a simple virial correction.
class Pressure : public Observable {
   private:
    std::vector<std::shared_ptr<Action>> &action_list;  ///< Reference to vector of all actions
    uint32_t n_part_total;                              ///< Total number of particles over all species
    double pressure;                                    ///< Total pressure accumulator
    double pressure_ideal;                              ///< Ideal-gas contribution accumulator
    double pressure_virial;                             ///< Virial contribution accumulator

    /// Interaction virial estimator accumulated over all particles and beads.
    double InteractionVirial() {
        double virial = 0.;
        for (auto &species : path.GetSpecies()) {
            for (uint32_t p_i = 0; p_i < species->GetNPart(); ++p_i) {
                std::vector<std::pair<std::shared_ptr<Species>, uint32_t>> particles;
                particles.push_back(std::make_pair(species, p_i));
                for (uint32_t b_i = 0; b_i < path.GetNBead(); ++b_i) {
                    vec<double> dr(path.Dr(species->GetBead(p_i, b_i), species->GetBead(p_i, 0)));
                    for (uint32_t i = 0; i < action_list.size(); ++i) {
                        if (!action_list[i]->is_importance_weight && action_list[i]->type != "Kinetic") {
                            vec<double> action_gradient = action_list[i]->GetActionGradient(b_i, b_i + 1, particles, 0);
                            virial += dot(action_gradient, dr);
                        }
                    }
                }
            }
        }
        return virial;
    }

    /// Accumulate the observable
    virtual void Accumulate() {
        path.SetMode(NEW_MODE);

        const int sign = path.GetSign();
        const double importance_weight = path.GetImportanceWeight();
        const double cofactor = sign * importance_weight;

        const double beta = path.GetTau() * path.GetNBead();
        const double vol = path.GetVol();
        const double n_d = path.GetND();

        // Ideal gas term: N / (beta * V)
        const double p_ideal = n_part_total / (beta * vol);

        // Virial correction: -(1 / (d * V)) <sum_i r_i . grad_i U>
        // grad_i U is obtained from action gradients using grad(S) = tau * grad(U).
        const double virial_action = InteractionVirial();
        const double p_virial = -virial_action / (path.GetTau() * path.GetNBead() * n_d * vol);
        const double p_sample = p_ideal + p_virial;

        pressure += cofactor * p_sample;
        pressure_ideal += cofactor * p_ideal;
        pressure_virial += cofactor * p_virial;
        n_measure += 1;
    }

    /// Reset the observable's counters
    virtual void Reset() {
        n_measure = 0;
        pressure = 0.;
        pressure_ideal = 0.;
        pressure_virial = 0.;
    }

   public:
    /// Constructor calls Init
    Pressure(Path &path, std::vector<std::shared_ptr<Action>> &tmp_action_list, Input &in, IO &out)
        : action_list(tmp_action_list), Observable(path, in, out, "scalar") {
        n_part_total = 0;
        for (const auto &species : path.GetSpecies())
            n_part_total += species->GetNPart();
        Reset();
    }

    /// Write relevant information about an observable to the output
    virtual void Write() {
        if (n_measure > 0) {
            const double p_avg = pressure / n_measure;
            const double p_ideal_avg = pressure_ideal / n_measure;
            const double p_virial_avg = pressure_virial / n_measure;
            if (first_time) {
                out.CreateGroup(prefix + "total");
                out.CreateExtendableDataSet("/" + prefix + "total/", "x", p_avg);
                out.Write(prefix + "total/data_type", data_type);

                out.CreateGroup(prefix + "ideal");
                out.CreateExtendableDataSet("/" + prefix + "ideal/", "x", p_ideal_avg);
                out.Write(prefix + "ideal/data_type", data_type);

                out.CreateGroup(prefix + "virial");
                out.CreateExtendableDataSet("/" + prefix + "virial/", "x", p_virial_avg);
                out.Write(prefix + "virial/data_type", data_type);

                first_time = 0;
            } else {
                out.AppendDataSet("/" + prefix + "total/", "x", p_avg);
                out.AppendDataSet("/" + prefix + "ideal/", "x", p_ideal_avg);
                out.AppendDataSet("/" + prefix + "virial/", "x", p_virial_avg);
            }
            Reset();
        }
    }
};

#endif  // SIMPIMC_OBSERVABLES_PRESSURE_CLASS_H_