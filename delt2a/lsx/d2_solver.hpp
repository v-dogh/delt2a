#ifndef D2_SOLVER_HPP
#define D2_SOLVER_HPP

#include "d2_solver_types.hpp"
#include "d2_solver_ops.hpp"
#include <limits>
#include <optional>
#include <absl/container/flat_hash_set.h>
#include <absl/container/flat_hash_map.h>

namespace d2::lsx
{
    template<typename Key, typename Value>
    using hash_map = absl::flat_hash_map<Key, Value>;

    template<typename Value>
    using hash_set = absl::flat_hash_set<Value>;

    // template<typename Key, typename Value>
    // using hash_map = std::unordered_map<Key, Value>;

    // template<typename Value>
    // using hash_set = std::unordered_set<Value>;

    enum class Strength : unsigned int
    {
        Required = 100'000'000,
        Strong = 100'000,
        Medium = 100,
        Weak = 1,
    };

    namespace impl
    {
        class Row
        {
        private:
            hash_set<Symbol> _terms{};
            result_type _lazy_terms_coefficient{ 1.f };
            result_type _constant{ 0.f };
        public:
            Row() = default;
            Row(const Row&) = default;
            Row(Row&&) = default;

            // Substitutes lhs into terms of this row with terms from rhs
            void substitute(Symbol lhs, const Row& rhs) noexcept;
            // Makes the constant positive
            void normalize() noexcept;

            // Returns the value if it's present in the terms
            std::optional<Symbol> find(Symbol symbol) noexcept;

            // Inserts a new term
            void insert(Symbol term) noexcept;
            template<typename... Argv>
            void insert(Argv&&... args) noexcept
            {
                insert(Symbol(std::forward<Argv>(args)...));
            }
            void insert_range(auto&& begin, auto&& end) noexcept
            {
                do insert(*begin);
                while (++begin != end);
            }
            void erase(Symbol term) noexcept;

            void reserve(std::size_t count) noexcept;

            // Returns true if the row contains the term
            bool contains(Symbol term) noexcept;

            std::size_t size() const noexcept;

            const result_type& constant() const noexcept;
            result_type& constant() noexcept;

            inline void foreach (auto&& func) const noexcept
            {
                for (decltype(auto) it : _terms)
                    if (!func(Symbol(it, it.coefficient() * _lazy_terms_coefficient)))
                        break;
            }
            inline void foreach (auto&& func) noexcept
            {
                for (decltype(auto) it : _terms)
                    if (!func(Symbol(it, it.coefficient() * _lazy_terms_coefficient)))
                        break;
            }

            std::string print() const noexcept;

            Row& operator=(const Row&) = default;
            Row& operator=(Row&&) = default;

            Row& operator*=(result_type cf) noexcept;
            Row& operator/=(result_type cf) noexcept;
        };
        struct ConstraintHandle
        {
        private:
            VariableIndex _low{};
            VariableIndex _high{};
        public:
            ConstraintHandle() = default;
            ConstraintHandle(std::nullptr_t) {}
            ConstraintHandle(ConstraintHandle&&) = default;
            ConstraintHandle(const ConstraintHandle&) = default;
            ConstraintHandle(VariableIndex low, VariableIndex high = VariableIndex::Type::Constant) noexcept
                : _low(low), _high(high) {}

            bool operator==(std::nullptr_t)
            { return _low.type() == VariableIndex::Type::Constant;  }

            ConstraintHandle& operator=(const ConstraintHandle&) = default;
            ConstraintHandle& operator=(ConstraintHandle&&) = default;
        };
    }

    class ConstraintArray
    {
    public:
        using handle = impl::ConstraintHandle;
        enum class Status
        {
            NotFound,
            Unsolvable,
            Ok
        };
    private:
        enum Flags : unsigned char
        {
            IsBatched = 1 << 0,
        };
        using tableau = hash_map<VariableIndex, impl::Row>;
        using tableau_iterator = tableau::iterator;
    private:
        static constexpr auto _unsolvable_const = std::numeric_limits<result_type>::infinity();
        static constexpr auto _not_found_const = std::numeric_limits<result_type>::quiet_NaN();
        static constexpr auto _epsilon = 1e-8;

        hash_map<VariableIndex, impl::Row> _tableau{};
        hash_set<VariableIndex> _infeasible{};
        std::uint32_t _internal_id_ctr{ 0 };
        unsigned char _flags{ 0x00 };

        std::optional<Symbol> _find_subject(const impl::Row& row) noexcept;
        std::optional<Symbol> _find_entering() noexcept;
        std::optional<Symbol> _find_dual_entering(tableau_iterator row) noexcept;
        std::optional<tableau_iterator> _find_leaving(Symbol entering) noexcept;
        std::optional<VariableIndex> _pop_infeasible() noexcept;

        void _pivot(Symbol entering, tableau_iterator leaving) noexcept;
        void _mark(VariableIndex symbol, bool condition) noexcept;

        bool _optimize() noexcept;
        bool _dual_optimize() noexcept;

        void _set_error(Variable error, result_type strength) noexcept;
        void _set_error(Variable e1, Variable e2, result_type strength) noexcept;
    public:
        static std::string status_to_string(Status status) noexcept;

        ConstraintArray() = default;
        ConstraintArray(const ConstraintArray&) = default;
        ConstraintArray(ConstraintArray&&) = default;

        // Converts a value into a Status enum
        Status       status(result_type value) const noexcept;

        // Returns true if the constraint was removed
        bool   remove_constraint(handle handle) noexcept;
        handle set_constraint(const Expression& expr, Strength strength = Strength::Required) noexcept;
        handle set_constraint(const Expression& expr, float strength) noexcept;

        result_type                get(Variable var) const noexcept;
        result_type                get_or(Variable var, auto&& func, result_type fallback) const noexcept
        {
            const auto value = get(var);
            const auto err   = status(value);
            switch (err)
            {
                case Status::Ok: return value;
                default: func(err); break;
            }
            return fallback;
        }
        std::optional<result_type> get_or(Variable var, auto&& func) const noexcept
        {
            const auto value = get(var);
            const auto err   = status(value);
            switch (err)
            {
                case Status::Ok: return value;
                default: func(err); break;
            }
            return std::nullopt;
        }

        void start_batch() noexcept;
        void end_batch() noexcept;

        // Delays constraint evaluation until the end of the batch
        // This prevents repeated solving when adding multiple constraints
        // The current constraint array reference is passed as the first argument
        void batch(auto&& func)
        {
            start_batch();
            func(*this);
            end_batch();
        }
        void clear() noexcept;

        std::string print() const noexcept;

        ConstraintArray& operator=(const ConstraintArray&) = default;
        ConstraintArray& operator=(ConstraintArray&&) = default;
    };
}

#endif
