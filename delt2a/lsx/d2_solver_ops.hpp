#include "d2_solver_types.hpp"

namespace d2::lsx
{
    // Operators (fun)

    // Variable (arithmetic)

    constexpr Variable operator*(Variable var, result_type cf)
    {
        return Variable(var, var.coefficient() * cf);
    }
    constexpr Variable operator/(Variable var, result_type cf)
    {
        return Variable(var, var.coefficient() / cf);
    }
    constexpr Variable operator*(result_type cf, Variable var)
    {
        return Variable(var, var.coefficient() * cf);
    }
    constexpr Variable operator/(result_type cf, Variable var)
    {
        return Variable(var, var.coefficient() / cf);
    }

    constexpr Expression operator+(Variable var, result_type cf)
    {
        return Expression::from(var, Variable(cf), 1);
    }
    constexpr Expression operator-(Variable var, result_type cf)
    {
        return Expression::from(var, Variable(cf), -1);
    }
    constexpr Expression operator+(result_type cf, Variable var)
    {
        return Expression::from(Variable(cf), var, 1);
    }
    constexpr Expression operator-(result_type cf, Variable var)
    {
        return Expression::from(Variable(cf), var, -1);
    }

    constexpr Expression operator+(Variable lhs, Variable rhs)
    {
        return Expression::from(lhs, rhs, 1);
    }
    constexpr Expression operator-(Variable lhs, Variable rhs)
    {
        return Expression::from(lhs, rhs, -1);
    }

// Expression (arithmetic)

    constexpr Expression operator*(const Expression& expr, result_type cf)
    {
        return Expression::from(expr, cf);
    }
    constexpr Expression operator/(const Expression& expr, result_type cf)
    {
        return Expression::from(expr, 1.f / cf);
    }
    constexpr Expression operator*(result_type cf, const Expression& expr)
    {
        return Expression::from(expr, cf);
    }
    constexpr Expression operator/(result_type cf, const Expression& expr)
    {
        return Expression::from(expr, 1.f / cf);
    }

    constexpr Expression operator+(const Expression& expr, Variable var)
    {
        return Expression::from(expr, var, 1);
    }
    constexpr Expression operator-(const Expression& expr, Variable var)
    {
        return Expression::from(expr, var, -1);
    }
    constexpr Expression operator+(Variable var, const Expression& expr)
    {
        return Expression::from(expr, var, 1);
    }
    constexpr Expression operator-(Variable var, const Expression& expr)
    {
        return Expression::from(expr, var, -1);
    }

// Expression (constraints)

    constexpr Expression operator==(const Expression& lhs, const Expression& rhs)
    {
        return Expression::reduce(lhs, rhs, Constraint::Equal);
    }
    constexpr Expression operator<(const Expression& lhs, const Expression& rhs)
    {
        return Expression::reduce(lhs, rhs, Constraint::Less);
    }
    constexpr Expression operator>(const Expression& lhs, const Expression& rhs)
    {
        return Expression::reduce(lhs, rhs, Constraint::More);
    }
    constexpr Expression operator<=(const Expression& lhs, const Expression& rhs)
    {
        return Expression::reduce(lhs, rhs, Constraint::LessEq);
    }
    constexpr Expression operator>=(const Expression& lhs, const Expression& rhs)
    {
        return Expression::reduce(lhs, rhs, Constraint::MoreEq);
    }

    constexpr Expression operator==(Variable lhs, Variable rhs)
    {
        return Expression::reduce(Expression::from(lhs), Expression::from(rhs), Constraint::Equal);
    }
    constexpr Expression operator<(Variable lhs, Variable rhs)
    {
        return Expression::reduce(Expression::from(lhs), Expression::from(rhs), Constraint::Less);
    }
    constexpr Expression operator>(Variable lhs, Variable rhs)
    {
        return Expression::reduce(Expression::from(lhs), Expression::from(rhs), Constraint::More);
    }
    constexpr Expression operator<=(Variable lhs, Variable rhs)
    {
        return Expression::reduce(Expression::from(lhs), Expression::from(rhs), Constraint::LessEq);
    }
    constexpr Expression operator>=(Variable lhs, Variable rhs)
    {
        return Expression::reduce(Expression::from(lhs), Expression::from(rhs), Constraint::MoreEq);
    }

    constexpr Expression operator==(const Expression& expr, result_type cf)
    {
        return Expression::reduce(expr, Expression::from(cf), Constraint::Equal);
    }
    constexpr Expression operator<(const Expression& expr, result_type cf)
    {
        return Expression::reduce(expr, Expression::from(cf), Constraint::Less);
    }
    constexpr Expression operator>(const Expression& expr, result_type cf)
    {
        return Expression::reduce(expr, Expression::from(cf), Constraint::More);
    }
    constexpr Expression operator<=(const Expression& expr, result_type cf)
    {
        return Expression::reduce(expr, Expression::from(cf), Constraint::LessEq);
    }
    constexpr Expression operator>=(const Expression& expr, result_type cf)
    {
        return Expression::reduce(expr, Expression::from(cf), Constraint::MoreEq);
    }

    constexpr Expression operator==(result_type cf, const Expression& expr)
    {
        return Expression::reduce(Expression::from(cf), expr, Constraint::Equal);
    }
    constexpr Expression operator<(result_type cf, const Expression& expr)
    {
        return Expression::reduce(Expression::from(cf), expr, Constraint::Less);
    }
    constexpr Expression operator>(result_type cf, const Expression& expr)
    {
        return Expression::reduce(Expression::from(cf), expr, Constraint::More);
    }
    constexpr Expression operator<=(result_type cf, const Expression& expr)
    {
        return Expression::reduce(Expression::from(cf), expr, Constraint::LessEq);
    }
    constexpr Expression operator>=(result_type cf, const Expression& expr)
    {
        return Expression::reduce(Expression::from(cf), expr, Constraint::MoreEq);
    }

    constexpr Expression operator==(const Expression& expr, Variable var)
    {
        return Expression::reduce(expr, Expression::from(var), Constraint::Equal);
    }
    constexpr Expression operator<(const Expression& expr, Variable var)
    {
        return Expression::reduce(expr, Expression::from(var), Constraint::Less);
    }
    constexpr Expression operator>(const Expression& expr, Variable var)
    {
        return Expression::reduce(expr, Expression::from(var), Constraint::More);
    }
    constexpr Expression operator<=(const Expression& expr, Variable var)
    {
        return Expression::reduce(expr, Expression::from(var), Constraint::LessEq);
    }
    constexpr Expression operator>=(const Expression& expr, Variable var)
    {
        return Expression::reduce(expr, Expression::from(var), Constraint::MoreEq);
    }

    constexpr Expression operator==(Variable var, const Expression& expr)
    {
        return Expression::reduce(Expression::from(var), expr, Constraint::Equal);
    }
    constexpr Expression operator<(Variable var, const Expression& expr)
    {
        return Expression::reduce(Expression::from(var), expr, Constraint::Less);
    }
    constexpr Expression operator>(Variable var, const Expression& expr)
    {
        return Expression::reduce(Expression::from(var), expr, Constraint::More);
    }
    constexpr Expression operator<=(Variable var, const Expression& expr)
    {
        return Expression::reduce(Expression::from(var), expr, Constraint::LessEq);
    }
    constexpr Expression operator>=(Variable var, const Expression& expr)
    {
        return Expression::reduce(Expression::from(var), expr, Constraint::MoreEq);
    }

    constexpr Expression operator==(Variable var, result_type cf)
    {
        return Expression::reduce(Expression::from(var), Expression::from(cf), Constraint::Equal);
    }
    constexpr Expression operator<(Variable var, result_type cf)
    {
        return Expression::reduce(Expression::from(var), Expression::from(cf), Constraint::Less);
    }
    constexpr Expression operator>(Variable var, result_type cf)
    {
        return Expression::reduce(Expression::from(var), Expression::from(cf), Constraint::More);
    }
    constexpr Expression operator<=(Variable var, result_type cf)
    {
        return Expression::reduce(Expression::from(var), Expression::from(cf), Constraint::LessEq);
    }
    constexpr Expression operator>=(Variable var, result_type cf)
    {
        return Expression::reduce(Expression::from(var), Expression::from(cf), Constraint::MoreEq);
    }

    constexpr Expression operator==(result_type cf, Variable var)
    {
        return Expression::reduce(Expression::from(cf), Expression::from(var), Constraint::Equal);
    }
    constexpr Expression operator<(result_type cf, Variable var)
    {
        return Expression::reduce(Expression::from(cf), Expression::from(var), Constraint::Less);
    }
    constexpr Expression operator>(result_type cf, Variable var)
    {
        return Expression::reduce(Expression::from(cf), Expression::from(var), Constraint::More);
    }
    constexpr Expression operator<=(result_type cf, Variable var)
    {
        return Expression::reduce(Expression::from(cf), Expression::from(var), Constraint::LessEq);
    }
    constexpr Expression operator>=(result_type cf, Variable var)
    {
        return Expression::reduce(Expression::from(cf), Expression::from(var), Constraint::MoreEq);
    }
}
