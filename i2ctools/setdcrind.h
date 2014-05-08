/*
 * setdcrind.h
 *
 *  Created on: May 1, 2014
 *      Author: remo
 */

#ifndef SETDCRIND_H_
#define SETDCRIND_H_


#include "defines.h"



#define TOP_FIRST_LOOP (0)
#define TOP_LAST_LOOP (11)
#define BOTTOM_FIRST_LOOP (12)
#define BOTTOM_LAST_LOOP (23)

#define TOP_BITMASK 0x00000FFF
#define BOTTOM_BITMASK 0x00FFF000
#define BOTH_BITMASK (TOP_BITMASK|BOTTOM_BITMASK)


#endif /* SETDCRIND_H_ */

