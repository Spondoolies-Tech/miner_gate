/*
 * Copyright 2014 Zvi (Zvisha) Shteingart - Spondoolies-tech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.  See COPYING for more details.
 *
 * Note that changing this SW will void your miners guaranty
 */

#ifndef _REAAL_TIME_QUEUE_H_
#define _REAAL_TIME_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>

#include "hammer.h"

int one_done_sw_rt_queue(RT_JOB *work);
void push_work_rsp(RT_JOB *work);
void reset_sw_rt_queue();

#endif
