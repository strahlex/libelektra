/*************************************************************************** 
 *  test_backendhelpers.c  - Test suite for helper functions of resolver
 *                  -------------------
 *  begin                : Fri 21 Mar 2008
 *  copyright            : (C) 2008 by Markus Raab
 *  email                : elektra@markus-raab.org
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

#include <tests_internal.h>

#include <kdbinternal.h>

#include <langinfo.h>

#include "resolver.h"

KeySet *set_pluginconf()
{
	return ksNew( 10 ,
		keyNew ("system/path", KEY_VALUE, KDB_DB_FILE, KEY_END),
		keyNew ("user/path", KEY_VALUE, "elektra.ecf", KEY_END),
		KS_END);
}


void test_resolve()
{
	int pathLen = tempHomeLen + 1 + strlen (KDB_DB_USER) + 12 + 1;
	char *path = malloc (pathLen);
	exit_if_fail (path != 0, "malloc failed");
	snprintf (path, pathLen, "%s/%s/elektra.ecf", tempHome, KDB_DB_USER);

	printf ("Resolve Filename\n");

	KeySet *modules = ksNew(0, KS_END);
	elektraModulesInit (modules, 0);

	Key *parentKey = keyNew("system", KEY_END);
	Plugin *plugin = elektraPluginOpen("resolver", modules, set_pluginconf(), 0);
	exit_if_fail (plugin, "could not load resolver plugin");

	KeySet *test_config = set_pluginconf();
	KeySet *config = elektraPluginGetConfig (plugin);
	succeed_if (config != 0, "there should be a config");
	compare_keyset(config, test_config);
	ksDel (test_config);

	succeed_if (plugin->kdbOpen != 0, "no open pointer");
	succeed_if (plugin->kdbClose != 0, "no open pointer");
	succeed_if (plugin->kdbGet != 0, "no open pointer");
	succeed_if (plugin->kdbSet != 0, "no open pointer");
	succeed_if (plugin->kdbError!= 0, "no open pointer");

	succeed_if (!strcmp(plugin->name, "resolver"), "got wrong name");

	resolverHandles *h = elektraPluginGetData(plugin);
	exit_if_fail (h != 0, "no plugin handle");
	succeed_if (!strcmp(h->system.path, "elektra.ecf"), "path not set correctly");
	succeed_if (!strcmp(h->system.filename, KDB_DB_SYSTEM "/elektra.ecf"), "resulting filename not correct");
	succeed_if_same_string (h->user.path, "elektra.ecf");
	succeed_if_same_string (h->user.filename, path);
	plugin->kdbClose(plugin, parentKey);

	// reinit with system path only
	plugin->kdbOpen(plugin, parentKey);
	h = elektraPluginGetData(plugin);
	exit_if_fail (h != 0, "no plugin handle");
	succeed_if (!strcmp(h->system.path, "elektra.ecf"), "path not set correctly");
	succeed_if (!strcmp(h->system.filename, KDB_DB_SYSTEM "/elektra.ecf"), "resulting filename not correct");
	succeed_if(h->user.filename == NULL, "user was initialized, but is not needed");
	plugin->kdbClose(plugin, parentKey);

	keyDel (parentKey);
	elektraPluginClose(plugin, 0);
	elektraModulesClose(modules, 0);
	ksDel (modules);
	free (path);
}

void test_name()
{
	printf ("Resolve Name\n");

	KeySet *modules = ksNew(0, KS_END);
	elektraModulesInit (modules, 0);

	Plugin *plugin = elektraPluginOpen("resolver", modules, set_pluginconf(), 0);
	exit_if_fail (plugin, "could not load resolver plugin");

	KeySet *test_config = set_pluginconf();
	KeySet *config = elektraPluginGetConfig (plugin);
	succeed_if (config != 0, "there should be a config");
	compare_keyset(config, test_config);
	ksDel (test_config);

	succeed_if (plugin->kdbOpen != 0, "no open pointer");
	succeed_if (plugin->kdbClose != 0, "no open pointer");
	succeed_if (plugin->kdbGet != 0, "no open pointer");
	succeed_if (plugin->kdbSet != 0, "no open pointer");
	succeed_if (plugin->kdbError!= 0, "no open pointer");

	succeed_if (!strcmp(plugin->name, "resolver"), "got wrong name");

	resolverHandles *h = elektraPluginGetData(plugin);
	succeed_if (h != 0, "no plugin handle");

	Key *parentKey= keyNew("system", KEY_END);
	plugin->kdbGet(plugin, 0, parentKey);
	succeed_if (!strcmp(keyString(parentKey), KDB_DB_SYSTEM "/elektra.ecf"),
			"resulting filename not correct");

	keyDel (parentKey);
	elektraPluginClose(plugin, 0);
	elektraModulesClose(modules, 0);
	ksDel (modules);
}

void test_lockname()
{
	printf ("Resolve Dirname\n");

	KeySet *modules = ksNew(0, KS_END);
	elektraModulesInit (modules, 0);

	Plugin *plugin = elektraPluginOpen("resolver", modules, set_pluginconf(), 0);
	exit_if_fail (plugin, "could not load resolver plugin");

	KeySet *test_config = set_pluginconf();
	KeySet *config = elektraPluginGetConfig (plugin);
	succeed_if (config != 0, "there should be a config");
	compare_keyset(config, test_config);
	ksDel (test_config);

	succeed_if (plugin->kdbOpen != 0, "no open pointer");
	succeed_if (plugin->kdbClose != 0, "no open pointer");
	succeed_if (plugin->kdbGet != 0, "no open pointer");
	succeed_if (plugin->kdbSet != 0, "no open pointer");
	succeed_if (plugin->kdbError!= 0, "no open pointer");

	succeed_if (!strcmp(plugin->name, "resolver"), "got wrong name");

	resolverHandles *h = elektraPluginGetData(plugin);
	succeed_if (h != 0, "no plugin handle");

	Key *parentKey= keyNew("system", KEY_END);
	plugin->kdbGet(plugin, 0, parentKey);
	succeed_if (!strcmp(h->system.dirname, KDB_DB_SYSTEM),
			"resulting filename not correct");

	keyDel (parentKey);
	elektraPluginClose(plugin, 0);
	elektraModulesClose(modules, 0);
	ksDel (modules);
}

void test_tempname()
{
	printf ("Resolve Tempname\n");

	KeySet *modules = ksNew(0, KS_END);
	elektraModulesInit (modules, 0);

	Plugin *plugin = elektraPluginOpen("resolver", modules, set_pluginconf(), 0);
	exit_if_fail (plugin, "could not load resolver plugin");

	KeySet *test_config = set_pluginconf();
	KeySet *config = elektraPluginGetConfig (plugin);
	succeed_if (config != 0, "there should be a config");
	compare_keyset(config, test_config);
	ksDel (test_config);

	succeed_if (plugin->kdbOpen != 0, "no open pointer");
	succeed_if (plugin->kdbClose != 0, "no open pointer");
	succeed_if (plugin->kdbGet != 0, "no open pointer");
	succeed_if (plugin->kdbSet != 0, "no open pointer");
	succeed_if (plugin->kdbError!= 0, "no open pointer");

	succeed_if (!strcmp(plugin->name, "resolver"), "got wrong name");

	resolverHandles *h = elektraPluginGetData(plugin);
	succeed_if (h != 0, "no plugin handle");

	Key *parentKey= keyNew("system", KEY_END);
	plugin->kdbGet(plugin, 0, parentKey);
	succeed_if (!strncmp(h->system.tempfile, KDB_DB_SYSTEM "/elektra.ecf", sizeof(KDB_DB_SYSTEM)),
			"resulting filename not correct");

	keyDel (parentKey);
	elektraPluginClose(plugin, 0);
	elektraModulesClose(modules, 0);
	ksDel (modules);
}

void test_checkfile()
{
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("valid") == 1, "valid file not recognised");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("/valid") == 0, "valid absolute file not recognised");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("/absolute/valid") == 0, "valid absolute file not recognised");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("../valid") == -1, "invalid file not recognised");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("valid/..") == -1, "invalid file not recognised");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("/../valid") == -1, "invalid absolute file not recognised");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("/valid/..") == -1, "invalid absolute file not recognised");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("very..strict") == -1, "resolver is currently very strict");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("very/..strict") == -1, "resolver is currently very strict");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("very../strict") == -1, "resolver is currently very strict");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("very/../strict") == -1, "resolver is currently very strict");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("/") == -1, "invalid absolute file not recognised");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)(".") == -1, "invalid file not recognised");
	succeed_if (ELEKTRA_PLUGIN_FUNCTION(resolver,checkFile)("..") == -1, "invalid file not recognised");
}


int main(int argc, char** argv)
{
	printf("  RESOLVER  TESTS\n");
	printf("====================\n\n");

	init (argc, argv);

	test_resolve();
	test_name();
	test_lockname();
	test_tempname();
	test_checkfile();


	printf("\ntest_backendhelpers RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);

	return nbError;
}

