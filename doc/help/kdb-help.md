kdb-help(1) -- Show man page for elektra tools
==============================================

## SYNOPSIS

`kdb help <tool>`
`kdb --help <tool>`
`kdb -H <tool>`

## DESCRIPTION

Show a man page for one of Elektra's tools.
Note that `kdb <tool> --help` might have a different behaviour, depending on the tool.

## KDB

- `/sw/kdb/current/man`:
  The man(1) utility to be used.
  Defaults to /usr/bin/man

## EXAMPLES

Show how to set keys:
	`kdb help set`

Use the info program as man page viewer for the current user:
	`kdb set user/sw/kdb/current/man /usr/bin/info`
