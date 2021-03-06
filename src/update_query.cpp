#include "update_query.h"
#include "log.h"
#include "schema.h"

using namespace std;

namespace rj
{
    namespace db
    {
        update_query::update_query(const std::shared_ptr<rj::db::session> &session) : modify_query(session), where_(session->impl(), this)
        {
        }

        /*!
         * @param schema the schema to modify
         */
        update_query::update_query(const std::shared_ptr<schema> &schema) : modify_query(schema), where_(schema->get_session()->impl(), this)
        {
        }

        /*!
         * @param db the database to modify
         * @param tableName the table to modify
         * @param columns the columns to modify
         */
        update_query::update_query(const std::shared_ptr<rj::db::session> &session, const std::string &tableName)
            : modify_query(session), where_(session->impl(), this)
        {
            tableName_ = tableName;
        }

        /*!
         * @param db the database to modify
         * @param columns the columns to modify
         */
        update_query::update_query(const std::shared_ptr<rj::db::session> &session, const std::string &tableName,
                                   const std::vector<std::string> &columns)
            : modify_query(session), where_(session->impl(), this)
        {
            tableName_ = tableName;
            columns_ = columns;
        }

        /*!
         * @param schema the schema to modify
         * @param column the specific columns to modify in the schema
         */
        update_query::update_query(const std::shared_ptr<schema> &schema, const std::vector<std::string> &columns)
            : modify_query(schema), where_(schema->get_session()->impl(), this)
        {
            tableName_ = schema->table_name();
            columns_ = columns;
        }

        update_query::update_query(const update_query &other)
            : modify_query(other), where_(other.where_), columns_(other.columns_), tableName_(other.tableName_)
        {
        }
        update_query::update_query(update_query &&other)
            : modify_query(std::move(other)),
              where_(std::move(other.where_)),
              columns_(std::move(other.columns_)),
              tableName_(std::move(other.tableName_))
        {
        }

        update_query::~update_query()
        {
        }
        update_query &update_query::operator=(const update_query &other)
        {
            modify_query::operator=(other);
            where_ = other.where_;
            columns_ = other.columns_;
            tableName_ = other.tableName_;
            return *this;
        }

        update_query &update_query::operator=(update_query &&other)
        {
            modify_query::operator=(std::move(other));
            where_ = std::move(other.where_);
            columns_ = std::move(other.columns_);
            tableName_ = std::move(other.tableName_);
            return *this;
        }

        bool update_query::is_valid() const
        {
            return query::is_valid() && !tableName_.empty();
        }

        string update_query::table() const
        {
            return tableName_;
        }

        update_query &update_query::table(const string &value)
        {
            tableName_ = value;
            return *this;
        }

        string update_query::to_string() const
        {
            string buf;

            buf += "UPDATE ";
            buf += tableName_;

            if (columns_.size() > 0) {
                buf += " SET ";
                buf += session_->impl()->join_params(columns_, op::type_values[op::EQ]);
            }

            if (!where_.empty()) {
                buf += " WHERE ";
                buf += where_.to_string();
            } else {
                log::warn("empty where clause for update query");
            }

            buf += ";";

            return buf;
        }

        update_query &update_query::columns(const vector<string> &columns)
        {
            columns_ = columns;
            return *this;
        }

        vector<string> update_query::columns() const
        {
            return columns_;
        }

        where_builder &update_query::where()
        {
            return where_;
        }
        where_builder &update_query::where(const where_clause &value)
        {
            where_.where_clause::reset(value);
            return where_;
        }

        where_builder &update_query::where(const string &value)
        {
            where_.where_clause::reset(value);
            return where_;
        }

        where_builder &update_query::where(const sql_operator &value)
        {
            where_.reset(value);
            return where_;
        }

        update_query &update_query::values(const std::vector<sql_value> &value)
        {
            bindable::bind(value);
            return *this;
        }

        update_query &update_query::values(const std::unordered_map<std::string, sql_value> &value)
        {
            bindable::bind(value);
            return *this;
        }
    }
}
