- infos = Information about the logchange plugin is in keys below
- infos/author = Name <name@libelektra.org>
- infos/licence = BSD
- infos/needs =
- infos/provides =
- infos/placements = postgetstorage postcommit
- infos/description =

## Purpose ##

The purpose of this plugin is to demonstrate how one can
be notified of every removed, added or changed key easily.


## Usage ##

Prints every added, changed or deleted key on the console.
To use it, add it during mounting:

    kdb mount logchange.dump user/logchange dump logchange
