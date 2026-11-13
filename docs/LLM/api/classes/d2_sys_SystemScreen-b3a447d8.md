# `d2::sys::SystemScreen`

**Kind:** class
**Declared in:** `core/screen/d2_screen.hpp:23`

**Bases:** `d2::sys::Module< SystemScreen, "Screen", Access::TUnsafe, Load::Immediate, SystemInput, SystemOutput >`

## Enums

### `Event`

```cpp
enum Event
```

Source: `core/screen/d2_screen.hpp:34`

Values: `Resize`, `PreRedraw`, `PostRedraw`, `KeyInput`, `KeySequenceInput`, `KeyMouseInput`, `MouseMoved`, `MouseInput`, `Update`, `TreeSwap`


## Functions

### `fps`

```cpp
static constexpr std::chrono::milliseconds d2::sys::SystemScreen::fps(std::size_t c)
```

Source: `core/screen/d2_screen.hpp:143`

### `root`

```cpp
TreeIter< ParentElement > d2::sys::SystemScreen::root() const
```

Source: `core/screen/d2_screen.hpp:152`

### `fetch_model`

```cpp
MatrixModel::ptr d2::sys::SystemScreen::fetch_model(const std::string &name, const std::string &path)
```

Source: `core/screen/d2_screen.hpp:156`

### `fetch_model`

```cpp
MatrixModel::ptr d2::sys::SystemScreen::fetch_model(const std::string &name, const std::string &path, ModelType type)
```

Source: `core/screen/d2_screen.hpp:158`

### `fetch_model`

```cpp
MatrixModel::ptr d2::sys::SystemScreen::fetch_model(const std::string &name, int width, int height, std::vector< pixel > data)
```

Source: `core/screen/d2_screen.hpp:160`

### `fetch_model`

```cpp
MatrixModel::ptr d2::sys::SystemScreen::fetch_model(const std::string &name, int width, int height, std::span< const pixel > data)
```

Source: `core/screen/d2_screen.hpp:162`

### `fetch_model`

```cpp
MatrixModel::ptr d2::sys::SystemScreen::fetch_model(const std::string &name)
```

Source: `core/screen/d2_screen.hpp:163`

### `remove_model`

```cpp
void d2::sys::SystemScreen::remove_model(const std::string &name)
```

Source: `core/screen/d2_screen.hpp:164`

### `focus`

```cpp
void d2::sys::SystemScreen::focus(eptr p)
```

Source: `core/screen/d2_screen.hpp:168`

### `focused`

```cpp
SystemScreen::eptr d2::sys::SystemScreen::focused() const
```

Source: `core/screen/d2_screen.hpp:169`

### `clicked`

```cpp
SystemScreen::eptr d2::sys::SystemScreen::clicked() const
```

Source: `core/screen/d2_screen.hpp:170`

### `hovered`

```cpp
SystemScreen::eptr d2::sys::SystemScreen::hovered() const
```

Source: `core/screen/d2_screen.hpp:171`

### `focus_next_minor`

```cpp
void d2::sys::SystemScreen::focus_next_minor(bool reverse=false)
```

Source: `core/screen/d2_screen.hpp:173`

### `focus_next_major`

```cpp
void d2::sys::SystemScreen::focus_next_major(bool reverse=false)
```

Source: `core/screen/d2_screen.hpp:174`

### `clear_keynav`

```cpp
void d2::sys::SystemScreen::clear_keynav()
```

Source: `core/screen/d2_screen.hpp:175`

### `fps`

```cpp
std::size_t d2::sys::SystemScreen::fps() const
```

Source: `core/screen/d2_screen.hpp:177`

### `animations`

```cpp
std::size_t d2::sys::SystemScreen::animations() const
```

Source: `core/screen/d2_screen.hpp:178`

### `delta`

```cpp
std::chrono::microseconds d2::sys::SystemScreen::delta() const
```

Source: `core/screen/d2_screen.hpp:179`

### `is_keynav`

```cpp
bool d2::sys::SystemScreen::is_keynav() const
```

Source: `core/screen/d2_screen.hpp:181`

### `apply`

```cpp
void d2::sys::SystemScreen::apply(const TreeIter<>::foreach_callback &func) const
```

Source: `core/screen/d2_screen.hpp:185`

### `apply_all`

```cpp
void d2::sys::SystemScreen::apply_all(const TreeIter<>::foreach_callback &func) const
```

Source: `core/screen/d2_screen.hpp:186`

### `set`

```cpp
template<typename First, typename... Rest>
void d2::sys::SystemScreen::set()
```

Source: `core/screen/d2_screen.hpp:191`

### `set`

```cpp
void d2::sys::SystemScreen::set(std::string_view name)
```

Source: `core/screen/d2_screen.hpp:219`

### `state`

```cpp
template<typename Type = void>
auto d2::sys::SystemScreen::state() const
```

Source: `core/screen/d2_screen.hpp:221`

### `state`

```cpp
template<typename Type = void>
auto d2::sys::SystemScreen::state(std::string_view name) const
```

Source: `core/screen/d2_screen.hpp:232`

### `name`

```cpp
std::string d2::sys::SystemScreen::name()
```

Source: `core/screen/d2_screen.hpp:251`

### `erase_tree`

```cpp
void d2::sys::SystemScreen::erase_tree(std::string_view name)
```

Source: `core/screen/d2_screen.hpp:253`

### `erase_tree`

```cpp
void d2::sys::SystemScreen::erase_tree()
```

Source: `core/screen/d2_screen.hpp:254`

### `clear_tree`

```cpp
void d2::sys::SystemScreen::clear_tree()
```

Source: `core/screen/d2_screen.hpp:255`

### `tags`

```cpp
TreeTags & d2::sys::SystemScreen::tags()
```

Source: `core/screen/d2_screen.hpp:257`

### `deps`

```cpp
DynamicDependencyManager & d2::sys::SystemScreen::deps()
```

Source: `core/screen/d2_screen.hpp:258`

### `deps`

```cpp
DynamicDependencyManager & d2::sys::SystemScreen::deps(std::string_view name)
```

Source: `core/screen/d2_screen.hpp:259`

### `input`

```cpp
in::InputFrame * d2::sys::SystemScreen::input()
```

Source: `core/screen/d2_screen.hpp:262`

### `animate`

```cpp
template<typename Type, typename... Argv>
Animation::ptr d2::sys::SystemScreen::animate(std::chrono::milliseconds time, std::function< void(d2::TreeIter<>)> finish, d2::TreeIter<> elem, Argv &&... args)
```

Source: `core/screen/d2_screen.hpp:268`

### `animate`

```cpp
template<typename Type, typename... Argv>
Animation::ptr d2::sys::SystemScreen::animate(std::chrono::milliseconds time, d2::TreeIter<> target, Argv &&... args)
```

Source: `core/screen/d2_screen.hpp:295`

### `clear_animations`

```cpp
void d2::sys::SystemScreen::clear_animations(TreeIter<> ptr)
```

Source: `core/screen/d2_screen.hpp:300`

### `clear_animations`

```cpp
void d2::sys::SystemScreen::clear_animations(TreeIter<> ptr, style::uai_property prop)
```

Source: `core/screen/d2_screen.hpp:301`

### `theme`

```cpp
template<typename Type>
Type & d2::sys::SystemScreen::theme()
```

Source: `core/screen/d2_screen.hpp:305`

### `theme`

```cpp
template<typename Type>
Type & d2::sys::SystemScreen::theme(const std::string &id)
```

Source: `core/screen/d2_screen.hpp:326`

### `themev`

```cpp
template<auto Var>
auto & d2::sys::SystemScreen::themev()
```

Source: `core/screen/d2_screen.hpp:349`

### `themev`

```cpp
template<auto Var>
auto & d2::sys::SystemScreen::themev(const std::string &id)
```

Source: `core/screen/d2_screen.hpp:353`

### `has_theme`

```cpp
template<typename Type>
bool d2::sys::SystemScreen::has_theme()
```

Source: `core/screen/d2_screen.hpp:358`

### `has_theme`

```cpp
bool d2::sys::SystemScreen::has_theme(const std::string &id)
```

Source: `core/screen/d2_screen.hpp:362`

### `make_theme`

```cpp
template<typename... Themes>
void d2::sys::SystemScreen::make_theme(Themes &&... themes)
```

Source: `core/screen/d2_screen.hpp:367`

### `set_refresh_rate`

```cpp
void d2::sys::SystemScreen::set_refresh_rate(std::chrono::milliseconds refresh)
```

Source: `core/screen/d2_screen.hpp:380`

### `tick`

```cpp
void d2::sys::SystemScreen::tick()
```

Source: `core/screen/d2_screen.hpp:381`

### `traverse`

```cpp
SystemScreen::eptr d2::sys::SystemScreen::traverse()
```

Source: `core/screen/d2_screen.hpp:383`

### `operator/`

```cpp
SystemScreen::eptr d2::sys::SystemScreen::operator/(const std::string &path)
```

Source: `core/screen/d2_screen.hpp:384`


## Typedefs

### `EventManifest`

```cpp
using d2::sys::SystemScreen::EventManifest = SignalManifest< EvDefault<Event>, EvCase<Event::Resize, BoundingBox>, EvCase<Event::KeyInput, in::InputFrame&>, EvCase<Event::KeySequenceInput, d2::string_view>, EvCase<Event::KeyMouseInput, in::InputFrame&>, EvCase<Event::MouseMoved, Position>, EvCase<Event::MouseInput, in::InputFrame&>, EvCase<Event::TreeSwap, std::string, std::string> >
```

Source: `core/screen/d2_screen.hpp:47`

### `ModelType`

```cpp
using d2::sys::SystemScreen::ModelType = MatrixModel::ModelType
```

Source: `core/screen/d2_screen.hpp:59`
