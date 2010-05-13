/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <errno.h>
#include <glib.h>

#include <openobex/obex.h>

#include "obex.h"
#include "service.h"
#include "logging.h"

static GSList *drivers = NULL;

struct obex_service_driver *obex_service_driver_find(GSList *drivers,
			const uint8_t *target, unsigned int target_size,
			const uint8_t *who, unsigned int who_size)
{
	GSList *l;

	for (l = drivers; l; l = l->next) {
		struct obex_service_driver *driver = l->data;

		if (memcmp0(who, driver->who, who_size))
			continue;

		if (memcmp0(target, driver->target, target_size) == 0)
			return driver;
	}

	return NULL;
}

GSList *obex_service_driver_list(uint16_t services)
{
	GSList *l;
	GSList *list = NULL;

	for (l = drivers; l && services; l = l->next) {
		struct obex_service_driver *driver = l->data;

		if (driver->service & services) {
			list = g_slist_append(list, driver);
			services &= ~driver->service;
		}
	}

	return list;
}

int obex_service_driver_register(struct obex_service_driver *driver)
{
	if (!driver) {
		error("Invalid driver");
		return -EINVAL;
	}

	if (obex_service_driver_list(driver->service)) {
		error("Permission denied: service %s already registered",
			driver->name);
		return -EPERM;
	}

	debug("driver %p service %s registered", driver, driver->name);

	/* Drivers that support who has priority */
	if (driver->who)
		drivers = g_slist_prepend(drivers, driver);
	else
		drivers = g_slist_append(drivers, driver);

	return 0;
}

void obex_service_driver_unregister(struct obex_service_driver *driver)
{
	if (!g_slist_find(drivers, driver)) {
		error("Unable to unregister: No such driver %p", driver);
		return;
	}

	debug("driver %p service %s unregistered", driver, driver->name);

	drivers = g_slist_remove(drivers, driver);
}
