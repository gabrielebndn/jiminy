#include <iostream>

#include "jiminy/core/robot/Robot.h"
#include "jiminy/core/control/AbstractController.h"

#include "jiminy/core/engine/Engine.h"


namespace jiminy
{
    Engine::Engine(void):
    EngineMultiRobot(),
    isInitialized_(false),
    robot_(nullptr),
    controller_(nullptr)
    {
        // Empty on purpose.
    }

    hresult_t Engine::initialize(std::shared_ptr<Robot>              robot,
                                 std::shared_ptr<AbstractController> controller,
                                 callbackFunctor_t                   callbackFct)
    {
        hresult_t returnCode = hresult_t::SUCCESS;

        // Make sure the simulation is properly stopped
        if (isSimulationRunning_)
        {
            stop();
        }

        // Remove the existing system if already initialized
        if(isInitialized_)
        {
            removeSystem("");  // It cannot fail at this point
            isInitialized_ = false;
        }

        /* Add the system without associated name, since
           it is irrelevant for a single robot engine. */
        returnCode = addSystem("", std::move(robot),
                               std::move(controller),
                               std::move(callbackFct));

        if (returnCode == hresult_t::SUCCESS)
        {
            // Get some convenience proxies
            robot_ = systems_.begin()->robot.get();
            controller_ = systems_.begin()->controller.get();

            // Set the initialization flag
            isInitialized_ = true;
        }

        return returnCode;
    }

    hresult_t Engine::initialize(std::shared_ptr<Robot> robot,
                                 callbackFunctor_t      callbackFct)
    {
        hresult_t returnCode = hresult_t::SUCCESS;

        /* Add the system without associated name, since
           it is irrelevant for a single robot engine. */
        returnCode = addSystem("", std::move(robot), std::move(callbackFct));

        if (returnCode == hresult_t::SUCCESS)
        {
            // Get some convenience proxies
            robot_ = systems_.begin()->robot.get();
            controller_ = systems_.begin()->controller.get();

            // Set the initialization flag
            isInitialized_ = true;
        }

        return returnCode;
    }

    hresult_t Engine::setController(std::shared_ptr<AbstractController> controller)
    {
        return setController("", controller);
    }

    hresult_t singleToMultipleSystemsInitialData(Robot const & robot,
                                                 bool_t const & isStateTheoretical,
                                                 vectorN_t const & qInit,
                                                 vectorN_t const & vInit,
                                                 std::optional<vectorN_t> const & aInit,
                                                 std::map<std::string, vectorN_t> & qInitList,
                                                 std::map<std::string, vectorN_t> & vInitList,
                                                 std::optional<std::map<std::string, vectorN_t> > & aInitList)
    {
        hresult_t returnCode = hresult_t::SUCCESS;

        if (isStateTheoretical && robot.mdlOptions_->dynamics.enableFlexibleModel)
        {
            vectorN_t q0;
            returnCode = robot.getFlexibleConfigurationFromRigid(qInit, q0);
            qInitList.emplace("", std::move(q0));
        }
        else
        {
            qInitList.emplace("", qInit);
        }

        if (returnCode == hresult_t::SUCCESS)
        {
            if (isStateTheoretical && robot.mdlOptions_->dynamics.enableFlexibleModel)
            {
                vectorN_t v0;
                returnCode = robot.getFlexibleVelocityFromRigid(vInit, v0);
                vInitList.emplace("", std::move(v0));
            }
            else
            {
                vInitList.emplace("", vInit);
            }
        }
        if (returnCode == hresult_t::SUCCESS)
        {
            if (aInit)
            {
                aInitList.emplace();
                if (isStateTheoretical && robot.mdlOptions_->dynamics.enableFlexibleModel)
                {
                    vectorN_t a0;
                    returnCode = robot.getFlexibleVelocityFromRigid(*aInit, a0);
                    aInitList->emplace("", std::move(a0));
                }
                else
                {
                    aInitList->emplace("", *aInit);
                }
            }
        }

        return returnCode;
    }


    hresult_t Engine::start(vectorN_t const & qInit,
                            vectorN_t const & vInit,
                            std::optional<vectorN_t> const & aInit,
                            bool_t    const & isStateTheoretical,
                            bool_t    const & resetRandomNumbers,
                            bool_t    const & resetDynamicForceRegister)
    {
        hresult_t returnCode = hresult_t::SUCCESS;

        if (!isInitialized_)
        {
            PRINT_ERROR("The engine is not initialized.");
            returnCode = hresult_t::ERROR_INIT_FAILED;
        }

        std::map<std::string, vectorN_t> qInitList;
        std::map<std::string, vectorN_t> vInitList;
        std::optional<std::map<std::string, vectorN_t> > aInitList = std::nullopt;
        if (returnCode == hresult_t::SUCCESS)
        {
            returnCode = singleToMultipleSystemsInitialData(
                *robot_, isStateTheoretical, qInit, vInit, aInit, qInitList, vInitList, aInitList);
        }

        if (returnCode == hresult_t::SUCCESS)
        {
            returnCode = EngineMultiRobot::start(
                qInitList, vInitList, aInitList, resetRandomNumbers, resetDynamicForceRegister);
        }

        return returnCode;
    }

    hresult_t Engine::simulate(float64_t const & tEnd,
                               vectorN_t const & qInit,
                               vectorN_t const & vInit,
                               std::optional<vectorN_t> const & aInit,
                               bool_t    const & isStateTheoretical)
    {
        hresult_t returnCode = hresult_t::SUCCESS;

        if (!isInitialized_)
        {
            PRINT_ERROR("The engine is not initialized.");
            returnCode = hresult_t::ERROR_INIT_FAILED;
        }

        std::map<std::string, vectorN_t> qInitList;
        std::map<std::string, vectorN_t> vInitList;
        std::optional<std::map<std::string, vectorN_t> > aInitList = std::nullopt;
        if (returnCode == hresult_t::SUCCESS)
        {
            returnCode = singleToMultipleSystemsInitialData(
                *robot_, isStateTheoretical, qInit, vInit, aInit, qInitList, vInitList, aInitList);
        }

        if (returnCode == hresult_t::SUCCESS)
        {
            returnCode = EngineMultiRobot::simulate(tEnd, qInitList, vInitList, aInitList);
        }

        return returnCode;
    }

    hresult_t Engine::registerForceImpulse(std::string      const & frameName,
                                           float64_t        const & t,
                                           float64_t        const & dt,
                                           pinocchio::Force const & F)
    {
        return EngineMultiRobot::registerForceImpulse("", frameName, t, dt, F);
    }

    hresult_t Engine::registerForceProfile(std::string           const & frameName,
                                           forceProfileFunctor_t         forceFct)
    {
        return EngineMultiRobot::registerForceProfile("", frameName, forceFct);
    }

    bool_t const & Engine::getIsInitialized(void) const
    {
        return isInitialized_;
    }

    hresult_t Engine::getRobot(std::shared_ptr<Robot> & robot)
    {
        if (!isInitialized_)
        {
            PRINT_ERROR("The engine is not initialized.");
            return hresult_t::ERROR_BAD_INPUT;
        }

        robot = systems_.begin()->robot;

        return hresult_t::SUCCESS;
    }

    hresult_t Engine::getController(std::shared_ptr<AbstractController> & controller)
    {
        if (!isInitialized_)
        {
            PRINT_ERROR("The engine is not initialized.");
            return hresult_t::ERROR_BAD_INPUT;
        }

        controller = systems_.begin()->controller;

        return hresult_t::SUCCESS;
    }

    hresult_t Engine::getSystemState(systemState_t const * & systemState) const
    {
        static systemState_t const systemStateEmpty;

        hresult_t returnCode = hresult_t::SUCCESS;

        if (!isInitialized_)
        {
            PRINT_ERROR("The engine is not initialized.");
            returnCode = hresult_t::ERROR_BAD_INPUT;
        }

        if (returnCode == hresult_t::SUCCESS)
        {
            EngineMultiRobot::getSystemState("", systemState);  // It cannot fail at this point
            return returnCode;
        }

        systemState = &systemStateEmpty;
        return returnCode;
    }
}
