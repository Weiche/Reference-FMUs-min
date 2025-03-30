#ifndef config_h
#define config_h
/**
 * @file config.h
 * @brief Configuration file for the DigitalController example.
 */
// Model descriptions

/**
 * Model identifier should be unique and It must be the same as in the XML file.
 * The identifier is also the name of build artifact (DLL or shared object).
 */ 
#define MODEL_IDENTIFIER DigitalController

/**
 * It is used to verify the XML file with the compiled model.
 * It must be the same as in the XML file.
 */
#define INSTANTIATION_TOKEN "{WTF-132BDF73E644}"

/**
 * These macros defines which FMI model are supported by the FMU.activateClock
 * The valid definitions are CO_SIMULATION, SCHEDULE_CO_SIMULATION or MODEL_EXCHANGE.
 */
#define CO_SIMULATION
#define MODEL_EXCHANGE

/**
 * These macros defines which model variable getter/setter functions are specificly implemented in the model.
 * GET_FLOAT64 seems always should be defined.
 */
#define GET_INT32
#define SET_INT32

/**
 * EVENT_UPDATE: if the model supports event mode feature, this macro should be defined.
 */
#define EVENT_UPDATE

/**
 * The fixedInternalStepSize attribute of CoSimulation in the model description XML
 */
#define FIXED_SOLVER_STEP 0.2
#define DEFAULT_STOP_TIME 10

/*
 ValueRefences are used to identify variables in the model. They are defined in the model description file (XML) and should be unique.
 The name should follows the pattern of vr_<name> which the name should be the same as in the XML file.
*/
typedef enum {
    vr_time,
    vr_counter
} ValueReference;


/*
 ModelData is a structure that contains the dedicated data of model (that is FMI implementation should not appear here)
*/
typedef struct {
    int counter;
} ModelData;

#endif /* config_h */
