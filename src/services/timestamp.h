/*
 * timecheck.h
 *
 *  Created on: 27 Jun 2024
 *      Author: Maldus
 */

#ifndef SERVICES_TIMESTAMP_H_
#define SERVICES_TIMESTAMP_H_

#include <stdint.h>
#include "bsp/system.h"


typedef uint32_t timestamp_t;

#define TIME_AFTER(a, b)          (((int32_t)((b) - (a)) < 0))
#define TIME_AFTER_OR_EQUAL(a, b) (((int32_t)((b) - (a)) <= 0))


static inline __attribute__((always_inline)) timestamp_t timestamp_get(void) {
    return bsp_system_get_millis();
}

static inline __attribute__((always_inline)) uint8_t timestamp_is_expired(timestamp_t start,
                                                                timestamp_t duration) {
    return TIME_AFTER_OR_EQUAL(timestamp_get(), start + duration);
}


static inline __attribute__((always_inline)) unsigned long timestamp_difference(timestamp_t a, timestamp_t b) {
    if (TIME_AFTER(a, b))
        return -((timestamp_t)b - (timestamp_t)a);
    else
        return (timestamp_t)b - (timestamp_t)a;
}


#endif /* SERVICES_TIMESTAMP_H_ */
