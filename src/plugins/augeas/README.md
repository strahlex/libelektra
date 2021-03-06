- infos = Information about augeas plugin is in keys below
- infos/author = Felix Berlakovich <elektra@berlakovich.net>
- infos/licence = BSD
- infos/needs =
- infos/provides = storage
- infos/recommends = glob keytometa
- infos/placements = getstorage setstorage
- infos/description = reading and writing configurations via libaugeas

## INTRODUCTION ##

This is a plugin for reading and writing configuration files with help from Augeas.
The plugin should be able to read all configuration files for which an Augeas lens exists.
However, not all stock lenses of Augeas have been tested yet.
A detailed description of the lens language and a tutorial on how to write new lenses"
can be found at http://augeas.net/

## DEPENDENCIES ##

- `libaugeas-dev` #note: You need version 0.16 or higher which is not in debian wheezy

## INSTALLATION ##

If you have installed Augeas manually, it may be neccessary to update the ld configuration. This is especially
true if an older version of Augeas is installed also. Such a situation may lead to an error similar to this:

/usr/lib/libaugeas.so.0: version `AUGEAS_0.16.0' not found (required by kdb)

This is because ld tries to link /usr/lib/libaugeas.so.0 which is an older version of Augeas. Simply add
the path to the newer library to your ld search paths (consult your system documentation on how to do this)


## MOUNTING AND CONFIGURATION ##

In order to mount the hosts file with the augeas plugin, issue the
following command:

	kdb mount /etc/hosts system/hosts augeas lens=Hosts.lns

The value of the plugin configuration option "lens" should be the
module name of the lens (Hosts in the example) with a '.lns' suffix.
Depending on your distribution and kind of installation, lenses can
be found at `/usr/share/augeas/lenses/dist`,
`/usr/local/share/augeas/lenses/dist`, or something similar.
The lens module name is equal to the filename without extension in pascal notation.
For example, the lens `/usr/share/augeas/lenses/dist/hosts.aug` contains the module Hosts.

Note that, without configuring the plugin to use a lens, the plugin
would bail out an error on the first usage:

	kdb ls system/hosts
	The command ls terminated unsuccessfully with the info: Error (#85) occurred!
	Description: an Augeas error occurred
	Ingroup: plugin
	Module: storage
	At: /path/augeas.c:166
	Reason: Lens not found

because the plugin would not know which lens to use to read and write the configuration.
For that reason, the lens configuration option was supplied together with the mount command.



## RESTRICTIONS ##

### Inner node values ###
Currently no Augeas lens supports values for inner nodes.
Unfortunately no validation plugin exists yet that would prevent such modifications early:

	kdb set system/hosts/1 somevalue
	The command set terminated unsuccessfully with the info: Error (#85) occurred!
	Description: an Augeas error occurred
	Ingroup: plugin
	Module: storage
	At: /path/augeas.c:166
	Reason: Malformed child node '1'

The operation simply fails with an undescriptive error.

### Leaky abstraction of order ###
Most Augeas lenses require subtrees to be in a specific order. For example the hosts lens requires the ipaddr node
of an entry to precede the canonical node. Unfortunately the Augeas storage plugin has no knowledge about this required
order. Therefore the correct order must be ensured via order meta keys. Otherwise saving the KeySet may fail. As an example
consider the following kdb shell script:

	kdbGet system/hosts
	keySetName system/hosts/6
	ksAppendKey
	keySetName system/hosts/6/ipaddr
	keySetString 14.14.14.14
	ksAppendKey
	keySetName system/hosts/6/canonical
	keySetString newhost
	ksAppendKey
	kdbSet system/hosts

This fails with an error similar to this
	
	Description: an Augeas error occurred
	Ingroup: plugin
	Module: storage
	At: /path/augeas.c:179
	Reason: Failed to match
	some augeas match expression
	with tree
	{ \"canonical\" = \"newhost\" } { \"ipaddr\" = \"14.14.14.14\" }

Wheras the following script succeeds due to the correct order

	kdbGet system/hosts
	keySetName system/hosts/6
	ksAppendKey
	keySetName system/hosts/6/ipaddr
	keySetString 14.14.14.14
	keySetMeta order 100
	ksAppendKey
	keySetName system/hosts/6/canonical
	keySetString newhost
	keySetMeta order 110
	ksAppendKey
	kdbSet system/hosts


## PLANNED IMPROVEMENTS ##

* a validation plugin preventing inner node values,
