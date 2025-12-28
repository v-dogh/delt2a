#include "d2_solver_types.hpp"
#include <sstream>
#include <format>

namespace d2::lsx
{
    std::string VariableIndex::print() const
    {
        char mark = 'I';
        switch (type())
        {
            case Type::Constant: mark = 'C'; break;
            case Type::Local: mark = 'L'; break;
            case Type::Global: mark = 'G'; break;
            case Type::Objective: mark = 'O'; break;
            case Type::Variable: mark = 'V'; break;
            case Type::Slack: mark = 'S'; break;
            case Type::Error: mark = 'E'; break;
            case Type::Dummy: mark = 'D'; break;
            case Type::Artificial: mark = 'A'; break;
        }
        return is_derived() ? std::format("{}_{}^", mark, index()) : std::format("{}_{}", mark, index());
    }
    std::string Symbol::print() const
    {
        return std::format("{}{}", _coefficient, VariableIndex::print());
    }

    std::string Expression::print() const
    {
        std::ostringstream out;
        std::size_t off = 0;
        for (std::size_t i = 0; i < _size_vars; i++, off++)
        {
            if (off || _terms[off].coefficient() < 0.f) { out << (_terms[off].coefficient() < 0.f ? " - " : " + "); }
            out
                    << std::abs(_terms[off].coefficient())
                    << "V["
                    << _terms[off].index()
                    << "]";
        }
        for (std::size_t i = 0; i < _size_locals; i++, off++)
        {
            if (off) { out << (_terms[off].coefficient() < 0.f ? " - " : " + "); }
            out
                    << std::abs(_terms[off].coefficient())
                    << "L["
                    << _terms[off].index()
                    << "]";
        }
        for (std::size_t i = 0; i < _size_globals; i++, off++)
        {
            if (off) { out << (_terms[off].coefficient() < 0.f ? " - " : " + "); }
            out
                    << std::abs(_terms[off].coefficient())
                    << "G["
                    << _terms[off].index()
                    << "] + ";
        }

        out << " == ";
        out << -_constant;

        return out.str();
    }
}
