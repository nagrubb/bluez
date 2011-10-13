/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011 GSyC/LibreSoft, Universidad Rey Juan Carlos.
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

#include <gdbus.h>
#include <errno.h>
#include <bluetooth/uuid.h>

#include "adapter.h"
#include "device.h"
#include "error.h"
#include "log.h"
#include "gattrib.h"
#include "attio.h"
#include "att.h"
#include "gatt.h"
#include "thermometer.h"

#define THERMOMETER_INTERFACE "org.bluez.Thermometer"

#define TEMPERATURE_TYPE_UUID		"00002a1d-0000-1000-8000-00805f9b34fb"
#define INTERMEDIATE_TEMPERATURE_UUID	"00002a1e-0000-1000-8000-00805f9b34fb"
#define MEASUREMENT_INTERVAL_UUID	"00002a21-0000-1000-8000-00805f9b34fb"

struct thermometer {
	DBusConnection		*conn;		/* The connection to the bus */
	struct btd_device	*dev;		/* Device reference */
	GAttrib			*attrib;	/* GATT connection */
	struct att_range	*svc_range;	/* Thermometer range */
	guint			attioid;	/* Att watcher id */
	guint			attindid;	/* Att incications id */
	GSList			*chars;		/* Characteristics */
};

struct characteristic {
	struct att_char		attr;	/* Characteristic */
	GSList			*desc;	/* Descriptors */
	struct thermometer	*t;	/* Thermometer where the char belongs */
};

struct descriptor {
	struct characteristic	*ch;
	uint16_t		handle;
	bt_uuid_t		uuid;
};

static GSList *thermometers = NULL;

static void destroy_char(gpointer user_data)
{
	struct characteristic *c = user_data;

	g_slist_free_full(c->desc, g_free);
	g_free(c);
}

static void destroy_thermometer(gpointer user_data)
{
	struct thermometer *t = user_data;

	if (t->attioid > 0)
		btd_device_remove_attio_callback(t->dev, t->attioid);

	if (t->attindid > 0)
		g_attrib_unregister(t->attrib, t->attindid);

	if (t->attrib != NULL)
		g_attrib_unref(t->attrib);

	if (t->chars != NULL)
		g_slist_free_full(t->chars, destroy_char);

	dbus_connection_unref(t->conn);
	btd_device_unref(t->dev);
	g_free(t->svc_range);
	g_free(t);
}

static gint cmp_device(gconstpointer a, gconstpointer b)
{
	const struct thermometer *t = a;
	const struct btd_device *dev = b;

	if (dev == t->dev)
		return 0;

	return -1;
}

static void discover_desc_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	/* TODO */
}

static void read_temp_type_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	/* TODO */
}

static void read_interval_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	/* TODO */
}

static void process_thermometer_char(struct characteristic *ch)
{
	GAttribResultFunc func;

	if (g_strcmp0(ch->attr.uuid, INTERMEDIATE_TEMPERATURE_UUID) == 0) {
		/* TODO: Change intermediate property and emit signal */
		return;
	} else if (g_strcmp0(ch->attr.uuid, TEMPERATURE_TYPE_UUID) == 0)
		func = read_temp_type_cb;
	else if (g_strcmp0(ch->attr.uuid, MEASUREMENT_INTERVAL_UUID) == 0)
		func = read_interval_cb;
	else
		return;

	gatt_read_char(ch->t->attrib, ch->attr.value_handle, 0, func, ch);
}

static void configure_thermometer_cb(GSList *characteristics, guint8 status,
							gpointer user_data)
{
	struct thermometer *t = user_data;
	GSList *l;

	if (status != 0) {
		error("Discover thermometer characteristics: %s",
							att_ecode2str(status));
		return;
	}

	for (l = characteristics; l; l = l->next) {
		struct att_char *c = l->data;
		struct characteristic *ch;
		uint16_t start, end;

		ch = g_new0(struct characteristic, 1);
		ch->attr.handle = c->handle;
		ch->attr.properties = c->properties;
		ch->attr.value_handle = c->value_handle;
		memcpy(ch->attr.uuid, c->uuid, MAX_LEN_UUID_STR + 1);
		ch->t = t;

		t->chars = g_slist_append(t->chars, ch);

		process_thermometer_char(ch);

		start = c->value_handle + 1;

		if (l->next != NULL) {
			struct att_char *c = l->next->data;
			if (start == c->handle)
				continue;
			end = c->handle - 1;
		} else if (c->value_handle != t->svc_range->end)
			end = t->svc_range->end;
		else
			continue;

		gatt_find_info(t->attrib, start, end, discover_desc_cb, ch);
	}
}

static DBusMessage *get_properties(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	/* TODO: */
	return g_dbus_create_error(msg, ERROR_INTERFACE ".ThermometerError",
						"Function not implemented.");
}

static DBusMessage *set_property(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	/* TODO: */
	return g_dbus_create_error(msg, ERROR_INTERFACE ".ThermometerError",
						"Function not implemented.");
}

static DBusMessage *register_watcher(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	/* TODO: */
	return g_dbus_create_error(msg, ERROR_INTERFACE ".ThermometerError",
						"Function not implemented.");
}

static DBusMessage *unregister_watcher(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	/* TODO: */
	return g_dbus_create_error(msg, ERROR_INTERFACE ".ThermometerError",
						"Function not implemented.");
}

static DBusMessage *enable_intermediate(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	/* TODO: */
	return g_dbus_create_error(msg, ERROR_INTERFACE ".ThermometerError",
						"Function not implemented.");
}

static DBusMessage *disable_intermediate(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	/* TODO: */
	return g_dbus_create_error(msg, ERROR_INTERFACE ".ThermometerError",
						"Function not implemented.");
}

static GDBusMethodTable thermometer_methods[] = {
	{ "GetProperties",	"",	"a{sv}",	get_properties },
	{ "SetProperty",	"sv",	"",		set_property,
						G_DBUS_METHOD_FLAG_ASYNC },
	{ "RegisterWatcher",	"o",	"",		register_watcher },
	{ "UnregisterWatcher",	"o",	"",		unregister_watcher },
	{ "EnableIntermediateMeasurement", "o", "", enable_intermediate },
	{ "DisableIntermediateMeasurement","o",	"", disable_intermediate },
	{ }
};

static GDBusSignalTable thermometer_signals[] = {
	{ "PropertyChanged",	"sv"	},
	{ }
};

static void ind_handler(const uint8_t *pdu, uint16_t len, gpointer user_data)
{
	/* TODO: Process indication */
}

static void attio_connected_cb(GAttrib *attrib, gpointer user_data)
{
	struct thermometer *t = user_data;

	t->attrib = g_attrib_ref(attrib);

	t->attindid = g_attrib_register(t->attrib, ATT_OP_HANDLE_IND,
							ind_handler, t, NULL);
	gatt_discover_char(t->attrib, t->svc_range->start, t->svc_range->end,
					NULL, configure_thermometer_cb, t);
}

static void attio_disconnected_cb(gpointer user_data)
{
	struct thermometer *t = user_data;

	DBG("GATT Disconnected");

	if (t->attindid > 0) {
		g_attrib_unregister(t->attrib, t->attindid);
		t->attindid = 0;
	}

	g_attrib_unref(t->attrib);
	t->attrib = NULL;
}

int thermometer_register(DBusConnection *connection, struct btd_device *device,
						struct att_primary *tattr)
{
	const gchar *path = device_get_path(device);
	struct thermometer *t;

	t = g_new0(struct thermometer, 1);
	t->conn = dbus_connection_ref(connection);
	t->dev = btd_device_ref(device);
	t->svc_range = g_new0(struct att_range, 1);
	t->svc_range->start = tattr->start;
	t->svc_range->end = tattr->end;

	if (!g_dbus_register_interface(t->conn, path, THERMOMETER_INTERFACE,
				thermometer_methods, thermometer_signals,
				NULL, t, destroy_thermometer)) {
		error("D-Bus failed to register %s interface",
							THERMOMETER_INTERFACE);
		destroy_thermometer(t);
		return -EIO;
	}

	thermometers = g_slist_prepend(thermometers, t);

	t->attioid = btd_device_add_attio_callback(device, attio_connected_cb,
						attio_disconnected_cb, t);
	return 0;
}

void thermometer_unregister(struct btd_device *device)
{
	struct thermometer *t;
	GSList *l;

	l = g_slist_find_custom(thermometers, device, cmp_device);
	if (l == NULL)
		return;

	t = l->data;
	thermometers = g_slist_remove(thermometers, t);
	g_dbus_unregister_interface(t->conn, device_get_path(t->dev),
							THERMOMETER_INTERFACE);
}
