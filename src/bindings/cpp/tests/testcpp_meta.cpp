#include <tests.hpp>

#include <vector>
#include <string>
#include <stdexcept>


void test_basic()
{
	cout << "testing metainfo" << endl;
	Key test;

	test.setUID(50);
	succeed_if (test.getUID() == 50, "could not set UID");
	succeed_if (test.getMeta<uid_t>("uid") == 50, "could not set UID");

	test.setMeta<uid_t>("uid", 80);
	succeed_if (test.getUID() == 80, "could not set UID");
	succeed_if (test.getMeta<uid_t>("uid") == 80, "could not set UID");

	test.setGID(50);
	succeed_if (test.getGID() == 50, "could not set GID");
	succeed_if (test.getMeta<gid_t>("gid") == 50, "could not set GID");

	succeed_if (test.getMode() == 0664, "not correct default mode");
	try {
		test.getMeta<mode_t>("mode");
		succeed_if (0, "failed, should raise exception");
	} catch (KeyNoSuchMeta const &e) {
		succeed_if (1, "exception raised successfully");
	}
	test.setDir ();
	succeed_if (test.isDir(), "is not dir");
	succeed_if (test.getMode() == 0775, "not correct default mode for dir");

	//octal problem for mode:
	succeed_if (test.getMeta<mode_t>("mode") == 775, "not correct default mode for dir");

	test.setMTime (200);
	succeed_if (test.getMTime() == 200, "could not set MTime");
	succeed_if (test.getMeta<time_t>("mtime") == 200, "could not set mtime");

	test.setATime (200);
	succeed_if (test.getATime() == 200, "could not set ATime");
	succeed_if (test.getMeta<time_t>("atime") == 200, "could not set atime");

	test.setCTime (200);
	succeed_if (test.getCTime() == 200, "could not set CTime");
	succeed_if (test.getMeta<time_t>("ctime") == 200, "could not set ctime");

	test.setMeta<int>("myint", 333);
	succeed_if (test.getMeta<int>("myint") == 333, "could not set other meta");

	test.setMeta<double>("mydouble", 333.3);
	succeed_if (test.getMeta<double>("mydouble") == 333.3, "could not set other meta");

	test.setMeta<std::string>("mystr", "str");
	succeed_if (test.getMeta<std::string>("mystr") == "str", "could not set other meta");

	const ckdb::Key *cmeta = test.getMeta<const ckdb::Key*>("mystr");
	succeed_if (!strcmp(static_cast<const char*>(ckdb::keyValue(cmeta)), "str"), "could not set other meta");

	const ckdb::Key *nmeta = test.getMeta<const ckdb::Key*>("not available");
	succeed_if (nmeta == 0, "not available meta data did not give a null pointer");

	const Key meta = test.getMeta<const Key>("mystr");
	succeed_if (meta, "null key");
	succeed_if (meta.getString() == "str", "could not set other meta");

	const Key xmeta = test.getMeta<const Key>("not available");
	succeed_if (!xmeta, "not a null key");

	const char * str = test.getMeta<const char*>("mystr");
	succeed_if (!strcmp(str, "str"), "could not get meta as c-string");

	const char * nstr = test.getMeta<const char*>("not available");
	succeed_if (nstr == 0, "did not get null pointer on not available meta data");

	try {
		test.getMeta<int>("not available");
		succeed_if (0, "exception did not raise");
	} catch (KeyNoSuchMeta const& e)
	{
		succeed_if (1, "no such meta data");
	}

	try {
		test.getMeta<std::string>("not available");
		succeed_if (0, "exception did not raise");
	} catch (KeyNoSuchMeta const& e)
	{
		succeed_if (1, "no such meta data");
	}
}

void test_iter()
{
	cout << "testing iterating" << endl;

	Key k ("user/metakey",
		KEY_META, "a", "meta",
		KEY_META, "b", "my",
		KEY_META, "c", "other",
		KEY_END);

	Key meta; //key = keyNew(0)

	succeed_if (meta, "key is a not null key");

	Key end = static_cast<ckdb::Key*>(0); // key = 0
	succeed_if (!end, "key is a null key");

	k.rewindMeta();
	while (meta = k.nextMeta())
	{
		// cout << meta.getName() << " " << meta.getString() << endl;
	}

	int count = 0;
	k.rewindMeta();
	while (meta = k.nextMeta()) count ++;
	succeed_if (count == 3, "Not the correct number of meta data");

	k.setMeta("d", "more");
	k.setMeta("e", "even more");

	count = 0;
	k.rewindMeta();
	while (meta = k.nextMeta()) count ++;
	succeed_if (count == 5, "Not the correct number of meta data");
}

void test_copy()
{
	cout << "testing copy meta" << std::endl;

	Key k ("user/metakey",
			KEY_META, "", "meta value",
			KEY_META, "a", "a meta value",
			KEY_META, "b", "b meta value",
			KEY_META, "c", "c meta value",
			KEY_END);
	Key c;

	c.copyMeta(k, "a");

	succeed_if (k.getMeta<const ckdb::Key*>("a") == c.getMeta<const ckdb::Key*>("a"), "copy meta did not work");

	c.copyMeta(k, "");

	succeed_if (k.getMeta<const ckdb::Key*>("") == c.getMeta<const ckdb::Key*>(""), "copy meta did not work");

	k.setMeta<int>("a", 420);

	succeed_if (k.getMeta<int>("a") == 420, "could not get value set before");
	try {
		c.getMeta<int>("a");
		succeed_if (0, "exception did not raise");
	} catch (KeyBadMeta const& e)
	{
		succeed_if (1, "no such meta data");
	}

	c.copyMeta(k, "a");
	succeed_if (c.getMeta<int>("a") == 420, "could not get value copied before");
	succeed_if (k.getMeta<const ckdb::Key*>("a") == c.getMeta<const ckdb::Key*>("a"), "copy meta did not work");

	c.copyMeta(k, "a");
	succeed_if (c.getMeta<int>("a") == 420, "could not get value copied before (again)");
	succeed_if (k.getMeta<const ckdb::Key*>("a") == c.getMeta<const ckdb::Key*>("a"),
			"copy meta did not work (again)");

	Key d;
	Key meta;

	k.rewindMeta();
	while (meta = k.nextMeta())
	{
		d.copyMeta(k, meta.getName());
	}

	succeed_if (d.getMeta<std::string>("a") == "420", "did not copy meta value in the loop");
	succeed_if (k.getMeta<const ckdb::Key*>("a") == d.getMeta<const ckdb::Key*>("a"),
			"copy meta did not work in the loop");

	succeed_if (d.getMeta<std::string>("") == "meta value", "did not copy meta value in the loop");
	succeed_if (k.getMeta<const ckdb::Key*>("") == d.getMeta<const ckdb::Key*>(""),
			"copy meta did not work in the loop");

	succeed_if (d.getMeta<std::string>("b") == "b meta value", "did not copy meta value in the loop");
	succeed_if (k.getMeta<const ckdb::Key*>("b") == d.getMeta<const ckdb::Key*>("b"),
			"copy meta did not work in the loop");

	succeed_if (d.getMeta<std::string>("c") == "c meta value", "did not copy meta value in the loop");
	succeed_if (k.getMeta<const ckdb::Key*>("c") == d.getMeta<const ckdb::Key*>("c"),
			"copy meta did not work in the loop");
}

void test_string()
{
	cout << "testing string inside meta" << std::endl;
	Key k("user/anything",
			KEY_META, "", "meta value",
			KEY_META, "a", "a meta value",
			KEY_META, "b", "b meta value",
			KEY_META, "c", "c meta value",
			KEY_END);

	succeed_if (k.getMeta<string>("a") == "a meta value", "could not get meta value");

	Key m = k.getMeta<const Key> ("a");
	succeed_if (m, "could not get meta key");
	succeed_if (m.getString()  == "a meta value", "could not get meta string");

	Key m1 = k.getMeta<const Key> ("x");
	succeed_if (!m1, "could not get meta key");
	succeed_if (m1.getKey() == 0, "key should be 0");
	succeed_if (m1.getString()  == "", "could not get meta string");
}

int main()
{
	cout << "KEY META TESTS" << endl;
	cout << "===============" << endl << endl;

	test_basic();
	test_iter();
	test_copy();
	test_string();

	cout << endl;
	cout << "testcpp_meta RESULTS: " << nbTest << " test(s) done. " << nbError << " error(s)." << endl;
	return nbError;
}