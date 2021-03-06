/*!
 * @file resultset.h
 * representation of results from a mysql query
 */
#ifndef RJ_DB_MYSQL_RESULTSET_H
#define RJ_DB_MYSQL_RESULTSET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBMYSQLCLIENT

#include <mysql/mysql.h>
#include <vector>
#include "../resultset.h"

namespace rj
{
    namespace db
    {
        namespace mysql
        {
            class session;
            class binding;

            /*!
             * a mysql specific implementation of a result set
             */
            class resultset : public resultset_impl
            {
                template <typename, typename>
                friend class resultset_iterator;

               private:
                std::shared_ptr<MYSQL_RES> res_;
                MYSQL_ROW row_;
                std::shared_ptr<mysql::session> sess_;

               public:
                /*!
                 * @param db the database in use
                 * @param res the query results
                 */
                resultset(const std::shared_ptr<mysql::session> &sess, const std::shared_ptr<MYSQL_RES> &res);

                /* non-copyable boilerplate */
                resultset(const resultset &other) = delete;
                resultset(resultset &&other);
                resultset &operator=(const resultset &other) = delete;
                resultset &operator=(resultset &&other);
                virtual ~resultset();

                /* resultset_impl overrides */
                bool is_valid() const;
                resultset::row_type current_row();
                void reset();
                bool next();
            };

            /*!
             * a mysql specific implementation of a result set using prepared statements
             */
            class stmt_resultset : public resultset_impl
            {
                template <typename, typename>
                friend class resultset_iterator;

               private:
                std::shared_ptr<MYSQL_STMT> stmt_;
                std::shared_ptr<MYSQL_RES> metadata_;
                std::shared_ptr<mysql::session> sess_;
                std::shared_ptr<mysql::binding> bindings_;
                int status_;
                void prepare_results();

                constexpr static const int INVALID = -1;

               public:
                /*!
                 * @param db the database in use
                 * @param stmt the statement being executed
                 */
                stmt_resultset(const std::shared_ptr<mysql::session> &sess, const std::shared_ptr<MYSQL_STMT> &stmt);

                /* non-copyable boilerplate */
                stmt_resultset(const stmt_resultset &other) = delete;
                stmt_resultset &operator=(const stmt_resultset &other) = delete;
                stmt_resultset(stmt_resultset &&other);
                stmt_resultset &operator=(stmt_resultset &&other);

                /* boilerplate */
                virtual ~stmt_resultset();

                /* resultset_impl overrides */
                bool is_valid() const;
                stmt_resultset::row_type current_row();
                void reset();
                bool next();
            };

            namespace helper
            {
                /*! helper to clean up after a query result */
                struct res_delete {
                    void operator()(MYSQL_RES *p) const;
                };
            }
        }
    }
}

#endif

#endif
