#include "model.h"

/**
 * @brief Retrieves a Float64 value based on the given value reference.
 *
 * This function retrieves a Float64 value based on the provided value reference (vr).
 * Currently, it only supports retrieving the current time (comp->time) when vr is vr_time.
 *
 * @param comp A pointer to the ModelInstance structure representing the component.
 * @param vr The ValueReference for which to retrieve the Float64 value.
 * @param values An array to store the retrieved Float64 value.
 * @param nValues The number of values to retrieve (must be 1).
 * @param index A pointer to the index in the values array where the value should be stored.
 * @return OK if the value is successfully retrieved, Error otherwise.
 */
Status getFloat64(ModelInstance* comp, ValueReference vr, double values[], size_t nValues, size_t* index) {

    ASSERT_NVALUES(1);

    switch (vr) {
    case vr_time:
        values[(*index)++] = comp->time;
        return OK;
    default:
        logError(comp, "Get Float64 is not allowed for value reference %u.", vr);
        return Error;
    }
}

/**
 * @brief Retrieves an Int32 value based on the given value reference.
 *
 * This function retrieves an Int32 value based on the provided value reference (vr).
 * It supports retrieving the counter value (M(counter)) when vr is vr_counter.
 * When FMI_VERSION is 3 and the component is in EventMode, it calculates the counter before retrieving the value.
 *
 * @param comp A pointer to the ModelInstance structure representing the component.
 * @param vr The ValueReference for which to retrieve the Int32 value.
 * @param values An array to store the retrieved Int32 value.
 * @param nValues The number of values to retrieve (must be 1).
 * @param index A pointer to the index in the values array where the value should be stored.
 * @return OK if the value is successfully retrieved, Error otherwise.
 */
Status getInt32(ModelInstance* comp, ValueReference vr, int32_t values[], size_t nValues, size_t* index) {

    ASSERT_NVALUES(1);

    switch (vr) {
        case vr_counter:
#if FMI_VERSION == 3
            if (comp->state == EventMode) {
                calculateCounter(comp);
            }
#endif
            values[(*index)++] = M(counter);
            return OK;
        default:
            logError(comp, "Get Int32 is not allowed for value reference %u.", vr);
            return Error;
    }
}

/**
 * @brief Sets an Int32 value based on the given value reference.
 *
 * This function sets an Int32 value based on the provided value reference (vr).
 * It supports setting the counter value (M(counter)) when vr is vr_counter.
 * It checks if the component is in a valid state to set the counter and if the value is within the allowed range (less than 10).
 *
 * @param comp A pointer to the ModelInstance structure representing the component.
 * @param vr The ValueReference for which to set the Int32 value.
 * @param values An array containing the Int32 value to set.
 * @param nValues The number of values to set (must be 1).
 * @param index A pointer to the index in the values array where the value is located.
 * @return OK if the value is successfully set, Error otherwise.
 */
Status setInt32(ModelInstance* comp, ValueReference vr, const int32_t values[], size_t nValues, size_t* index) {

    ASSERT_NVALUES(1);

    switch (vr) {
    case vr_counter:
#if FMI_VERSION == 1
        if (comp->state != Instantiated) {
            logError(comp, "Variable \"counter\" can only be set after instantiation.");
            return Error;
        }
#else
        if (comp->state != Instantiated && comp->state != InitializationMode) {
            logError(comp, "Variable \"counter\" can only be set in Instantiated and Intialization Mode.");
            return Error;
        }
#endif
        if (values[*index] >= 10) {
            logError(comp, "The maximum value for variable \"counter\" is 10.");
            return Error;
        }

        M(counter) = values[(*index)++];

        return OK;
    default:
        logError(comp, "Set Int32 is not allowed for value reference %u.", vr);
        return Error;
    }
}