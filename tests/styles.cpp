#include "check.hpp"
#include <elements/d2_box.hpp>

// Check outside the UAI's friend context before any derived interface is instantiated.
static_assert(d2::style::impl::chained<d2::dx::Box>);
static_assert(!d2::style::impl::chained<d2::VecParentElement>);

#include <elements/d2_draggable_box.hpp>
#include <elements/d2_switch.hpp>

namespace d2::style
{
    D2_UAI_INTERFACE(
        TestLayer,
        D2_UAI_OPTS(),
        D2_UAI_FIELDS(int extra_value{};),
        D2_UAI_PROPS(ExtraValue),
        D2_UAI_LINK(D2_UAI_PROP(ExtraValue, extra_value, 0x00))
    )
}

using namespace d2;

class LayerBox : public style::UAIC<dx::VirtualBox, LayerBox, style::ITestLayer>
{
public:
    using data = style::UAIC<dx::VirtualBox, LayerBox, style::ITestLayer>;
    D2_UAI_CHAIN(dx::VirtualBox)
};

static_assert(dx::VirtualBox::Title == style::impl::ResolveChain<dx::Box>::offset);
static_assert(dx::VerticalSwitch::SliderBackgroundColor ==
    style::impl::ResolveChain<dx::Switch>::offset);
static_assert(LayerBox::ExtraValue == style::impl::ResolveChain<dx::VirtualBox>::offset);
static_assert(std::is_same_v<LayerBox::type_of<LayerBox::Width>, HDUnit>);
static_assert(std::is_same_v<LayerBox::type_of<LayerBox::Title>, string>);
static_assert(std::is_same_v<LayerBox::type_of<LayerBox::ExtraValue>, int>);
static_assert(std::is_same_v<dx::VerticalSwitch::type_of<dx::VerticalSwitch::Options>,
    std::vector<string>>);

// Instantiate the setters used by DebugBox without requiring a running screen.
[[maybe_unused]] void setters(LayerBox& box, dx::VerticalSwitch& choice)
{
    box.set<LayerBox::Width>(82.0_px)
       .set<LayerBox::Height>(26.0_px)
       .set<LayerBox::ContainerBorder>(true)
       .set<LayerBox::Title>("test")
       .set<LayerBox::ExtraValue>(42);
    choice.set<dx::VerticalSwitch::Width>(20.0_px)
          .set<dx::VerticalSwitch::Height>(10.0_px)
          .set<dx::VerticalSwitch::Y>(1.0_px)
          .set<dx::VerticalSwitch::Options>(std::vector<string>{"one", "two"})
          .set<dx::VerticalSwitch::OnChangeValues>(
              [](TreeIter<dx::Switch>, string, string) {});
}

int main()
{
    LayerBox box("box", nullptr);
    auto& base = static_cast<dx::Box&>(box);
    auto& parent = static_cast<dx::VirtualBox&>(box);
    CHECK(box.getref<LayerBox::Width>() == base.getref<dx::Box::Width>());
    CHECK(box.getref<LayerBox::Title>() == parent.getref<dx::VirtualBox::Title>());
    *box.getref<LayerBox::Width>() = 82.0_px;
    *box.getref<LayerBox::Title>() = "test";
    *box.getref<LayerBox::ExtraValue>() = 42;
    CHECK(base.getref<dx::Box::Width>()->raw() == 82);
    CHECK(*parent.getref<dx::VirtualBox::Title>() == "test");
    CHECK(*box.getref<LayerBox::ExtraValue>() == 42);

    dx::VerticalSwitch choice("choice", nullptr);
    auto& switch_base = static_cast<dx::Switch&>(choice);
    CHECK(choice.getref<dx::VerticalSwitch::Options>() ==
        switch_base.getref<dx::Switch::Options>());
    *choice.getref<dx::VerticalSwitch::Options>() = {"one", "two"};
    CHECK(switch_base.getref<dx::Switch::Options>()->size() == 2);
    *choice.getref<dx::VerticalSwitch::SliderColor>() = pixel{.v = '#'};
    CHECK(choice.getref<dx::VerticalSwitch::SliderColor>()->v == '#');
}
