// == PREAMBLE ================================================

// * Licensed under the Apache License, Version 2.0; see README

// == FILEDOC =================================================

/** @file Integrator.h
* @author ETS, WBC, JKM
* @date Sep 7, 2013
* @copyright Apache License, Version 2.0
* @brief Contains the base class for RoadRunner integrators
**/

# ifndef RR_INTEGRATOR_H_
# define RR_INTEGRATOR_H_

// == INCLUDES ================================================

#include "rrLogger.h"
#include "rrOSSpecifics.h"
#include "Dictionary.h"
#include "rrException.h"
#include "Solver.h"

#include "tr1proxy/rr_memory.h"
#include "tr1proxy/rr_unordered_map.h"
#include "Registrable.h"
#include "RegistrationFactory.h"
#include <stdexcept>

// == CODE ====================================================

namespace rr {

    class Integrator;

    class ExecutableModel;

    /*-------------------------------------------------------------------------------------------
        IntegratorListener listens for integrator events.
    ---------------------------------------------------------------------------------------------*/
    class IntegratorListener {
    public:

        /**
        * is called after the internal integrator completes each internal time step.
        */
        virtual uint onTimeStep(Integrator *integrator, ExecutableModel *model, double time) = 0;

        /**
        * whenever model event occurs and after it is procesed.
        */
        virtual uint onEvent(Integrator *integrator, ExecutableModel *model, double time) = 0;

        virtual ~IntegratorListener() {};
    };

    typedef cxx11_ns::shared_ptr<IntegratorListener> IntegratorListenerPtr;

    /*-------------------------------------------------------------------------------------------
        Integrator is an abstract base class that provides an interface to specific integrator
        class implementations.
    ---------------------------------------------------------------------------------------------*/
    class RR_DECLSPEC Integrator : public Solver {
    public:

        using Solver::Solver;

        /**
        * Pull down the setValue from superclass.
        * We do not need to reimplement this
        * but we make it explicit.
        */
        using Solver::setValue;

        enum IntegrationMethod {
            Deterministic,
            Stochastic,
            Hybrid,
            Other
        };

        explicit Integrator(ExecutableModel* model);

        virtual ~Integrator() {};

        virtual IntegrationMethod getIntegrationMethod() const = 0;


//        /**
//         * @brief Gets the name associated with this Solver type
//         * @details For developers. This method is a part of the
//         * Registrar interface, repeated here to make polymorphism
//         * on Solver types more natural. i.e.
//         * @code
//         *  solver->getName() // this, rather than
//         *  solver->Registrar::getName() // ugly.
//         * @endcode
//         */
//        virtual std::string getName() const = 0;
//
//        /**
//         * @brief Gets the description with this Solver type
//         * @details For developers. This method is a part of the
//         * Registrar interface, repeated here to make polymorphism
//         * on Solver types more natural. i.e.
//         * @code
//         *  solver->getDescription() // we want this, rather than
//         *  solver->Registrar::getDescription() // this, which is ugly.
//         * @endcode
//         */
//        virtual std::string getDescription() const = 0;
//
//        /**
//         * @brief Gets the hint with this Solver type
//         * @details For developers. This method is a part of the
//         * Registrar interface, repeated here to make polymorphism
//         * on Solver types more natural. i.e.
//         * @code
//         *  solver->getHint() // this, rather than
//         *  solver->Registrar::getHint() // ugly.
//         * @endcode
//         */
//        virtual std::string getHint() const = 0;

        /**
        * @author JKM
        * @brief Called whenever a new model is loaded to allow integrator
        * to reset internal state
        */
        virtual void setModel(ExecutableModel *m);

        virtual void loadConfigSettings();

        virtual void loadSBMLSettings(const std::string &filename);

        virtual double integrate(double t0, double hstep) = 0;

        virtual void restart(double t0) = 0;

        /**
         * @author JKM, WBC, ETS, MTK
         * @brief Fix tolerances for SBML tests
         * @details In order to ensure that the results of the SBML test suite
         * remain valid, this method enforces a lower bound on tolerance values.
         * Sets minimum absolute and relative tolerances to
         * Config::CVODE_MIN_ABSOLUTE and Config::CVODE_MIN_RELATIVE resp.
         */
        virtual void tweakTolerances();

        /**
        * @author FY
        * @brief Set tolerance for floating species or variables that have a rate rule, will only be used in CVODEIntegrator
        */
        virtual void setIndividualTolerance(std::string sid, double value);


        /**
        * @author FY
        * @brief Set tolerance based on concentration of species, will only be used in CVODEIntegrator
        */
        virtual void setConcentrationTolerance(Setting value);

        /**
        * @author FY
        * @brief Get tolerance based on concentration of species, will only be used in CVODEIntegrator
        */
        virtual std::vector<double> getConcentrationTolerance();


        /* CARRYOVER METHODS */
        virtual void setListener(IntegratorListenerPtr) = 0;

        virtual IntegratorListenerPtr getListener() = 0;

        std::string toString() const;

        /**
        * @author JKM
        * @brief Return std::string representation a la Python __repr__ method
        */
        virtual std::string toRepr() const;
        /* !-- END OF CARRYOVER METHODS */
    };


    class IntegratorException : public std::runtime_error {
    public:
        explicit IntegratorException(const std::string &what) :
                std::runtime_error(what) {
        }

        explicit IntegratorException(const std::string &what, const std::string &where) :
                std::runtime_error(what + "; In " + where) {
        }
    };

    static std::mutex integratorFactoryMutex;
    static std::mutex integratorRegistrationMutex;

    /**
     * @author JKM, WBC
     * @brief Constructs new integrators
     * @details This is a singleton class. All methods except for
     * getInstance are fully determined by superclass FactoryWithRegistration.
     */
    class RR_DECLSPEC IntegratorFactory : public RegistrationFactory {
    public:
        /**
         * @brief get an instance of this IntegratorFactory.
         * @details If one exists
         * return is otherwise create one. This method implements the
         * sigleton pattern and is thread safe due to use of std::mutex.
         */
        static IntegratorFactory &getInstance();

        /**
         * @author JKM
         * @brief Registers all integrators at startup
         * @details Is run at first instantiation of @ref RoadRunner.
         * Subsequent calls have no effect.
         */
        static void Register();

    };

}

# endif /* RR_INTEGRATOR_H_ */
