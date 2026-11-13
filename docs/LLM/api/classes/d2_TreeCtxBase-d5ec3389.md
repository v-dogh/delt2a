# `d2::TreeCtxBase`

**Kind:** class
**Declared in:** `core/tree/d2_tree_construct.hpp:60`

## Functions

### `_reg_elem`

```cpp
void d2::TreeCtxBase::_reg_elem(TreeIter<> ptr) const
```

Source: `core/tree/d2_tree_construct.hpp:87`

### `_get_branch`

```cpp
static std::vector< TreeIter<> > d2::TreeCtxBase::_get_branch()
```

Source: `core/tree/d2_tree_construct.hpp:92`

### `TreeCtxBase`

```cpp
d2::TreeCtxBase::TreeCtxBase(d2::TreeIter<> ptr)
```

Source: `core/tree/d2_tree_construct.hpp:97`

### `TreeCtxBase`

```cpp
d2::TreeCtxBase::TreeCtxBase(const TreeCtxBase &)=default
```

Source: `core/tree/d2_tree_construct.hpp:98`

### `TreeCtxBase`

```cpp
d2::TreeCtxBase::TreeCtxBase(TreeCtxBase &&)=default
```

Source: `core/tree/d2_tree_construct.hpp:99`

### `context`

```cpp
IOContext::ptr d2::TreeCtxBase::context() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:101`

### `scheduler`

```cpp
mt::ConcurrentPool::ptr d2::TreeCtxBase::scheduler() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:105`

### `screen`

```cpp
sys::module< sys::screen > d2::TreeCtxBase::screen() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:110`

### `input`

```cpp
sys::module< sys::input > d2::TreeCtxBase::input() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:114`

### `output`

```cpp
sys::module< sys::output > d2::TreeCtxBase::output() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:118`

### `tag`

```cpp
template<meta::ConstString Name, typename Type>
void d2::TreeCtxBase::tag(Type &&value) const
```

Source: `core/tree/d2_tree_construct.hpp:125`

### `dep`

```cpp
template<typename Type>
auto & d2::TreeCtxBase::dep(const std::string &name) const
```

Source: `core/tree/d2_tree_construct.hpp:142`

### `dep`

```cpp
template<typename Type>
auto & d2::TreeCtxBase::dep(const std::string &tree, const std::string &name) const
```

Source: `core/tree/d2_tree_construct.hpp:146`

### `var`

```cpp
template<auto Var>
auto & d2::TreeCtxBase::var() const
```

Source: `core/tree/d2_tree_construct.hpp:151`

### `dynavar`

```cpp
template<auto Var, auto Callback>
auto d2::TreeCtxBase::dynavar() const
```

Source: `core/tree/d2_tree_construct.hpp:155`

### `subscribe`

```cpp
template<typename Type, typename Func>
style::DependencyHandle d2::TreeCtxBase::subscribe(style::Dependency< Type > &dep, Func &&callback) const
```

Source: `core/tree/d2_tree_construct.hpp:161`

### `subscribe`

```cpp
template<typename Type, typename Func>
style::DependencyHandle d2::TreeCtxBase::subscribe(const std::string &name, Func &&callback) const
```

Source: `core/tree/d2_tree_construct.hpp:180`

### `subscribe`

```cpp
template<typename Type, typename Func>
style::DependencyHandle d2::TreeCtxBase::subscribe(const std::string &tree, const std::string &name, Func &&callback) const
```

Source: `core/tree/d2_tree_construct.hpp:186`

### `declare`

```cpp
template<typename Type, typename... Argv>
Type & d2::TreeCtxBase::declare(Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:197`

### `repeat`

```cpp
template<typename Func, typename... Argv>
auto d2::TreeCtxBase::repeat(std::chrono::milliseconds timing, Func &&callback, Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:211`

### `defer`

```cpp
template<typename Func, typename... Argv>
auto d2::TreeCtxBase::defer(std::chrono::milliseconds delay, Func &&callback, Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:218`

### `async`

```cpp
template<typename Func, typename... Argv>
auto d2::TreeCtxBase::async(Func &&callback, Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:224`

### `sync`

```cpp
template<typename Func, typename... Argv>
auto d2::TreeCtxBase::sync(Func &&callback, Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:228`

### `sync_async`

```cpp
template<typename Func, typename... Argv>
auto d2::TreeCtxBase::sync_async(Func &&callback, Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:234`

### `on`

```cpp
template<typename Event, typename Func>
Signals::Handle d2::TreeCtxBase::on(Event ev, Func &&callback) const
```

Source: `core/tree/d2_tree_construct.hpp:240`

### `onv`

```cpp
Element::EventListener d2::TreeCtxBase::onv(Element::State state, bool value, Element::event_callback callback) const
```

Source: `core/tree/d2_tree_construct.hpp:245`

### `onv`

```cpp
Element::EventListener d2::TreeCtxBase::onv(Element::State state, Element::event_callback callback) const
```

Source: `core/tree/d2_tree_construct.hpp:249`

### `operator=`

```cpp
TreeCtxBase & d2::TreeCtxBase::operator=(const TreeCtxBase &)=default
```

Source: `core/tree/d2_tree_construct.hpp:254`

### `operator=`

```cpp
TreeCtxBase & d2::TreeCtxBase::operator=(TreeCtxBase &&)=default
```

Source: `core/tree/d2_tree_construct.hpp:255`

### `operator==`

```cpp
bool d2::TreeCtxBase::operator==(std::nullptr_t) const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:257`

### `operator!=`

```cpp
bool d2::TreeCtxBase::operator!=(std::nullptr_t) const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:261`


## Variables

### `_ptr`

```cpp
TreeIter d2::TreeCtxBase::_ptr
```

Source: `core/tree/d2_tree_construct.hpp:85`
