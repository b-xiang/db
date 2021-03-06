
#include "where_clause.h"
#include <sstream>

using namespace std;

namespace rj
{
    namespace db
    {
        sql_operator::sql_op() : rvalue()
        {
        }

        sql_operator::sql_op(const sql_operator &other)
        {
            copy(other);
        }

        sql_operator::sql_op(sql_operator &&other)
        {
            move(std::move(other));
        }

        sql_operator &sql_operator::operator=(const sql_operator &other)
        {
            copy(other);
            return *this;
        }

        sql_operator &sql_operator::operator=(sql_operator &&other)
        {
            move(std::move(other));
            return *this;
        }

        sql_operator::~sql_op()
        {
            switch (type) {
                case op::EQ:
                case op::NEQ:
                case op::LIKE:
                    rvalue.~sql_value();
                    break;
                case op::IN:
                    rvalues.~vector<sql_value>();
                    break;
                case op::BETWEEN:
                    rrange.~pair<sql_value, sql_value>();
                    break;
            }
        }

        void sql_operator::copy(const sql_operator &other)
        {
            lvalue = other.lvalue;
            type = other.type;
            switch (type) {
                case op::EQ:
                case op::NEQ:
                case op::LIKE:
                    new (&rvalue) sql_value(other.rvalue);
                    break;
                case op::IN:
                    new (&rvalues) std::vector<sql_value>(other.rvalues);
                    break;
                case op::BETWEEN:
                    new (&rrange) std::pair<sql_value, sql_value>(other.rrange);
                    break;
            }
        }

        void sql_operator::move(sql_operator &&other)
        {
            lvalue = std::move(other.lvalue);
            type = std::move(other.type);
            switch (type) {
                case op::EQ:
                case op::NEQ:
                case op::LIKE:
                    new (&rvalue) sql_value(std::move(other.rvalue));
                    break;
                case op::IN:
                    new (&rvalues) std::vector<sql_value>(std::move(other.rvalues));
                    break;
                case op::BETWEEN:
                    new (&rrange) std::pair<sql_value, sql_value>(std::move(other.rrange));
                    break;
            }
        }
        namespace op
        {
            sql_operator equals(const sql_value &lvalue, const sql_value &rvalue)
            {
                sql_operator op;
                op.lvalue = lvalue;
                op.rvalue = rvalue;
                op.type = op::EQ;
                return op;
            }
            sql_operator nequals(const sql_value &lvalue, const sql_value &rvalue)
            {
                sql_operator op;
                op.lvalue = lvalue;
                op.rvalue = rvalue;
                op.type = op::NEQ;
                return op;
            }
            sql_operator like(const sql_value &lvalue, const std::string &rvalue)
            {
                sql_operator op;
                op.lvalue = lvalue;
                op.rvalue = rvalue;
                op.type = op::LIKE;
                return op;
            }
            sql_operator in(const sql_value &lvalue, const std::vector<sql_value> &rvalue)
            {
                sql_operator op;
                op.lvalue = lvalue;
                op.rvalues = rvalue;
                op.type = op::IN;
                return op;
            }
            sql_operator between(const sql_value &lvalue, const sql_value rvalue1, const sql_value rvalue2)
            {
                sql_operator op;
                op.lvalue = lvalue;
                op.rrange = {rvalue1, rvalue2};
                op.type = op::BETWEEN;
                return op;
            }
        }
        where_clause::where_clause()
        {
        }

        where_clause::where_clause(const string &value) : value_(value)
        {
        }

        where_clause::where_clause(const where_clause &other) : value_(other.value_), and_(other.and_), or_(other.or_)
        {
        }

        where_clause::where_clause(where_clause &&other) : value_(std::move(other.value_)), and_(std::move(other.and_)), or_(std::move(other.or_))
        {
        }

        where_clause::~where_clause()
        {
        }

        where_clause &where_clause::operator=(const where_clause &other)
        {
            value_ = other.value_;
            and_ = other.and_;
            or_ = other.or_;

            return *this;
        }

        where_clause &where_clause::operator=(where_clause &&other)
        {
            value_ = std::move(other.value_);
            and_ = std::move(other.and_);
            or_ = std::move(other.or_);

            return *this;
        }

        string where_clause::to_string() const
        {
            string buf;

            if (and_.size() > 0 && or_.size() > 0) {
                buf += "(";
            }

            buf += value_;

            if (and_.size() > 0) {
                buf += " AND ";
                auto it = and_.begin();
                while (it < and_.end() - 1) {
                    buf += it->to_string();
                    buf += " AND ";
                    ++it;
                }
                if (it != and_.end()) {
                    buf += it->to_string();
                }
                if (or_.size() > 0) {
                    buf += ")";
                }
            }

            if (or_.size() > 0) {
                if (and_.size() > 0) {
                    buf += " OR (";
                } else {
                    buf += " OR ";
                }
                auto it = or_.begin();
                while (it < or_.end() - 1) {
                    buf += it->to_string();
                    buf += " OR ";
                    ++it;
                }
                if (it != or_.end()) {
                    buf += it->to_string();
                }
                if (and_.size() > 0) {
                    buf += ")";
                }
            }
            return buf;
        }

        bool where_clause::empty() const
        {
            return value_.empty() && and_.empty() && or_.empty();
        }

        where_clause::operator string()
        {
            return to_string();
        }

        where_clause &where_clause::operator&&(const where_clause &value)
        {
            if (value_.empty())
                value_ = value.to_string();
            else
                and_.push_back(value);
            return *this;
        }
        where_clause &where_clause::operator&&(const string &value)
        {
            if (value_.empty())
                value_ = value;
            else
                and_.push_back(where_clause(value));

            return *this;
        }
        where_clause &where_clause::operator||(const where_clause &value)
        {
            if (value_.empty())
                value_ = value.to_string();
            else
                or_.push_back(value);
            return *this;
        }
        where_clause &where_clause::operator||(const string &value)
        {
            if (value_.empty())
                value_ = value;
            else
                or_.push_back(where_clause(value));
            return *this;
        }

        void where_clause::reset()
        {
            value_.clear();
            and_.clear();
            or_.clear();
        }

        void where_clause::reset(const std::string &value)
        {
            reset();
            value_ = value;
        }
        void where_clause::reset(const where_clause &other)
        {
            value_ = other.value_;
            and_ = other.and_;
            or_ = other.or_;
        }

        ostream &operator<<(ostream &out, const where_clause &where)
        {
            out << where.to_string();
            return out;
        }


        where_builder::where_builder(const std::shared_ptr<session_impl> &session, bindable *binder) : session_(session), binder_(binder)
        {
            assert(binder != nullptr);
        }

        where_builder::where_builder(const where_builder &other) : where_clause(other), session_(other.session_), binder_(other.binder_)
        {
        }

        where_builder::where_builder(where_builder &&other)
            : where_clause(std::move(other)), session_(std::move(other.session_)), binder_(other.binder_)
        {
        }

        where_builder &where_builder::operator=(const where_builder &other)
        {
            where_clause::operator=(other);
            session_ = other.session_;
            binder_ = other.binder_;
            return *this;
        }

        where_builder &where_builder::operator=(where_builder &&other)
        {
            where_clause::operator=(std::move(other));
            session_ = std::move(other.session_);
            binder_ = other.binder_;
            return *this;
        }

        where_builder::~where_builder()
        {
        }

        where_builder &where_builder::bind(size_t index, const sql_operator &value)
        {
            switch (value.type) {
                case op::EQ:
                case op::NEQ:
                case op::LIKE:
                    binder_->bind(index, value.rvalue);
                    break;
                case op::IN:
                    for (size_t i = 0; i < value.rvalues.size(); i++) {
                        binder_->bind(index + i, value.rvalues[i]);
                    }
                    break;
                case op::BETWEEN:
                    binder_->bind(index, value.rrange.first);
                    binder_->bind(index, value.rrange.second);
                    break;
            }
            return *this;
        }

        std::string where_builder::to_sql(size_t index, const sql_operator &value)
        {
            return value.lvalue.to_string() + std::string(" ") + op::type_values[value.type] + std::string(" ") + session_->bind_param(index);
        }

        where_builder &where_builder::bind(size_t index, const sql_value &value)
        {
            binder_->bind(index, value);
            return *this;
        }
        where_builder &where_builder::bind(const std::string &name, const sql_value &value)
        {
            binder_->bind(name, value);
            return *this;
        }

        void where_builder::reset(const sql_operator &value)
        {
            size_t index = binder_->num_of_bindings() + 1;
            where_clause::reset(to_sql(index, value));
            bind(index, value);
        }

        size_t where_builder::num_of_bindings() const
        {
            return binder_->num_of_bindings();
        }

        /*!
         * Appends and AND part to this where clause
         * @param value   the sql to append
         */
        where_builder &where_builder::operator&&(const sql_operator &value)
        {
            size_t index = binder_->num_of_bindings() + 1;
            where_clause::operator&&(to_sql(index, value));
            return bind(index, value);
        }
        /*!
         * Appends and OR part to this where clause
         * @param value   the sql to append
         */
        where_builder &where_builder::operator||(const sql_operator &value)
        {
            size_t index = binder_->num_of_bindings() + 1;
            where_clause::operator&&(to_sql(index, value));
            return bind(index, value);
        }
    }
}
