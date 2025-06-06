#include "array.h"

bool array_contains(void *array, void *element) {
    if (!array) {
        return false;
    }

    for (void **item = array; *item; ++item) {
        if (*item == element) {
            return true;
        }
    }

    return false;
}
