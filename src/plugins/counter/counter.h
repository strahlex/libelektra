/***************************************************************************
                     counter.c  -  Skeleton of a plugin
                             -------------------
    begin                : Fri May 21 2010
    copyright            : (C) 2010 by Markus Raab
    email                : elektra@markus-raab.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This is the skeleton of the methods you'll have to implement in order *
 *   to provide a valid plugin.                                            *
 *   Simple fill the empty functions with your code and you are            *
 *   ready to go.                                                          *
 *                                                                         *
 ***************************************************************************/


#ifndef ELEKTRA_PLUGIN_COUNTER_H
#define ELEKTRA_PLUGIN_COUNTER_H

#include <kdbplugin.h>


int elektraCounterOpen(Plugin *handle, Key *errorKey);
int elektraCounterClose(Plugin *handle, Key *errorKey);
int elektraCounterGet(Plugin *handle, KeySet *ks, Key *parentKey);
int elektraCounterSet(Plugin *handle, KeySet *ks, Key *parentKey);
int elektraCounterError(Plugin *handle, KeySet *ks, Key *parentKey);

Plugin *ELEKTRA_PLUGIN_EXPORT(counter);

#endif
