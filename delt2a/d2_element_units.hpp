#ifndef D2_ELEMENT_UNITS_HPP
#define D2_ELEMENT_UNITS_HPP

namespace d2
{
// More boolerplate xd
    class Unit
    {
    public:
        enum Type : unsigned char
        {
            // Pixels (characters)
            Px,
            // Percentage of viewport
            Pv,
            // Percentage of parent
            Pc,

            // Automatic units

            // Allow the object to align itself
            Auto,
        };
        enum Mods : unsigned char
        {
            // Makes the units relative
            // Dependent on how the container handles them
            Relative = 1 << 0,
            // Inverts the units
            // E.g. by default x offsets are from left
            // Inverted, they will be from right
            Inverted = 1 << 1,
            // Center the object (modified by Horizontal/Vertical mods)
            Center = 1 << 2,

            // Multiplies the unit by cell the ratio of cell height to width
            Adjusted = 1 << 3,

            // Context flags
            // These are aliased by UnitContext

            Horizontal = 1 << 4,
            Vertical = 1 << 5,

            // (internal use only)

            _Position = 1 << 6,
            _Dimensions = 1 << 7,
        };
    private:
        struct UnitType
        {
            unsigned char mods{ 0x00 };
            Type type{ Px };

            constexpr UnitType() = default;
            constexpr UnitType(Type type, unsigned char mods = 0x00) : mods(mods), type(type) { }
        };
    protected:
        float value_{ 0.f };
        UnitType units_{};
    public:
        constexpr Unit() = default;
        constexpr Unit(float value, Type units = Px, unsigned char mods = 0x00)
            : value_(value), units_(units, mods) {}
        constexpr Unit(Type units, unsigned char mods = 0x00) : units_(units, mods) {}
        constexpr Unit(Mods mods) : units_(Type::Px, mods) {}
        constexpr Unit(const Unit&) = default;
        constexpr Unit(Unit&&) = default;

        constexpr bool relative() const
        {
            // Auto does not count as relative
            return units_.mods & Mods::Relative;
        }
        constexpr bool contextual() const
        {
            // Auto does not count as relative
            return
                units_.type == Type::Pc ||
                (units_.mods & (Mods::Inverted | Mods::Center));
        }
        constexpr bool inverted() const
        {
            return units_.mods & Mods::Inverted;
        }

        constexpr const float& raw() const
        {
            return value_;
        }

        constexpr void setunits(Type units)
        {
            units_.type = units;
        }
        constexpr Type getunits() const
        {
            return units_.type;
        }

        constexpr void setmods(unsigned char mods, bool value = true)
        {
            units_.mods &= ~mods;
            units_.mods |= (value * mods);
        }
        constexpr unsigned char getmods() const
        {
            return units_.mods;
        }

        constexpr Unit& operator=(const Unit&) = default;
        constexpr Unit& operator=(Unit&&) = default;
        constexpr Unit& operator=(Mods mods)
        {
            return operator=(Unit(mods));
        }
        constexpr Unit& operator=(Type units)
        {
            return operator=(Unit(units));
        }

        constexpr Unit operator-() const
        {
            return Unit(-value_, units_.type, units_.mods);
        }

        constexpr Unit& operator++()
        {
            ++value_;
            return *this;
        }
        constexpr Unit& operator--()
        {
            --value_;
            return *this;
        }
        constexpr Unit operator++(int)
        {
            Unit copy(*this);
            ++value_;
            return copy;
        }
        constexpr Unit operator--(int)
        {
            Unit copy(*this);
            ++value_;
            return copy;
        }

        constexpr Unit& operator+=(float value)
        {
            value_ += value;
            return *this;
        }
        constexpr Unit& operator-=(float value)
        {
            value_ -= value;
            return *this;
        }
        constexpr Unit& operator*=(float value)
        {
            value_ *= value;
            return *this;
        }
        constexpr Unit& operator/=(float value)
        {
            value_ /= value;
            return *this;
        }

        constexpr Unit operator+(float value) const
        {
            return Unit(value_ + value, units_.type, units_.mods);
        }
        constexpr Unit operator-(float value) const
        {
            return Unit(value_ + value, units_.type, units_.mods);
        }
        constexpr Unit operator*(float value) const
        {
            return Unit(value_ * value, units_.type, units_.mods);
        }
        constexpr Unit operator/(float value) const
        {
            return Unit(value_ / value, units_.type, units_.mods);
        }

        constexpr Unit operator|(Mods mods) const
        {
            return Unit(value_, units_.type, units_.mods | mods);
        }

        constexpr bool operator<(float value) const
        {
            return value_ < value;
        }
        constexpr bool operator>(float value) const
        {
            return value_ > value;
        }
        constexpr bool operator<=(float value) const
        {
            return value_ <= value;
        }
        constexpr bool operator>=(float value) const
        {
            return value_ >= value;
        }
        constexpr bool operator==(float value) const
        {
            return value_ == value;
        }
        constexpr bool operator!=(float value) const
        {
            return value_ != value;
        }
    };

    enum UnitContext : unsigned char
    {
        Horizontal = Unit::Mods::Horizontal,
        Vertical = Unit::Mods::Vertical,
        Position = Unit::Mods::_Position,
        Dimensions = Unit::Mods::_Dimensions,
    };

    template<unsigned char Ctx>
    class AlignedUnit : public Unit
    {
    private:
        void _setctx()
        {
            const auto mods = getmods();
            setmods(
                (Ctx & UnitContext::Horizontal) * !(mods & UnitContext::Vertical) |
                (Ctx & UnitContext::Vertical) * !(mods & UnitContext::Horizontal) |
                (Ctx & (UnitContext::Dimensions | UnitContext::Position))
            );
        }
    public:
        constexpr AlignedUnit() = default;
        constexpr AlignedUnit(const Unit& value)
            : Unit(value)
        {
            _setctx();
        }
        constexpr AlignedUnit(float value, Type units = Px, unsigned char mods = 0x00)
            : Unit(value, units, mods)
        {
            _setctx();
        }
        constexpr AlignedUnit(Type units, unsigned char mods = 0x00)
            : Unit(units, mods)
        {
            _setctx();
        }
        constexpr AlignedUnit(Mods mods) : Unit(mods)
        {
            _setctx();
        }
        constexpr AlignedUnit(const AlignedUnit&) = default;
        constexpr AlignedUnit(AlignedUnit&&) = default;

        consteval bool getcontext(UnitContext value) const
        {
            return Ctx & value;
        }
        consteval bool getcontext(UnitContext value)
        {
            return Ctx & value;
        }

        constexpr AlignedUnit& operator=(const AlignedUnit&) = default;
        constexpr AlignedUnit& operator=(AlignedUnit&&) = default;
        constexpr Unit& operator=(Mods mods)
        {
            return operator=(AlignedUnit(mods));
        }
        constexpr Unit& operator=(Type units)
        {
            return operator=(AlignedUnit(units));
        }
    };

    // Aliases for conventionalism

    using HDUnit = AlignedUnit<UnitContext::Horizontal | UnitContext::Dimensions>;
    using VDUnit = AlignedUnit<UnitContext::Vertical | UnitContext::Dimensions>;
    using HPUnit = AlignedUnit<UnitContext::Horizontal | UnitContext::Position>;
    using VPUnit = AlignedUnit<UnitContext::Vertical | UnitContext::Position>;
}

constexpr d2::Unit operator""_px(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Px);
}
constexpr d2::Unit operator""_pxi(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Px, d2::Unit::Inverted);
}

constexpr d2::Unit operator""_pv(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Pv);
}
constexpr d2::Unit operator""_pvi(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Pv, d2::Unit::Inverted);
}

constexpr d2::Unit operator""_pc(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Pc);
}
constexpr d2::Unit operator""_pci(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Pc, d2::Unit::Inverted);
}

constexpr d2::Unit operator""_cpx(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Px, d2::Unit::Mods::Center);
}
constexpr d2::Unit operator""_cpxi(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Px, d2::Unit::Inverted | d2::Unit::Mods::Center);
}

constexpr d2::Unit operator""_cpv(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Pv, d2::Unit::Mods::Center);
}
constexpr d2::Unit operator""_cpvi(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Pv, d2::Unit::Inverted | d2::Unit::Mods::Center);
}

constexpr d2::Unit operator""_cpc(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Pc, d2::Unit::Mods::Center);
}
constexpr d2::Unit operator""_cpci(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Pc, d2::Unit::Inverted | d2::Unit::Mods::Center);
}

constexpr d2::Unit operator""_center(long double value)
{
    return d2::Unit(value, d2::Unit::Px, d2::Unit::Mods::Center);
}
constexpr d2::Unit operator""_auto(long double value)
{
    return d2::Unit(value, d2::Unit::Type::Auto);
}
constexpr d2::Unit operator""_relative(long double value)
{
    return d2::Unit(value, d2::Unit::Px, d2::Unit::Mods::Relative);
}

#endif // D2_ELEMENT_UNITS_HPP
