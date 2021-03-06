#include <kdb.h>
#include <stdio.h>

int main()
{
	KeySet *myConfig = ksNew(0, KS_END);
	Key *key = keyNew("/sw/MyApp", KEY_CASCADING_NAME, KEY_END);
	KDB *handle = kdbOpen(key);

	kdbGet(handle, myConfig, key);
	// to get an intention of proper error handling see kdbget_error.c

	keyDel (key);

	Key * result = ksLookupByName (myConfig,"/sw/MyApp/Tests/TestKey1", 0);

	const char * key_name = keyName(result);
	const char * key_value = keyString(result);
	const char * key_comment = keyString(keyGetMeta(result, "comment"));
	printf("key: %s value: %s comment: %s\n", key_name, key_value, key_comment);

	ksDel (myConfig); // delete the in-memory configuration


	// maybe you want kdbSet() myConfig here

	kdbClose(handle, 0); // no more affairs with the key database.
}
