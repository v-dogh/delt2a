# `d2::impl::args_sig_normalize`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:124`

## Typedefs

### `type`

```cpp
using d2::impl::args_sig_normalize< Type >::type = std::conditional_t<std::is_lvalue_reference_v<Type>, Type, const value_t&>
```

Source: `core/io/d2_signal_handler.hpp:130`
