#include "d2_solver.hpp"
#include <sstream>

namespace d2::lsx
{
    namespace impl
    {
        void Row::substitute(Symbol lhs, const Row& rhs)
        {
            const auto sub = _terms.find(lhs);
            if (sub != _terms.end())
            {
                // 1. Substitution coefficient
                // 2. Coefficient of the substituted values
                // 3. Coefficient of the substituted constant
                const auto mcf = sub->coefficient() / lhs.coefficient();
                const auto scf = mcf * rhs._lazy_terms_coefficient;
                const auto ccf = mcf * _lazy_terms_coefficient;
                _terms.reserve(_terms.size() + rhs._terms.size());
                for (decltype(auto) term : rhs._terms)
                {
                    // If the variable is present we just add the coefficients together
                    // Else we insert a new term
                    // We have to remember to apply the lazy coefficient to any terms that come from rhs

                    const auto var = Symbol(term, term.coefficient() * scf);
                    const auto [ it, created ] = _terms.emplace(var);

                    // The term is already present
                    if (!created)
                    {
                        // The coefficients cancel out
                        const auto ncf = var.coefficient() + it->coefficient();
                        if (ncf == 0.f)
                        {
                            _terms.erase(it);
                        }
                        else
                        {
                            *it = ncf;
                        }
                    }
                }
                _constant += rhs._constant * ccf;
                _terms.erase(lhs);
            }
        }
        void Row::normalize()
        {
            if (_constant < 0.f)
            {
                _constant *= -1.f;
                _lazy_terms_coefficient *= -1.f;
            }
        }

        std::optional<Symbol> Row::find(Symbol symbol)
        {
            auto f = _terms.find(symbol);
            if (f == _terms.end())
                return std::nullopt;
            return Symbol(symbol, symbol.coefficient() * _lazy_terms_coefficient);
        }

        void Row::insert(Symbol term)
        {
            _terms.emplace(Symbol(term, term.coefficient() / _lazy_terms_coefficient));
        }
        void Row::erase(Symbol term)
        {
            _terms.erase(term);
        }

        void Row::reserve(std::size_t count)
        {
            _terms.reserve(count);
        }

        bool Row::contains(Symbol term)
        {
            return _terms.contains(term);
        }

        const result_type& Row::constant() const
        {
            return _constant;
        }
        result_type& Row::constant()
        {
            return _constant;
        }

        std::size_t Row::size() const
        {
            return _terms.size();
        }

        std::string Row::print() const
        {
            std::stringstream out;
            out << _constant;
            foreach ([&](Symbol term)
        {
            out
                    << (term.coefficient() < 0.f ? " - " : " + ")
                        << Symbol(term, std::abs(term.coefficient())).print();
                return true;
            });
            return out.str();
        }

        Row& Row::operator*=(result_type cf)
        {
            _lazy_terms_coefficient *= cf;
            _constant *= cf;
            return *this;
        }
        Row& Row::operator/=(result_type cf)
        {
            _lazy_terms_coefficient /= cf;
            _constant /= cf;
            return *this;
        }
    }

    std::optional<Symbol> ConstraintArray::_find_subject(const impl::Row& row)
    {
        auto target_tertiary = Symbol();
        auto target_secondary = Symbol();
        auto target_primary = Symbol();
        row.foreach([&](Symbol term)
        {
            if (term.is_external())
            {
                target_primary = term;
                return false;
            }
            else if (term.is_internal())
            {
                if (term.coefficient() < 0.f)
                    target_secondary = term;
                else
                    target_tertiary = term;
            }
            return true;
        });
        if (target_primary.type() != Symbol::Type::Constant) return target_primary.invert();
        if (target_secondary.type() != Symbol::Type::Constant) return target_secondary.invert();
        if (target_tertiary.type() != Symbol::Type::Constant) return target_tertiary.invert();
        return std::nullopt;
    }
    std::optional<Symbol> ConstraintArray::_find_entering()
    {
        const auto f = _tableau.find(Symbol(Symbol::Type::Objective));
        if (f != _tableau.end())
        {
            auto target = Symbol(Symbol::Type::Constant, std::numeric_limits<result_type>::infinity());
            f->second.foreach([&](Symbol term)
            {
                if (term.is_pivotable())
                {
                    if (term.coefficient() < -_epsilon &&
                            term.coefficient() < target.coefficient())
                        target = term;
                }
                return true;
            });
            if (target.is_pivotable())
                return target;
        }
        return std::nullopt;
    }
    std::optional<Symbol> ConstraintArray::_find_dual_entering(tableau_iterator row)
    {
        result_type target_ratio = std::numeric_limits<result_type>::infinity();
        Symbol target{};

        const auto obj = _tableau.find(VariableIndex::Type::Objective);
        row->second.foreach([&](Symbol term)
        {
            if (term.coefficient() > _epsilon)
            {
                const auto f = obj->second.find(term);
                const auto ocf = f.has_value() ? f->coefficient() : 0.f;
                const auto ratio = ocf / term.coefficient();
                // We use the index as a tie-breaker if the ratio is almost equal to the target ratio
                if ((ratio >= 0.f && ratio < target_ratio) ||
                        ((ratio - _epsilon) < target_ratio && term.index() < target.index()))
                {
                    target_ratio = ratio;
                    target = term;
                }
            }
            return true;
        });

        if (target.type() == Symbol::Type::Constant)
            return std::nullopt;
        return target;
    }
    std::optional<ConstraintArray::tableau_iterator> ConstraintArray::_find_leaving(Symbol entering)
    {
        result_type row_ratio = std::numeric_limits<result_type>::infinity();
        tableau_iterator row = _tableau.end();

        // Find best leaving row
        for (auto it = _tableau.begin(); it != _tableau.end(); ++it)
        {
            if (it->first.type() != Symbol::Type::Objective &&
                    it->first.type() != Symbol::Type::Variable)
            {
                // Must contain the entering symbol
                // The coefficient in the leaving row must be negative
                if (const auto term = it->second.find(entering);
                        term.has_value() && term->coefficient() < -_epsilon)
                {
                    // The ratio of the _constant to the coefficient must be the smallest possible
                    const auto ratio = it->second.constant() / term->coefficient();
                    if ((ratio >= 0.f && ratio < row_ratio))
                    {
                        row_ratio = ratio;
                        row = it;
                    }
                }
            }
        }

        if (row == _tableau.end())
            return std::nullopt;
        return row;
    }
    std::optional<VariableIndex> ConstraintArray::_pop_infeasible()
    {
        if (_infeasible.empty())
            return std::nullopt;
        const auto id = *_infeasible.begin();
        _infeasible.erase(_infeasible.begin());
        return id;
    }

    void ConstraintArray::_pivot(Symbol entering, tableau_iterator leaving)
    {
        auto row = std::move(leaving->second);
        row /= -entering.coefficient();
        row.insert(Symbol(leaving->first, -1.f));
        _tableau.erase(leaving);
        const auto sym = Symbol(entering, 1.f);
        for (decltype(auto) it : _tableau)
        {
            it.second.substitute(sym, row);
            _mark(it.first, it.first.is_pivotable() && it.second.constant() <= -_epsilon);
        }
        _mark(entering, row.constant() <= -_epsilon);
        _tableau.emplace(entering, std::move(row));
    }
    void ConstraintArray::_mark(VariableIndex symbol, bool condition)
    {
        if (condition) _infeasible.emplace(symbol);
        else _infeasible.erase(symbol);
    }

    bool ConstraintArray::_dual_optimize()
    {
        std::optional<VariableIndex> leaving;
        while ((leaving = _pop_infeasible()).has_value())
        {
            const auto row = _tableau.find(leaving.value());
            const auto entering = _find_dual_entering(row);
            if (!entering.has_value())
                return false;
            _pivot(entering.value(), row);
        }
        return true;
    }
    bool ConstraintArray::_optimize()
    {
        std::optional<Symbol> entering;
        while ((entering = _find_entering()).has_value())
        {
            const auto leaving = _find_leaving(entering.value());
            if (!leaving.has_value())
                return false;
            _pivot(entering.value(), leaving.value());
        }
        return true;
    }

    void ConstraintArray::_set_error(Variable error, result_type strength)
    {
        const auto [ it, _ ] = _tableau.emplace(VariableIndex(Symbol::Type::Objective), impl::Row());
        it->second.insert(Variable(error, strength));
    }
    void ConstraintArray::_set_error(Variable e1, Variable e2, result_type strength)
    {
        const auto [ it, _ ] = _tableau.emplace(VariableIndex(Symbol::Type::Objective), impl::Row());
        it->second.reserve(it->second.size() + 2);
        it->second.insert(Symbol(e1, strength));
        it->second.insert(Symbol(e2, strength));
    }

    std::string ConstraintArray::status_to_string(Status status)
    {
        switch (status)
        {
            case Status::Unsolvable: return "No solution";
            case Status::NotFound: return "Not found";
            case Status::Ok: return "Ok";
        }
    }

    ConstraintArray::Status ConstraintArray::status(result_type value) const
    {
        if (value == _unsolvable_const) return Status::Unsolvable;
        if (value == _not_found_const) return Status::NotFound;
        return Status::Ok;
    }

    bool ConstraintArray::remove_constraint(handle handle)
    {
        return false;
    }
    ConstraintArray::handle ConstraintArray::set_constraint(const Expression& expr, Strength strength)
    {
        return set_constraint(expr, result_type(strength));
    }
    ConstraintArray::handle ConstraintArray::set_constraint(const Expression& expr, result_type strength)
    {
        const auto is_less = expr.type() == Constraint::LessEq;
        const auto is_error = strength < result_type(Strength::Required);
        const auto vars = expr.vars();
        VariableIndex low{};
        VariableIndex high{};

        impl::Row row{};
        row.reserve(
            vars.size() +
            (is_less && is_error) * 2 +
            (is_error && !is_less) * 2 +
            (!is_error && !is_less) * 1
        );
        row.insert_range(vars.begin(), vars.end());
        row.constant() = expr.constant();

        // We add a slack
        if (is_less)
        {
            low = VariableIndex(_internal_id_ctr++, Symbol::Type::Slack);
            row.insert(low, 1.f);
            // We add a single error
            if (is_error)
            {
                high = VariableIndex(_internal_id_ctr++, Symbol::Type::Error);
                row.insert(high, 1.f);
                _set_error(high, strength);
            }
        }
        // We add two errors (+ and -)
        else if (is_error)
        {
            low = VariableIndex(_internal_id_ctr++, Symbol::Type::Error);
            high = VariableIndex(_internal_id_ctr++, Symbol::Type::Error);
            row.insert(low,  1.f);
            row.insert(high, -1.f);
            _set_error(low, high, strength);
        }
        // We add a dummy
        else
        {
            low = VariableIndex(_internal_id_ctr++, Symbol::Type::Dummy);
            row.insert(low,  1.f);
        }

        // Substitute all basics into current row
        {
            std::vector<tableau_iterator> sub;
            sub.reserve(row.size());
            row.foreach([&](Symbol symbol)
            {
                const auto f = _tableau.find(symbol);
                if (f != _tableau.end())
                    sub.push_back(f);
                return true;
            });
            for (decltype(auto) it : sub)
                row.substitute(it->first, it->second);
        }
        row.normalize();

        // Select the subject and substitute it to all other rows
        const auto subject = _find_subject(row);
        if (subject.has_value())
        {
            row.erase(subject.value());
            row /= subject->coefficient();

            // Search tableau and substitute current subject
            // Since we already divided the row by the symbols coefficient the substitute should have a coefficient of '1'
            const auto sub = Symbol(subject.value());
            for (decltype(auto) it : _tableau)
            {
                it.second.substitute(sub, row);
                _mark(it.first, it.first.is_pivotable() && it.second.constant() <= -_epsilon);
            }

            _mark(sub, sub.is_pivotable() && row.constant() <= -_epsilon);
            _tableau.emplace(sub, std::move(row));
        }
        // Create artificial and restore feasability
        else
        {

        }

        if (!_optimize() || (!_infeasible.empty() && !_dual_optimize()))
            return nullptr;
        return handle{ low, high };
    }
    result_type ConstraintArray::get(Variable var) const
    {
        const auto f = _tableau.find(var);
        if (f == _tableau.end())
            return _not_found_const;
        return f->second.constant();
    }

    void ConstraintArray::start_batch()
    {
        _flags |= IsBatched;
    }
    void ConstraintArray::end_batch()
    {
        _flags |= ~IsBatched;
    }

    void ConstraintArray::clear()
    {
        _tableau.clear();
        _infeasible.clear();
        _internal_id_ctr = 0;
    }

    std::string ConstraintArray::print() const
    {
        std::stringstream out;
        out << "Tableau:\n";
        for (decltype(auto) it : _tableau)
        {
            out
                    << it.first.print()
                    << " = "
                    << it.second.print()
                    << '\n';
        }
        if (!_infeasible.empty())
        {
            out << "Infeasible:\n";
            for (decltype(auto) it : _infeasible)
            {
                out
                        << it.print()
                        << " = "
                        << _tableau.at(it).print()
                        << '\n';
            }
        }
        return out.str();
    }
}
