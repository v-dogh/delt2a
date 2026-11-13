# `d2::Element`

**Kind:** class
**Declared in:** `core/tree/d2_tree_element.hpp:25`

**Bases:** `std::enable_shared_from_this< Element >`, `d2::style::UniversalAccessInterfaceBase`

## Enums

### `State`

```cpp
enum State
```

Source: `core/tree/d2_tree_element.hpp:74`

Values: `Invalid`, `Display`, `Swapped`, `Created`, `Embedded`, `Event`, `RcEvent`, `Clicked`, `Hovered`, `Focused`, `RcClick`, `RcHover`, `RcFocus`, `Keynavi`, `Public1`, `Public2`, `Public3`

### `InternalState`

```cpp
enum InternalState
```

Source: `core/tree/d2_tree_element.hpp:94`

Values: `IsBeingRendered`, `IsBeingInitialized`, `WasWritten`, `WasWrittenLayout`, `PositionXUpdated`, `PositionYUpdated`, `DimensionsWidthUpdated`, `DimensionsHeightUpdated`, `CachePolicyDynamic`, `CachePolicyVolatile`, `CachePolicyStatic`, `CachePolicyBypass`

### `WriteType`

```cpp
enum WriteType
```

Source: `core/tree/d2_tree_element.hpp:120`

Values: `None`, `Style`, `InternalLayout`, `LayoutXPos`, `LayoutYPos`, `LayoutWidth`, `LayoutHeight`, `Dimensions`, `Offset`, `Masked`, `Strip`, `StripPerm`

### `UnitState`

```cpp
enum UnitState
```

Source: `core/tree/d2_tree_element.hpp:135`

Values: `RelativeXPos`, `RelativeYPos`, `RelativeWidth`, `RelativeHeight`, `ContextualXPos`, `ContextualYPos`, `ContextualWidth`, `ContextualHeight`, `InvertedXPos`, `InvertedYPos`, `InvertedWidth`, `InvertedHeight`

### `CachePolicy`

```cpp
enum CachePolicy
```

Source: `core/tree/d2_tree_element.hpp:157`

Values: `Static`, `Dynamic`, `Volatile`

### `Layout`

```cpp
enum Layout
```

Source: `core/tree/d2_tree_element.hpp:163`

Values: `X`, `Y`, `Width`, `Height`


## Friends

### `internal::ElementView`

```cpp
friend class internal::ElementView
```

Source: `core/tree/d2_tree_element.hpp:459`


## Functions

### `_is_write_type`

```cpp
bool d2::Element::_is_write_type(write_flag type) const
```

Source: `core/tree/d2_tree_element.hpp:389`

### `_signal_write_child_impl`

```cpp
virtual void d2::Element::_signal_write_child_impl(write_flag type, unsigned int prop, ptr element)
```

Source: `core/tree/d2_tree_element.hpp:391`

### `_signal_write_impl`

```cpp
virtual void d2::Element::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
```

Source: `core/tree/d2_tree_element.hpp:392`

### `_signal_context_change_impl`

```cpp
virtual void d2::Element::_signal_context_change_impl(write_flag type, unsigned int prop, ptr element)
```

Source: `core/tree/d2_tree_element.hpp:393`

### `_signal_write_child`

```cpp
void d2::Element::_signal_write_child(write_flag type, unsigned int prop, ptr element)
```

Source: `core/tree/d2_tree_element.hpp:395`

### `_signal_write`

```cpp
void d2::Element::_signal_write(write_flag type, unsigned int prop, ptr element)
```

Source: `core/tree/d2_tree_element.hpp:396`

### `_clone_state_if`

```cpp
void d2::Element::_clone_state_if(ptr parent)
```

Source: `core/tree/d2_tree_element.hpp:398`

### `_obliterate_dep_bind`

```cpp
void d2::Element::_obliterate_dep_bind(style::uai_property prop)
```

Source: `core/tree/d2_tree_element.hpp:400`

### `_bind_dep`

```cpp
void d2::Element::_bind_dep(style::uai_property prop, style::DependencyHandle handle)
```

Source: `core/tree/d2_tree_element.hpp:401`

### `_signal_context_change_sub`

```cpp
void d2::Element::_signal_context_change_sub(write_flag type, unsigned int prop, ptr element)
```

Source: `core/tree/d2_tree_element.hpp:405`

### `_signal_context_change_sub`

```cpp
void d2::Element::_signal_context_change_sub(write_flag type, unsigned int prop)
```

Source: `core/tree/d2_tree_element.hpp:406`

### `_signal_context_change`

```cpp
void d2::Element::_signal_context_change(write_flag type, unsigned int prop, ptr element)
```

Source: `core/tree/d2_tree_element.hpp:407`

### `_signal_context_change`

```cpp
void d2::Element::_signal_context_change(write_flag type=WriteType::Masked, unsigned int prop=~0u)
```

Source: `core/tree/d2_tree_element.hpp:408`

### `_signal_write`

```cpp
void d2::Element::_signal_write(write_flag type=WriteType::Masked, unsigned int prop=~0u)
```

Source: `core/tree/d2_tree_element.hpp:409`

### `_signal_initialization`

```cpp
void d2::Element::_signal_initialization(unsigned int prop)
```

Source: `core/tree/d2_tree_element.hpp:410`

### `_signal_write_update`

```cpp
void d2::Element::_signal_write_update(write_flag type) const
```

Source: `core/tree/d2_tree_element.hpp:411`

### `_signal_update`

```cpp
void d2::Element::_signal_update(internal_flag type) const
```

Source: `core/tree/d2_tree_element.hpp:412`

### `_trigger_event`

```cpp
void d2::Element::_trigger_event(in::InputFrame &frame, bool recursive)
```

Source: `core/tree/d2_tree_element.hpp:413`

### `_layout_impl`

```cpp
virtual Unit d2::Element::_layout_impl(Layout) const =0
```

Source: `core/tree/d2_tree_element.hpp:417`

### `_provides_input_impl`

```cpp
virtual bool d2::Element::_provides_input_impl() const
```

Source: `core/tree/d2_tree_element.hpp:421`

### `_unit_report_impl`

```cpp
virtual unit_meta_flag d2::Element::_unit_report_impl() const =0
```

Source: `core/tree/d2_tree_element.hpp:425`

### `_index_impl`

```cpp
virtual char d2::Element::_index_impl() const =0
```

Source: `core/tree/d2_tree_element.hpp:426`

### `_event_impl`

```cpp
virtual void d2::Element::_event_impl(in::InputFrame &)
```

Source: `core/tree/d2_tree_element.hpp:430`

### `_state_change_impl`

```cpp
virtual void d2::Element::_state_change_impl(State, bool)
```

Source: `core/tree/d2_tree_element.hpp:431`

### `_reserve_buffer_impl`

```cpp
virtual BoundingBox d2::Element::_reserve_buffer_impl() const
```

Source: `core/tree/d2_tree_element.hpp:435`

### `_fetch_pixel_buffer_impl`

```cpp
PixelBuffer::View d2::Element::_fetch_pixel_buffer_impl() const
```

Source: `core/tree/d2_tree_element.hpp:439`

### `_provides_buffer_impl`

```cpp
virtual bool d2::Element::_provides_buffer_impl() const
```

Source: `core/tree/d2_tree_element.hpp:440`

### `_update_layout_impl`

```cpp
virtual void d2::Element::_update_layout_impl()
```

Source: `core/tree/d2_tree_element.hpp:447`

### `_update_style_impl`

```cpp
virtual void d2::Element::_update_style_impl()
```

Source: `core/tree/d2_tree_element.hpp:448`

### `_frame_impl`

```cpp
virtual void d2::Element::_frame_impl(PixelBuffer::View)=0
```

Source: `core/tree/d2_tree_element.hpp:449`

### `make`

```cpp
template<typename Type, typename... Argv>
static auto d2::Element::make(const std::string &name, TreeState::ptr state, Argv &&... args)
```

Source: `core/tree/d2_tree_element.hpp:452`

### `Element`

```cpp
d2::Element::Element()=default
```

Source: `core/tree/d2_tree_element.hpp:461`

### `Element`

```cpp
d2::Element::Element(const std::string &name, TreeState::ptr state)
```

Source: `core/tree/d2_tree_element.hpp:462`

### `Element`

```cpp
d2::Element::Element(Element &&)=delete
```

Source: `core/tree/d2_tree_element.hpp:463`

### `Element`

```cpp
d2::Element::Element(const Element &)=delete
```

Source: `core/tree/d2_tree_element.hpp:464`

### `~Element`

```cpp
virtual d2::Element::~Element()=default
```

Source: `core/tree/d2_tree_element.hpp:465`

### `remove`

```cpp
void d2::Element::remove()
```

Source: `core/tree/d2_tree_element.hpp:469`

### `extract`

```cpp
Element::ptr d2::Element::extract()
```

Source: `core/tree/d2_tree_element.hpp:470`

### `screen`

```cpp
sys::module< sys::screen > d2::Element::screen() const
```

Source: `core/tree/d2_tree_element.hpp:474`

### `context`

```cpp
IOContext::ptr d2::Element::context() const
```

Source: `core/tree/d2_tree_element.hpp:475`

### `state`

```cpp
TreeState::ptr d2::Element::state() const
```

Source: `core/tree/d2_tree_element.hpp:476`

### `parent`

```cpp
Element::pptr d2::Element::parent() const
```

Source: `core/tree/d2_tree_element.hpp:477`

### `name`

```cpp
const std::string & d2::Element::name() const
```

Source: `core/tree/d2_tree_element.hpp:478`

### `input_frame`

```cpp
in::InputFrame & d2::Element::input_frame() const
```

Source: `core/tree/d2_tree_element.hpp:479`

### `getistate`

```cpp
bool d2::Element::getistate(internal_flag flags) const
```

Source: `core/tree/d2_tree_element.hpp:483`

### `setcache`

```cpp
void d2::Element::setcache(CachePolicy flag) const
```

Source: `core/tree/d2_tree_element.hpp:484`

### `getstate`

```cpp
bool d2::Element::getstate(State state) const
```

Source: `core/tree/d2_tree_element.hpp:485`

### `setstate`

```cpp
void d2::Element::setstate(State state, bool value=true)
```

Source: `core/tree/d2_tree_element.hpp:486`

### `depth`

```cpp
std::size_t d2::Element::depth()
```

Source: `core/tree/d2_tree_element.hpp:487`

### `unit_report`

```cpp
Element::unit_meta_flag d2::Element::unit_report() const
```

Source: `core/tree/d2_tree_element.hpp:491`

### `provides_cursor_sink`

```cpp
bool d2::Element::provides_cursor_sink() const
```

Source: `core/tree/d2_tree_element.hpp:493`

### `provides_input`

```cpp
bool d2::Element::provides_input() const
```

Source: `core/tree/d2_tree_element.hpp:494`

### `needs_update`

```cpp
bool d2::Element::needs_update() const
```

Source: `core/tree/d2_tree_element.hpp:495`

### `listen`

```cpp
Element::EventListener d2::Element::listen(State event, bool value, event_callback callback)
```

Source: `core/tree/d2_tree_element.hpp:497`

### `listen`

```cpp
Element::EventListener d2::Element::listen(State event, event_callback callback)
```

Source: `core/tree/d2_tree_element.hpp:498`

### `frame`

```cpp
Element::Frame d2::Element::frame()
```

Source: `core/tree/d2_tree_element.hpp:502`

### `box`

```cpp
BoundingBox d2::Element::box() const
```

Source: `core/tree/d2_tree_element.hpp:504`

### `position`

```cpp
Position d2::Element::position() const
```

Source: `core/tree/d2_tree_element.hpp:505`

### `internal_box`

```cpp
BoundingBox d2::Element::internal_box() const
```

Source: `core/tree/d2_tree_element.hpp:506`

### `internal_position`

```cpp
Position d2::Element::internal_position() const
```

Source: `core/tree/d2_tree_element.hpp:507`

### `position_screen_space`

```cpp
Position d2::Element::position_screen_space() const
```

Source: `core/tree/d2_tree_element.hpp:509`

### `mouse_object_space`

```cpp
Position d2::Element::mouse_object_space() const
```

Source: `core/tree/d2_tree_element.hpp:510`

### `layout`

```cpp
int d2::Element::layout(Layout type) const
```

Source: `core/tree/d2_tree_element.hpp:512`

### `internal_layout`

```cpp
Unit d2::Element::internal_layout(Layout type) const
```

Source: `core/tree/d2_tree_element.hpp:513`

### `accept_layout`

```cpp
void d2::Element::accept_layout() const
```

Source: `core/tree/d2_tree_element.hpp:515`

### `accept_layout`

```cpp
void d2::Element::accept_layout(Layout type) const
```

Source: `core/tree/d2_tree_element.hpp:516`

### `override_layout`

```cpp
void d2::Element::override_layout(Layout type, int value) const
```

Source: `core/tree/d2_tree_element.hpp:517`

### `override_dimensions`

```cpp
void d2::Element::override_dimensions(int width, int height) const
```

Source: `core/tree/d2_tree_element.hpp:518`

### `override_position`

```cpp
void d2::Element::override_position(int x, int y) const
```

Source: `core/tree/d2_tree_element.hpp:519`

### `contextual_layout`

```cpp
bool d2::Element::contextual_layout(Layout type) const
```

Source: `core/tree/d2_tree_element.hpp:521`

### `relative_layout`

```cpp
bool d2::Element::relative_layout(Layout type) const
```

Source: `core/tree/d2_tree_element.hpp:522`

### `getzindex`

```cpp
char d2::Element::getzindex() const
```

Source: `core/tree/d2_tree_element.hpp:524`

### `resolve_units`

```cpp
int d2::Element::resolve_units(Unit unit) const
```

Source: `core/tree/d2_tree_element.hpp:526`

### `traverse`

```cpp
TreeIter d2::Element::traverse()
```

Source: `core/tree/d2_tree_element.hpp:528`

### `operator+`

```cpp
TreeIter d2::Element::operator+()
```

Source: `core/tree/d2_tree_element.hpp:529`

### `operator=`

```cpp
Element & d2::Element::operator=(const Element &)=delete
```

Source: `core/tree/d2_tree_element.hpp:531`

### `operator=`

```cpp
Element & d2::Element::operator=(Element &&)=delete
```

Source: `core/tree/d2_tree_element.hpp:532`


## Typedefs

### `eptr`

```cpp
template<typename Type>
using d2::Element::eptr = std::shared_ptr<Type>
```

Source: `core/tree/d2_tree_element.hpp:30`

### `ptr`

```cpp
using d2::Element::ptr = std::shared_ptr<Element>
```

Source: `core/tree/d2_tree_element.hpp:31`

### `cptr`

```cpp
using d2::Element::cptr = std::shared_ptr<const Element>
```

Source: `core/tree/d2_tree_element.hpp:32`

### `wptr`

```cpp
using d2::Element::wptr = std::weak_ptr<Element>
```

Source: `core/tree/d2_tree_element.hpp:33`

### `pptr`

```cpp
using d2::Element::pptr = std::shared_ptr<ParentElement>
```

Source: `core/tree/d2_tree_element.hpp:34`

### `cpptr`

```cpp
using d2::Element::cpptr = std::shared_ptr<const ParentElement>
```

Source: `core/tree/d2_tree_element.hpp:35`

### `pwptr`

```cpp
using d2::Element::pwptr = std::weak_ptr<ParentElement>
```

Source: `core/tree/d2_tree_element.hpp:36`

### `iptr`

```cpp
using d2::Element::iptr = TreeIter<>
```

Source: `core/tree/d2_tree_element.hpp:37`

### `unit_meta_flag`

```cpp
using d2::Element::unit_meta_flag = unsigned short
```

Source: `core/tree/d2_tree_element.hpp:63`

### `internal_flag`

```cpp
using d2::Element::internal_flag = unsigned short
```

Source: `core/tree/d2_tree_element.hpp:64`

### `state_flag`

```cpp
using d2::Element::state_flag = unsigned short
```

Source: `core/tree/d2_tree_element.hpp:65`

### `write_flag`

```cpp
using d2::Element::write_flag = element_write_flag
```

Source: `core/tree/d2_tree_element.hpp:66`

### `event_callback`

```cpp
using d2::Element::event_callback = std::function<void(EventListener, TreeIter<>)>
```

Source: `core/tree/d2_tree_element.hpp:68`

### `foreach_callback`

```cpp
using d2::Element::foreach_callback = TreeIter<>::foreach_callback
```

Source: `core/tree/d2_tree_element.hpp:69`

### `foreach_internal_callback`

```cpp
using d2::Element::foreach_internal_callback = std::function<bool(ptr)>
```

Source: `core/tree/d2_tree_element.hpp:70`
