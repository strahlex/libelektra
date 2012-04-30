/*************************************************************************** 
 *           test_mount.c  - Test suite for split buildup during mount
 *                  -------------------
 *  begin                : Sat Jul 10 2010
 *  copyright            : (C) 2010 by Markus Raab
 *  email                : elektra@markus-raab.org
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_KDBCONFIG_H
#include "kdbconfig.h"
#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <tests.h>

/*Needs private declarations*/
#include <kdbprivate.h>

KDB* kdb_new()
{
	KDB *kdb = elektraCalloc (sizeof (KDB));
	kdb->split = elektraSplitNew();
	return kdb;
}

Backend *b_new(const char *name, const char *value)
{
	Backend *backend = elektraCalloc (sizeof (Backend));
	backend->refcounter = 1;

	backend->mountpoint = keyNew (name, KEY_VALUE, value, KEY_END);
	keyIncRef (backend->mountpoint);

	return backend;
}

void kdb_del(KDB *kdb)
{
	elektraBackendClose (kdb->defaultBackend, 0);
	elektraTrieClose(kdb->trie, 0);
	elektraSplitDel (kdb->split);

	elektraFree (kdb);
}

void test_mount()
{
	printf ("test mount backend\n");

	KDB *kdb = kdb_new();
	elektraMountBackend (kdb, b_new("user", "user"), 0);
	succeed_if (kdb->trie, "there should be a trie");

	Key *mp = keyNew ("user", KEY_VALUE, "user", KEY_END);
	Key *sk = keyNew ("user", KEY_VALUE, "user", KEY_END);

	succeed_if (kdb->split->size == 1, "size of split not correct");
	succeed_if (compare_key (mp, kdb->split->parents[0]) == 0, "parent not correct in split");

	succeed_if (compare_key (elektraMountGetBackend (kdb, sk)->mountpoint, mp) == 0, "could not find mp");
	succeed_if (compare_key (elektraMountGetMountpoint (kdb, sk), mp) == 0, "could not find mp");

	keySetName (sk, "user/below");
	succeed_if (compare_key (elektraMountGetBackend (kdb, sk)->mountpoint, mp) == 0, "could not find mp");
	succeed_if (compare_key (elektraMountGetMountpoint (kdb, sk), mp) == 0, "could not find mp below");

	keySetName (sk, "system");
	kdb->defaultBackend = b_new("", "default");
	succeed_if (elektraMountGetBackend (kdb, sk) == kdb->defaultBackend, "did not return default backend");

	keySetName (mp, "");
	keySetString (mp, "default");
	succeed_if (compare_key (elektraMountGetBackend (kdb, sk)->mountpoint, mp) == 0, "could not find mp");
	succeed_if (compare_key (elektraMountGetMountpoint (kdb, sk), mp) == 0, "could not find mp below");

	keyDel (sk);
	keyDel (mp);

	kdb_del (kdb);
}

KeySet *modules_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/modules", KEY_END),
		KS_END);
}

KeySet *minimal_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		KS_END);
}


void test_minimaltrie()
{
	printf ("Test minimal mount\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, minimal_config(), modules, errorKey) == 0, "could not open minimal config");

	output_warnings (errorKey);
	output_errors (errorKey);

	succeed_if (!kdb->trie, "minimal trie is null");
	succeed_if (kdb->split->size == 0, "minimal trie has size 0");

	keyDel (errorKey);
	ksDel (modules);
	kdb_del (kdb);
}

KeySet *simple_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/simple", KEY_END),
		KS_END);
}

void test_simple()
{
	printf ("Test simple mount\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	Key *mp = keyNew("user/tests/simple", KEY_VALUE, "simple", KEY_END);

	succeed_if (elektraMountOpen(kdb, simple_config(), modules, errorKey) == 0, "could not open trie");

	succeed_if (kdb->split->size == 1, "size of split not correct");
	succeed_if (compare_key (mp, kdb->split->parents[0]) == 0, "parent not correct in split");

	output_warnings (errorKey);
	output_errors (errorKey);

	exit_if_fail (kdb->trie, "kdb->trie was not build up successfully");

	Key *searchKey = keyNew("user", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");


	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/below");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");

	keyDel (errorKey);
	ksDel (modules);
	keyDel (mp);
	keyDel (searchKey);
	kdb_del (kdb);
}

KeySet *set_us()
{
	return ksNew(50,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/user", KEY_END),
		keyNew("system/elektra/mountpoints/user/mountpoint", KEY_VALUE, "user", KEY_END),
		keyNew("system/elektra/mountpoints/system", KEY_END),
		keyNew("system/elektra/mountpoints/system/mountpoint", KEY_VALUE, "system", KEY_END),
		KS_END);

}

void test_us()
{
	printf ("Test mounting of user and system backends\n");

	KDB *kdb = kdb_new();
	KeySet *modules = ksNew(0);
	elektraModulesInit(modules, 0);
	Key *mp;

	KeySet *config = set_us();
	ksAppendKey(config, keyNew("system/elektra/mountpoints", KEY_END));
	succeed_if (elektraMountOpen(kdb, config, modules, 0) == 0, "could not open mount");
	succeed_if (elektraMountDefault(kdb, modules, 0) == 0, "could not mount default backend");

	succeed_if (kdb->split->size == 3, "size of split not correct");
	mp = keyNew("system", KEY_VALUE, "system", KEY_END);
	succeed_if (compare_key (mp, kdb->split->parents[0]) == 0, "parent not correct in split");
	keySetName(mp, "user"); keySetString (mp, "user");
	succeed_if (compare_key (mp, kdb->split->parents[1]) == 0, "parent not correct in split");
	keySetName(mp, "system/elektra"); keySetString (mp, "default");
	succeed_if (compare_key (mp, kdb->split->parents[2]) == 0, "parent not correct in split");
	keyDel (mp);

	Key *key = keyNew("user/anywhere/backend/simple", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, key);

	keyAddBaseName(key, "somewhere"); keyAddBaseName(key, "deep"); keyAddBaseName(key, "below");
	Backend *backend2 = elektraTrieLookup(kdb->trie, key);
	succeed_if (backend == backend2, "should be same backend");

	succeed_if ((mp = backend->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "user"), "wrong mountpoint for backend");
	succeed_if (!strcmp(keyString(mp), "user"), "wrong name for backend");


	keySetName(key, "system/anywhere/tests/backend/two");
	Backend *two = elektraTrieLookup(kdb->trie, key);
	succeed_if (two != backend, "should be differnt backend");

	succeed_if ((mp = two->mountpoint) != 0, "no mountpoint found");
	succeed_if (!strcmp(keyName(mp), "system"), "wrong mountpoint for backend two");
	succeed_if (!strcmp(keyString(mp), "system"), "wrong name for backend");

	keyDel (key);
	elektraModulesClose (modules, 0);
	ksDel (modules);
	kdb_del (kdb);
}


KeySet *cascading_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "/tests/simple", KEY_END),
		KS_END);
}

void keySetCascading(Key *key, const char* name)
{
	size_t size = elektraStrLen(name) + 1;
	if (size < 2) return;

	elektraFree (key->key);
	key->key = elektraMalloc (size);
	key->keySize = size;
	key->key[0] = '/';
	strcpy (key->key+1, name);
}

void test_cascading()
{
	printf ("Test simple mount with cascading\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, cascading_config(), modules, errorKey) == 0, "could not open trie");
	succeed_if (elektraMountDefault(kdb, modules, errorKey) == 0, "could not mount default backend");

	output_warnings (errorKey);
	output_errors (errorKey);

	exit_if_fail (kdb->trie, "kdb->trie was not build up successfully");

	succeed_if (kdb->split->size == 4, "size of split not correct");
	Key *mp = keyNew("system/tests/simple", KEY_VALUE, "simple", KEY_END);
	succeed_if (compare_key (mp, kdb->split->parents[0]) == 0, "parent not correct in split");
	keySetName(mp, "user/tests/simple"); keySetString (mp, "simple");
	succeed_if (compare_key (mp, kdb->split->parents[1]) == 0, "parent not correct in split");
	keyDel (mp);

	// output_split (kdb->split);
	// output_trie (kdb->trie);

	Key *searchKey = keyNew("user", KEY_END);
	Backend *backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");

	keySetName(searchKey, "system");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (!backend, "there should be no backend");


	mp = keyNew("", KEY_VALUE, "simple", KEY_END);
	keySetCascading (mp, "tests/simple");

	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/below");
	Backend *b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "system/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");

	keySetName(searchKey, "system/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "system/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keyDel (errorKey);
	ksDel (modules);
	keyDel (mp);
	keyDel (searchKey);
	kdb_del (kdb);
}


KeySet *root_config(void)
{
	return ksNew(5,
		keyNew("system/elektra/mountpoints", KEY_END),
		keyNew("system/elektra/mountpoints/root", KEY_END),
		keyNew("system/elektra/mountpoints/root/mountpoint", KEY_VALUE, "/", KEY_END),
		keyNew("system/elektra/mountpoints/simple", KEY_END),
		keyNew("system/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user/tests/simple", KEY_END),
		KS_END);
}

void test_root()
{
	printf ("Test mounting with root\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, root_config(), modules, errorKey) == 0, "could not buildup mount");

	output_warnings (errorKey);
	output_errors (errorKey);

	exit_if_fail (kdb->trie, "trie was not build up successfully");

	succeed_if (kdb->split->size == 3, "size of split not correct");
	Key *mp = keyNew("system", KEY_VALUE, "root", KEY_END);
	succeed_if (compare_key (mp, kdb->split->parents[0]) == 0, "parent not correct in split");
	keySetName(mp, "user"); keySetString (mp, "root");
	succeed_if (compare_key (mp, kdb->split->parents[1]) == 0, "parent not correct in split");
	keySetName(mp, "user/tests/simple"); keySetString (mp, "simple");
	succeed_if (compare_key (mp, kdb->split->parents[2]) == 0, "parent not correct in split");

	Key *searchKey = keyNew("", KEY_END);
	Key *rmp = keyNew("", KEY_VALUE, "root", KEY_END);
	keySetCascading (rmp, "");
	Backend *b2 = 0;

	keySetName (searchKey, "user");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (compare_key(b2->mountpoint, rmp) == 0, "mountpoint key not correct");


	Backend *backend = 0;
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");

	keyDel (mp);
	keyDel (rmp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

void test_default()
{
	printf ("Test mounting with default\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, root_config(), modules, errorKey) == 0, "could not buildup mount");
	succeed_if (elektraMountDefault(kdb, modules, errorKey) == 0, "could not mount default backend");

	succeed_if (kdb->split->size == 4, "size of split not correct");
	Key *mp = keyNew("system", KEY_VALUE, "root", KEY_END);
	succeed_if (compare_key (mp, kdb->split->parents[0]) == 0, "parent not correct in split");
	keySetName(mp, "user"); keySetString (mp, "root");
	succeed_if (compare_key (mp, kdb->split->parents[1]) == 0, "parent not correct in split");
	keySetName(mp, "system/elektra"); keySetString (mp, "default");
	succeed_if (compare_key (mp, kdb->split->parents[3]) == 0, "parent not correct in split");
	keySetName(mp, "user/tests/simple"); keySetString (mp, "simple");
	succeed_if (compare_key (mp, kdb->split->parents[2]) == 0, "parent not correct in split");

	/*
	output_warnings (errorKey);
	output_errors (errorKey);
	*/

	exit_if_fail (kdb->trie, "trie was not build up successfully");

	// output_trie (kdb->trie);

	Key *searchKey = keyNew("", KEY_END);
	Key *rmp = keyNew("", KEY_VALUE, "root", KEY_END);
	keySetCascading (rmp, "");
	Backend *b2 = 0;

	keySetName (searchKey, "user");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (compare_key(b2->mountpoint, rmp) == 0, "mountpoint key not correct");


	Backend *backend = 0;
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");

	Key *dmp = keyNew ("", KEY_VALUE, "default", KEY_END);
	keySetName(searchKey, "system/elektra");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 == kdb->defaultBackend, "should be the default backend");
	succeed_if (compare_key(b2->mountpoint, dmp) == 0, "mountpoint key not correct");

	keySetName(searchKey, "system/elektra/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 == kdb->defaultBackend, "should be the default backend");
	succeed_if (compare_key(b2->mountpoint, dmp) == 0, "mountpoint key not correct");

	keyDel (dmp);
	keyDel (mp);
	keyDel (rmp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

void test_modules()
{
	printf ("Test mounting with modules\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, root_config(), modules, errorKey) == 0, "could not buildup mount");
	succeed_if (elektraMountDefault(kdb, modules, errorKey) == 0, "could not mount default backend");
	succeed_if (elektraMountModules(kdb, modules, errorKey) == 0, "could not mount modules");

	output_warnings (errorKey);
	output_errors (errorKey);

	// output_split (kdb->split);

	succeed_if (kdb->split->size > 5, "size of split not correct");
	Key *mp = keyNew("system", KEY_VALUE, "root", KEY_END);
	succeed_if (compare_key (mp, kdb->split->parents[0]) == 0, "parent not correct in split");
	keySetName(mp, "user"); keySetString (mp, "root");
	succeed_if (compare_key (mp, kdb->split->parents[1]) == 0, "parent not correct in split");
	keySetName(mp, "system/elektra"); keySetString (mp, "default");
	succeed_if (compare_key (mp, kdb->split->parents[3]) == 0, "parent not correct in split");
	/*
	keySetName(mp, "system/elektra/modules/default"); keySetString (mp, "modules");
	succeed_if (compare_key (mp, kdb->split->parents[4]) == 0, "parent not correct in split");
	*/

	keySetName(mp, "user/tests/simple"); keySetString (mp, "simple");
	succeed_if (compare_key (mp, kdb->split->parents[2]) == 0, "parent not correct in split");

	exit_if_fail (kdb->trie, "trie was not build up successfully");

	// output_trie (kdb->trie);

	Key *searchKey = keyNew("", KEY_END);
	Key *rmp = keyNew("", KEY_VALUE, "root", KEY_END);
	keySetCascading (rmp, "");
	Backend *b2 = 0;

	keySetName (searchKey, "user");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (compare_key(b2->mountpoint, rmp) == 0, "mountpoint key not correct");


	Backend *backend = 0;
	keySetName(searchKey, "user/tests/simple");
	backend = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (backend, "there should be a backend");
	succeed_if (compare_key(backend->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	succeed_if (compare_key(b2->mountpoint, mp) == 0, "mountpoint key not correct");

	Key *dmp = keyNew ("", KEY_VALUE, "default", KEY_END);
	keySetName(searchKey, "system/elektra");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 == kdb->defaultBackend, "should be the default backend");
	succeed_if (compare_key(b2->mountpoint, dmp) == 0, "mountpoint key not correct");

	keySetName(searchKey, "system/elektra/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 == kdb->defaultBackend, "should be the default backend");
	succeed_if (compare_key(b2->mountpoint, dmp) == 0, "mountpoint key not correct");

	Key *mmp = keyNew ("system/elektra/modules", KEY_VALUE, "modules", KEY_END);
	keyAddBaseName (mmp, "default");

	/*
	keySetName(searchKey, "system/elektra/modules/default");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2, "there should be a backend");
	succeed_if (b2 != kdb->defaultBackend, "should not be the default backend");
	succeed_if (compare_key(b2->mountpoint, mmp) == 0, "mountpoint key for modules not correct");
	*/

	keyDel (mmp);
	keyDel (dmp);
	keyDel (mp);
	keyDel (rmp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

void test_kdbopen()
{
	printf ("Test mounting modules\n");

	Key *errorKey = keyNew("", KEY_END);
	KDB *kdb = kdbOpen (errorKey);

	output_trie (kdb->trie);
	output_split (kdb->split);

	kdbClose (kdb, errorKey);

	output_errors (errorKey);
	output_warnings (errorKey);
	keyDel (errorKey);
}

void test_defaultonly()
{
	printf ("Test mounting with default only\n");

	KDB *kdb = kdb_new();
	Key *errorKey = keyNew(0);
	KeySet *modules = modules_config();
	succeed_if (elektraMountOpen(kdb, minimal_config(), modules, errorKey) == 0, "could not buildup mount");
	succeed_if (elektraMountDefault(kdb, modules, errorKey) == 0, "could not mount default backend");

	succeed_if (kdb->split->size == 2, "size of split not correct");
	Key *mp = keyNew("system", KEY_VALUE, "default", KEY_END);
	succeed_if (compare_key (mp, kdb->split->parents[0]) == 0, "parent not correct in split");
	keySetName(mp, "user"); keySetString (mp, "default");
	succeed_if (compare_key (mp, kdb->split->parents[1]) == 0, "parent not correct in split");

	output_warnings (errorKey);
	output_errors (errorKey);

	succeed_if (!kdb->trie, "trie should be empty");

	Key *searchKey = keyNew("", KEY_END);
	Backend *b2 = 0;

	keySetName (searchKey, "user");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2 == 0, "should be default backend");


	keySetName(searchKey, "user/tests/simple");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2 == 0, "should be default backend");


	keySetName(searchKey, "user/tests/simple/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2 == 0, "should be default backend");


	keySetName(searchKey, "user/tests/simple/deep/below");
	b2 = elektraTrieLookup(kdb->trie, searchKey);
	succeed_if (b2 == 0, "should be default backend");

	keyDel (mp);

	keyDel (searchKey);

	kdb_del (kdb);
	keyDel (errorKey);
	ksDel (modules);
}

int main(int argc, char** argv)
{
	printf("TRIE       TESTS\n");
	printf("==================\n\n");

	init (argc, argv);

	test_mount();
	test_minimaltrie();
	test_simple();
	test_us();
	test_cascading();
	test_root();
	test_default();
	test_modules();
	// test_kdbopen();
	test_defaultonly();

	printf("\ntest_trie RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);

	return nbError;
}
