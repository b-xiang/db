/*!
 * implementation of a database record
 * @copyright ryan jennings (arg3.com), 2013 under LGPL
 */
#ifndef _ARG3_DB_BASE_RECORD_H_
#define _ARG3_DB_BASE_RECORD_H_

#include "select_query.h"
#include "modify_query.h"
#include "delete_query.h"
#include "schema.h"
#include <memory>

namespace arg3
{
    namespace db
    {
        class record_db;
        class row;

        /*!
         * an active-record (ish) pattern
         */
        template<typename T>
        class base_record
        {
        private:
            std::shared_ptr<schema> schema_;
            map<string, sql_value> values_;
            string idColumnName_;
        public:

            /*!
             * @param db the database the record uses
             * @param tablename the table in the database to use
             * @param idColumnName the name of the id column in the table
             */
            base_record(sqldb *db, const string &tablename, const string &idColumnName) : schema_(db->schemas()->get(tablename)), idColumnName_(idColumnName)
            {
                assert(schema_ != nullptr);
            }

            /*!
             * @param schema the schema to operate on
             * @param columnName the name of the id column in the schema
             */
            base_record(std::shared_ptr<schema> schema, const string &columnName) : schema_(schema), idColumnName_(columnName)
            {
                assert(schema_ != nullptr);
            }

            /*!
             * @param schema the schema to operate on
             * @param columnName the name of the column the id column in the schema
             * @param value the value of the id column
             */
            template<typename V>
            base_record(std::shared_ptr<schema> schema, const string &columnName, V value) : base_record(schema, columnName)
            {
                set(idColumnName_, value);
                refresh(); // load up from database
            }

            /*!
             * @param db the database to operate on
             * @param tableName the name of the table in the database
             * @param columnName the name of the id column in the table
             * @param value the value of the id column
             */
            template<typename V>
            base_record(sqldb *db, const string &tableName, const string &columnName, V value) : base_record(db, tableName, columnName)
            {
                set(idColumnName_, value);
                refresh();
            }

            /*!
             * construct with values from a database row
             */
            base_record(std::shared_ptr<schema> schema, const string &columnName, const row &values) : base_record(schema, columnName)
            {
                init(values);
            }

            /*!
             * construct with values from a database row
             */
            base_record(sqldb *db, const string &tableName, const string &columnName, const row &values) : base_record(db, tableName, columnName)
            {
                init(values);
            }

            /*!
             * copy constructor
             */
            base_record(const base_record &other) : schema_(other.schema_), values_(other.values_), idColumnName_(other.idColumnName_)
            {}

            /*!
             * move constructor
             */
            base_record(base_record &&other) : schema_(other.schema_), values_(std::move(other.values_)), idColumnName_(std::move(other.idColumnName_))
            {
            }

            virtual ~base_record()
            {

            }

            /*!
             * assignment operator
             */
            base_record &operator=(const base_record &other)
            {
                values_ = other.values_;
                schema_ = other.schema_;
                idColumnName_ = other.idColumnName_;

                return *this;
            }

            /*!
             * move assignment operator
             */
            base_record &operator=(base_record && other)
            {
                values_ = std::move(other.values_);
                schema_ = std::move(other.schema_);
                idColumnName_ = std::move(other.idColumnName_);

                other.schema_ = nullptr;

                return *this;
            }

            /*!
             * sets the id column of the record
             */
            void set_id(const sql_value &value)
            {
                set(idColumnName_, value);
            }

            /*!
             * returns the value of the id column in the record
             */
            sql_value id() const
            {
                return get(idColumnName_);
            }

            /*!
             * initializes with values from a database row
             */
            void init(const row &values)
            {
                for (auto v = values.begin(); v != values.end(); v++)
                {
                    set(v.name(), v->to_value());
                }
            }

            /*!
             * check if the record internals are valid
             */
            bool is_valid() const
            {
                return schema()->is_valid();
            }

            /*!
             * returns the name of the id column for this record
             */
            string id_column_name() const
            {
                return idColumnName_;
            }

            /*!
             * returns the schema for this record
             */
            shared_ptr<schema> schema() const
            {
                return schema_;
            }

            /*!
             * saves this instance
             */
            bool save()
            {
                modify_query query(schema());

                int index = 1;

                // bind the column values
                for (auto & column : schema()->columns())
                {
                    auto value = get(column.name);

                    query.bind_value(index, value);

                    index++;
                }

                return query.execute();

            }

            /*!
             * gets a value specified by column name
             */
            sql_value get(const string &name) const
            {
                if (!has(name))
                    return sql_value();

                return values_.at(name);
            }

            /*!
             * check for the existance of a column by name
             */
            bool has(const string &name) const
            {

                return values_.size() > 0 && values_.count(name) > 0;
            }

            /*!
             * sets a string for a column name
             */
            void set(const string &name, const sql_value &value)
            {
                values_[name] = value;
            }

            /*!
             * unsets / removes a column
             */
            void unset(const string &name)
            {
                values_.erase(name);
            }

            /*!
             * looks up and returns all objects of a base_record type
             */

            vector<shared_ptr<T>> find_all()
            {
                auto query = select_query(schema());

                auto results = query.execute();

                /* convert sql rows to objects */
                vector<shared_ptr<T>> items;

                for (auto & row : results)
                {
                    items.push_back(make_shared<T>(row));
                }

                return items;
            }

            /*!
             * finds a single record by its id column
             */
            template<typename V>
            shared_ptr<T> find_by_id(V value)
            {
                auto query = select_query(schema());

                query.where(idColumnName_ + " = ?");

                query.bind_value(1, value);

                auto results = query.execute();

                auto it = results.begin();

                if (it != results.end())
                {
                    return make_shared<T>(*it);
                }

                throw record_not_found_exception();
            }

            /*!
             * find records by a column and its value
             */
            template<typename V>
            vector<shared_ptr<T>> find_by(const string &name, const V &value)
            {
                auto query = select_query(schema());

                query.where(name + " = ?");

                query.bind(1, value);

                auto results = query.execute();

                /* convert sql rows to objects */
                vector<shared_ptr<T>> items;

                for (auto & row : results)
                {
                    items.push_back(make_shared<T>(row));
                }

                return items;
            }

            /*!
             * refreshes from the database
             */
            bool refresh()
            {
                return refresh_by(idColumnName_);
            }

            /*!
             * refreshes by a column name
             */
            bool refresh_by(const string &name)
            {
                auto query = select_query(schema());

                query.where(name + " = ?");

                query.limit("1");

                query.bind_value(1, get(name));

                auto result = query.execute();

                if (!result.next())
                    return false;

                init(*result);

                return true;
            }

            /*!
             * deletes this record from the database
             */
            bool de1ete()
            {
                auto query = delete_query(schema());

                query.where(idColumnName_ + " = ?");

                query.bind_value(1, id());

                return query.execute();
            }
        };

    }

}

#endif