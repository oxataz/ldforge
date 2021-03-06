/*
 *  LDForge: LDraw parts authoring CAD
 *  Copyright (C) 2013, 2014 Santeri Piippo
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  =====================================================================
 *
 *  config.cxx: Configuration management. I don't like how unsafe QSettings
 *  is so this implements a type-safer and identifer-safer wrapping system of
 *  configuration variables. QSettings is used underlyingly, this is a matter
 *  of interface.
 */

#include <errno.h>
#include <QDir>
#include <QTextStream>
#include <QSettings>
#include "main.h"
#include "configuration.h"
#include "miscallenous.h"
#include "mainWindow.h"
#include "ldDocument.h"

#ifdef _WIN32
# define EXTENSION ".ini"
#else
# define EXTENSION ".cfg"
#endif // _WIN32

#define MAX_CONFIG 512

ConfigEntry*						g_configPointers[MAX_CONFIG];
static int							g_cfgPointerCursor = 0;
static QMap<QString, ConfigEntry*>	g_configsByName;
static QList<ConfigEntry*>			g_configs;

ConfigEntry::ConfigEntry (QString name) :
	m_name (name) {}

//
// Load the configuration from file
//
bool Config::load()
{
	QSettings* settings = settingsObject();
	print ("config::load: Loading configuration file from %1\n", settings->fileName());

	for (ConfigEntry* cfg : g_configPointers)
	{
		if (cfg == null)
			break;

		QVariant val = settings->value (cfg->name(), cfg->getDefaultAsVariant());
		cfg->loadFromVariant (val);
		g_configsByName[cfg->name()] = cfg;
		g_configs << cfg;
	}

	if (g_win != null)
		g_win->loadShortcuts (settings);

	delete settings;
	return true;
}

//
// Save the configuration to disk
//
bool Config::save()
{
	QSettings* settings = settingsObject();

	for (ConfigEntry* cfg : g_configs)
	{
		if (not cfg->isDefault())
			settings->setValue (cfg->name(), cfg->toVariant());
		else
			settings->remove (cfg->name());
	}

	if (g_win != null)
		g_win->saveShortcuts (settings);

	settings->sync();
	print ("Configuration saved to %1.\n", settings->fileName());
	delete settings;
	return true;
}

//
// Reset configuration to defaults.
//
void Config::reset()
{
	for (ConfigEntry* cfg : g_configs)
		cfg->resetValue();
}

//
// Where is the configuration file located at?
//
QString Config::filepath (QString file)
{
	return Config::dirpath() + DIRSLASH + file;
}

//
// Directory of the configuration file.
//
QString Config::dirpath()
{
	QSettings* settings = settingsObject();
	QString result = dirname (settings->fileName());
	delete settings;
	return result;
}

//
// Accessor to the settings object
//
QSettings* Config::settingsObject()
{
	QString path = qApp->applicationDirPath() + "/" UNIXNAME EXTENSION;
	return new QSettings (path, QSettings::IniFormat);
}

//
// We cannot just add config objects to a list or vector because that would rely
// on the vector's c-tor being called before the configs' c-tors. With global
// variables we cannot assume that, therefore we need to use a C-style array here.
//
void ConfigEntry::addToArray (ConfigEntry* ptr)
{
	if (g_cfgPointerCursor == 0)
		memset (g_configPointers, 0, sizeof g_configPointers);

	assert (g_cfgPointerCursor < MAX_CONFIG);
	g_configPointers[g_cfgPointerCursor++] = ptr;
}

template<typename T>
T* getConfigByName (QString name, ConfigEntry::Type type)
{
	auto it = g_configsByName.find (name);

	if (it == g_configsByName.end())
		return null;

	ConfigEntry* cfg = it.value();

	if (cfg->getType() != type)
	{
		fprint (stderr, "type of %1 is %2, not %3\n", name, cfg->getType(), type);
		abort();
	}

	return reinterpret_cast<T*> (cfg);
}

#undef IMPLEMENT_CONFIG

#define IMPLEMENT_CONFIG(NAME)												\
	NAME##ConfigEntry* NAME##ConfigEntry::getByName (QString name)			\
	{																		\
		return getConfigByName<NAME##ConfigEntry> (name, E##NAME##Type);	\
	}

IMPLEMENT_CONFIG (Int)
IMPLEMENT_CONFIG (String)
IMPLEMENT_CONFIG (Bool)
IMPLEMENT_CONFIG (Float)
IMPLEMENT_CONFIG (List)
IMPLEMENT_CONFIG (KeySequence)
IMPLEMENT_CONFIG (Vertex)
