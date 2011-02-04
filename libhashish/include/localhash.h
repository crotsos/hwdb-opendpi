/*
** $Id$
**
** Copyright (C) 2006 - Hagen Paul Pfeifer <hagen@jauu.net>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _LOCALHASH_H
#define	_LOCALHASH_H

#include "../config.h"

#include <stdlib.h>
#include <inttypes.h>

/* localhash/datagen.c */
int random_string(uint32_t, char **, struct drand48_data *);
double gaussian(double, double, struct drand48_data *);

/* localhash/random.c */
unsigned int get_proper_seed(void);

/* localhash/mt.c */
void seed_mt(uint32_t);
uint32_t random_mt(void);

#ifdef __cplusplus
}
#endif

#endif /* _LOCALHASH_H */

/* vim:set ts=4 sw=4 sts=4 tw=78 ff=unix noet: */
