
#include "jiminy/core/stepper/abstract_stepper.h"

namespace jiminy
{
    AbstractStepper::AbstractStepper(const systemDynamics & f,
                                     const std::vector<const Robot *> & robots) noexcept :
    f_{f},
    robots_{robots},
    state_(robots),
    stateDerivative_(robots),
    fOutput_(robots)
    {
    }

    bool AbstractStepper::tryStep(std::vector<Eigen::VectorXd> & qSplit,
                                  std::vector<Eigen::VectorXd> & vSplit,
                                  std::vector<Eigen::VectorXd> & aSplit,
                                  double & t,
                                  double & dt)
    {
        // Update buffers
        double t_next = t + dt;
        state_.q = qSplit;
        state_.v = vSplit;
        stateDerivative_.v = vSplit;
        stateDerivative_.a = aSplit;

        // Try doing a single step
        bool result = tryStepImpl(state_, stateDerivative_, t, dt);

        // Make sure everything went fine
        if (result)
        {
            for (const Eigen::VectorXd & a : stateDerivative_.a)
            {
                if ((a.array() != a.array()).any())
                {
                    dt = qNAN;
                    result = false;
                }
            }
        }

        // Update output if successful
        if (result)
        {
            t = t_next;
            qSplit = state_.q;
            vSplit = state_.v;
            aSplit = stateDerivative_.a;
        }
        return result;
    }

    const StateDerivative & AbstractStepper::f(double t, const State & state)
    {
        f_(t, state.q, state.v, fOutput_.a);
        fOutput_.v = state.v;
        return fOutput_;
    }
}
