/*
 * sblib_default_objects.h
 *
 *  Created on: 15.07.2015
 *      Author: glueck
 */

#ifndef SBLIB_DEFAULT_OBJECTS_H_
#define SBLIB_DEFAULT_OBJECTS_H_

#include "eib.h"
//#include <sblib/io_pin_names.h>
#ifndef IS_BOOTLOADER
static BCU _bcu = BCU();
BcuBase& bcu = _bcu;
#endif

// The EIB bus access objects
BusHal busHal;
Bus bus(busHal);
//Bus bus(timer16_1, PIN_EIB_RX, PIN_EIB_TX, CAP0, MAT0);

#endif /* SBLIB_DEFAULT_OBJECTS_H_ */
