#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <bandit/bandit.h>
#include "db.test.h"
#include "log.h"
#include "sqldb.h"
#include "sqlite/session.h"
#include "testicle.h"

using namespace bandit;
using namespace std;

#if !defined(HAVE_LIBMYSQLCLIENT) && !defined(HAVE_LIBSQLITE3) && !defined(HAVE_LIBPQ)
#error "Mysql, postgres or sqlite is not installed on the system"
#endif

int main(int argc, char *argv[])
{
    int opt = getopt(argc, argv, "l::");

    if (opt == 'l') {
        rj::db::log::set_level(optarg);
    } else {
        rj::db::log::set_level(rj::db::log::Error);
    }

    register_test_sessions();

#ifdef HAVE_LIBSQLITE3
#ifdef TEST_SQLITE
    puts("running sqlite3 tests");

    current_session = rj::db::sqldb::create_session("file://testdb.db");

    auto sqlite_session = current_session->impl<rj::db::sqlite::session>();

    // run the uncached test
    if (bandit::run(argc, argv)) {
        return 1;
    }
#endif
#else
    cout << "Sqlite not supported" << endl;
#endif

#ifdef HAVE_LIBMYSQLCLIENT
#ifdef TEST_MYSQL
    puts("running mysql tests");

    auto uri_s = get_env_uri("MYSQL_URI", "mysql://localhost/test");
    current_session = rj::db::sqldb::create_session(uri_s);
    cout << "connecting to " << uri_s << endl;

    if (bandit::run(argc, argv)) {
        return 1;
    }
    mysql_library_end();
#endif
#else
    cout << "Mysql not supported" << endl;
#endif

#ifdef HAVE_LIBPQ
#ifdef TEST_POSTGRES
    puts("running postgres tests");

    auto uri_s = get_env_uri("POSTGRES_URI", "postgres://localhost/test");
    current_session = rj::db::sqldb::create_session(uri_s);
    cout << "connecting to " << uri_s << endl;

    if (bandit::run(argc, argv)) {
        return 1;
    }
#endif
#else
    cout << "Postgres not supported" << endl;
#endif

    current_session = nullptr;

    return 0;
}
