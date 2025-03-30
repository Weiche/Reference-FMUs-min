#include "model.h"

/**
 * @brief Calculates the counter value based on the component's time and next event time.
 *
 * This function increments the counter (M(counter)) if the current time (comp->time) is close to the next event time (comp->nextEventTime).
 * It also updates the next event time by incrementing it by 1.
 *
 * @param comp A pointer to the ModelInstance structure representing the component.
 */
static void calculateCounter(ModelInstance* comp) {

    if (comp->nextEventTimeDefined && isClose(comp->time, comp->nextEventTime)) {
        M(counter)++;
        comp->nextEventTime += 1;
    }
}

/**
 * @brief Sets the initial values for the model instance.
 *
 * This function initializes the model-specific data, such as setting the initial
 * value of the counter and defining the next event time. It is called during
 * the instantiation and reset of the model instance.
 *
 * @param comp A pointer to the ModelInstance structure representing the component.
 *
 * @note This function is called by:
 *       - createModelInstance (src/cosimulation.c), which is called by:
 *           - fmi3InstantiateModelExchange (src/fmi3Functions.c)
 *           - fmi3InstantiateCoSimulation (src/fmi3Functions.c)
 *           - fmi3InstantiateScheduledExecution (src/fmi3Functions.c)
 *       - reset (src/cosimulation.c), which is called by:
 *           - fmi3Reset (src/fmi3Functions.c)
 */
void setStartValues(ModelInstance *comp) {
    M(counter) = 1;

    // TODO: move this to initialize()?
    comp->nextEventTime        = 1;
    comp->nextEventTimeDefined = true;
}

/**
 * @brief Calculates values for the model instance.
 *
 * This function currently does nothing.
 *
 * @param comp A pointer to the ModelInstance structure representing the component.
 * @return OK always.
 */
Status calculateValues(ModelInstance *comp) {
    UNUSED(comp);
    // nothing to do
    return OK;
}

/**
 * @brief Updates the state of the model instance during an event.
 *
 * This function calculates the counter, checks if the counter exceeds the maximum value (10),
 * and sets the flags for continuous state changes, termination, and next event time.
 *
 * @param comp A pointer to the ModelInstance structure representing the component.
 * @return OK if the event update is successful, Error otherwise.
 */
Status eventUpdate(ModelInstance *comp) {

    calculateCounter(comp);

    if (M(counter) > 10) {
        logError(comp, "Variable \"counter\" cannot be incremented for values >= 10.");
        return Error;
    }
    // comp->nextEventTime = 
    comp->valuesOfContinuousStatesChanged   = false;
    comp->nominalsOfContinuousStatesChanged = false;
    comp->terminateSimulation               = M(counter) >= 10;
    comp->nextEventTimeDefined              = true;

    return OK;
}
