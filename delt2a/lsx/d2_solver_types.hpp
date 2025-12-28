#ifndef D2_SOLVER_TYPES_HPP
#define D2_SOLVER_TYPES_HPP

#include <array>
#include <cstdint>
#include <span>
#include <string>

namespace d2::lsx
{
    using result_type = float;

    enum class Constraint
    {
        Equal,
        Less,
        More,
        LessEq,
        MoreEq
    };

    class VariableIndex
    {
    public:
        enum class Type : unsigned char
        {
            // Special types

            Constant,
            Local,
            Global,
            Objective,

            // External types

            Variable,

            // Internal types

            Slack,
            Error,
            Dummy,
            Artificial,
        };
    private:
        static constexpr auto _idx_bits   = 27u;
        static constexpr auto _type_bits  = 4u;
        static constexpr auto _type_shift = _idx_bits;
        static constexpr auto _derived_shift  = _idx_bits + _type_bits;

        static constexpr auto _idx_mask   = (1u << _idx_bits) - 1u;
        static constexpr auto _type_mask  = (1u << _type_bits) - 1u;
        static constexpr auto _derived_mask = (1u << _derived_shift);

        std::uint32_t _code{ 0x00 };

        static constexpr std::uint32_t _pack(std::uint32_t idx, Type type, bool derived)
        {
            return
                (idx & _idx_mask) |
                ((static_cast<uint32_t>(type) & _type_mask) << _type_shift) |
                (derived ? _derived_mask : 0u);
        }
    public:
        constexpr VariableIndex()
            : _code(_pack(0, Type::Constant, false)) {}
        constexpr VariableIndex(const VariableIndex&) = default;
        constexpr VariableIndex(VariableIndex&&) = default;
        constexpr VariableIndex(Type type, bool derived = false)
            : _code(_pack(0, type, derived)) {}
        constexpr VariableIndex(std::uint32_t idx, Type type, bool derived = false)
            : _code(_pack(idx, type, derived)) {}

        constexpr bool is_derived() const
        {
            return _code & _derived_mask;
        }
        constexpr bool is_internal() const
        {
            const auto tp = type();
            return
                tp == Type::Slack ||
                tp == Type::Error ||
                tp == Type::Artificial ||
                tp == Type::Dummy;
        }
        constexpr bool is_pivotable() const
        {
            const auto tp = type();
            return
                tp == Type::Slack ||
                tp == Type::Error;
        }
        constexpr bool is_external() const
        {
            return type() == Type::Variable;
        }

        constexpr Type type() const
        {
            return static_cast<Type>((_code >> _type_shift) & _type_mask);
        }
        constexpr std::uint32_t index() const
        {
            return _code & _idx_mask;
        }
        constexpr std::uint32_t code() const
        {
            return _code;
        }

        std::string print() const;

        VariableIndex& operator=(const VariableIndex&) = default;
        VariableIndex& operator=(VariableIndex&&) = default;

        bool operator==(const VariableIndex& copy) const
        {
            return copy._code == _code;
        }
    };
    class Variable : public VariableIndex
    {
    private:
        result_type _coefficient{ 1.f };
    public:
        constexpr Variable()
            : VariableIndex(VariableIndex::Type::Constant) {}
        constexpr Variable(VariableIndex idx, result_type coefficient = 1.f)
            : VariableIndex(idx), _coefficient(coefficient) {}
        constexpr Variable(result_type coefficient)
            : VariableIndex(VariableIndex::Type::Constant), _coefficient(coefficient) {}

        constexpr result_type coefficient() const
        {
            return _coefficient;
        }

        constexpr Variable operator[](std::size_t idx) const
        {
            return Variable(VariableIndex(index() + idx, type(), is_derived()));
        }
    };
    class Symbol : public VariableIndex
    {
    private:
        mutable result_type _coefficient{ 1.f };
    public:
        constexpr Symbol() = default;
        constexpr Symbol(const Variable& var)
            : _coefficient(var.coefficient()), VariableIndex(var) {}
        constexpr Symbol(VariableIndex idx, result_type coefficient = 1.f)
            : VariableIndex(idx), _coefficient(coefficient) {}
        constexpr Symbol(result_type coefficient)
            : _coefficient(coefficient) {}

        constexpr result_type coefficient() const
        {
            return _coefficient;
        }
        constexpr Symbol invert() const
        {
            return Symbol(*this, -_coefficient);
        }

        std::string print() const;

        constexpr const Symbol& operator*=(result_type cf) const
        {
            _coefficient *= cf;
            return *this;
        }
        constexpr const Symbol& operator/=(result_type cf) const
        {
            _coefficient /= cf;
            return *this;
        }
        constexpr const Symbol& operator+=(result_type cf) const
        {
            _coefficient += cf;
            return *this;
        }
        constexpr const Symbol& operator-=(result_type cf) const
        {
            _coefficient -= cf;
            return *this;
        }
        constexpr const Symbol& operator=(result_type cf) const
        {
            _coefficient = cf;
            return *this;
        }
    };

    class Expression
    {
    private:
        std::size_t _size_vars{ 0 };
        std::size_t _size_locals{ 0 };
        std::size_t _size_globals{ 0 };
        std::array<Variable, 24> _terms{};
        Constraint _op{ Constraint::Equal };
        result_type _constant{ 0 };
        result_type _scale{ 0 };

        static constexpr void _accumulate_expr(
            std::span<Variable> sink_v,
            std::span<Variable> sink_l,
            std::span<Variable> sink_g,
            const Expression& source,
            Expression& dest,
            int sign
        )
        {
            std::size_t vcnt = 0;
            std::size_t lcnt = 0;
            std::size_t gcnt = 0;

            std::span<Variable> vs = sink_v.subspan(dest._size_vars);
            std::span<Variable> ls = sink_l.subspan(dest._size_locals);
            std::span<Variable> gs = sink_g.subspan(dest._size_globals);

            for (std::size_t i = 0; i < source._size_vars; i++)
            {
                const auto& v = source._terms[i];
                const auto type = v.type();
                const auto c = v.coefficient() * sign;
                const auto var = Variable(v, c);
                dest._scale += std::abs(c);
                switch (type)
                {
                    case VariableIndex::Type::Constant: dest._constant += c; break;
                    case VariableIndex::Type::Variable: vs[vcnt++] = var; dest._size_vars++; break;
                    case VariableIndex::Type::Local: ls[lcnt++] = var; dest._size_locals++; break;
                    case VariableIndex::Type::Global: gs[gcnt++] = var; dest._size_globals++; break;
                    default: std::abort();
                }
            }
        }
    public:
        // Returns an expression in form
        // _terms[0] + ... + _terms[n] + _constant <_op> 0
        // Performs normalization if the operator is <more/eq>
        static constexpr Expression reduce(const Expression& lhs, const Expression& rhs, Constraint op)
        {
            Expression result;
            result._op = op;

            std::array<Variable, 24> vars;
            std::array<Variable, 24> locals;
            std::array<Variable, 24> globals;

            // Accumulate types into their respective storage
            // (coefficients are added directly to result._rhs)

            _accumulate_expr(vars, locals, globals, lhs, result,  1);
            _accumulate_expr(vars, locals, globals, rhs, result, -1);

            // Simplify and write back

            std::copy(
                vars.begin(),
                vars.begin() + result._size_vars,
                result._terms.begin()
            );
            std::copy(
                locals.begin(),
                locals.begin() + result._size_locals,
                result._terms.begin() + result._size_vars
            );
            std::copy(
                globals.begin(),
                globals.begin() + result._size_globals,
                result._terms.begin() + result._size_vars + result._size_locals
            );

            // Normalize

            // We add an epsilon (and transform into a less/more/eq inequality)
            if (op == Constraint::More || op == Constraint::Less)
            {
                constexpr auto epsilon_rel = result_type(1e-8);
                constexpr auto epsilon_abs = result_type(1e-7);
                const auto epsilon = std::max(epsilon_abs, epsilon_rel * std::max(result_type(1), result._scale));
                result._constant -= epsilon;
                if (op == Constraint::More) op = Constraint::MoreEq;
                else if (op == Constraint::Less) op = Constraint::LessEq;
            }

            // Transform into a less/eq inequality
            const auto is_more = op == Constraint::MoreEq;
            if (is_more)
            {
                for (std::size_t i = 0; i < result._size_globals + result._size_locals + result._size_vars; i++)
                {
                    auto& term = result._terms[i];
                    term = Variable(term, term.coefficient() * -1.f);
                }
                result._constant *= -1.f;
                result._op = Constraint::LessEq;
            }

            return result;
        }
        static constexpr Expression from(Variable lhs, Variable rhs, int sign)
        {
            Expression result;
            result._terms[0] = lhs;
            result._terms[1] = Variable(rhs, rhs.coefficient() * sign);
            result._size_vars = 2;
            return result;
        }
        static constexpr Expression from(const Expression& lhs, Variable rhs, int sign)
        {
            Expression result;
            std::copy(lhs._terms.begin(), lhs._terms.begin() + lhs._size_vars, result._terms.begin());
            result._terms[lhs._size_vars] =
                Variable(rhs, rhs.coefficient() * sign);
            result._size_vars = lhs._size_vars + 1;
            return result;
        }
        static constexpr Expression from(Variable lhs, const Expression& rhs, int sign)
        {
            return from(rhs, lhs, -sign);
        }
        static constexpr Expression from(const Expression& lhs, result_type cf)
        {
            Expression result;
            while (result._size_vars != lhs._size_vars)
            {
                const auto idx = result._size_vars;
                result._terms[idx] =
                    Variable(lhs._terms[idx], lhs._terms[idx].coefficient() * cf);
                result._size_vars++;
            }
            return result;
        }
        static constexpr Expression from(result_type cf, const Expression& rhs, int sign)
        {
            return from(rhs, cf, -sign);
        }
        static constexpr Expression from(Variable var)
        {
            Expression result;
            result._terms[0] = var;
            result._size_vars++;
            return result;
        }

        constexpr Constraint type() const
        {
            return _op;
        }

        constexpr std::span<const Variable> vars() const
        {
            return std::span(_terms.begin(), _terms.begin() + _size_vars);
        }
        constexpr std::span<const Variable> locals() const
        {
            return std::span(_terms.begin() + _size_vars, _terms.begin() + _size_vars + _size_locals);
        }
        constexpr std::span<const Variable> globals() const
        {
            return std::span(_terms.begin() + _size_vars + _size_locals, _terms.begin() + _size_vars + _size_globals);
        }

        constexpr result_type constant() const
        {
            return _constant;
        }

        std::string print() const;
    };

    struct ParentVarWrapper
    {
        constexpr Variable operator[](Variable var) const
        {
            return Variable(
                       VariableIndex(var.index(), var.type(), true)
                   );
        }
    };

    constexpr Variable var = Variable(VariableIndex(0, VariableIndex::Type::Variable));
    constexpr Variable local = Variable(VariableIndex(0, VariableIndex::Type::Local));
    constexpr Variable global = Variable(VariableIndex(0, VariableIndex::Type::Global));
    constexpr ParentVarWrapper parent = ParentVarWrapper();
}

// std::hash overloads
namespace std
{
    template<> struct hash<d2::lsx::VariableIndex>
    {
        std::size_t operator()(const d2::lsx::VariableIndex& idx) const
        {
            return std::hash<std::uint32_t>()(idx.code());
        }
    };
    template<> struct hash<d2::lsx::Symbol>
    {
        std::size_t operator()(const d2::lsx::Symbol& idx) const
        {
            return std::hash<std::uint32_t>()(idx.code());
        }
    };
}
// ABSL hash overloads
namespace d2::lsx
{
    template<typename Hash>
    Hash AbslHashValue(Hash hash, const lsx::Symbol& val)
    {
        return Hash::combine(std::move(hash), val.code());
    }
    template<typename Hash>
    Hash AbslHashValue(Hash hash, const lsx::VariableIndex& val)
    {
        return Hash::combine(std::move(hash), val.code());
    }
}


#endif // D2_SOLVER_TYPES_HPP
