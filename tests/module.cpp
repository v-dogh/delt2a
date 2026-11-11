#include "check.hpp"
#include "core/io/d2_module.hpp"
#include <thread>

struct Module : d2::sys::SystemModule
{
    int loads = 0, unloads = 0, posts = 0;
    Status pre = Status::Ok, result = Status::Ok;
    bool fail = false;
    Module() : SystemModule({}, {}, sizeof(Module)) {}
    ModInfo info() const override { return {"Test", typeid(Module)}; }
    Status _pre_load_impl() override { return pre; }
    Status _load_impl() override
    {
        ++loads;
        if (fail)
            throw std::runtime_error("load failed");
        return result;
    }
    Status _unload_impl() override { ++unloads; return Status::Ok; }
    Status _post_unload_impl() override { ++posts; return Status::Ok; }
};

int main()
{
    Module module;
    std::vector<std::jthread> threads;
    for (int i = 0; i < 8; ++i)
        threads.emplace_back([&] { module.load(); });
    threads.clear();
    CHECK(module.loads == 1);
    for (int i = 0; i < 8; ++i)
        threads.emplace_back([&] { module.unload(); });
    threads.clear();
    CHECK(module.unloads == 1 && module.posts == 1);
    CHECK(module.status() == Module::Status::Offline);
    module.load();
    CHECK(module.loads == 2);
    module.unload();
    module.fail = true;
    module.load();
    CHECK(module.status() == Module::Status::Failure);
    module.unload();
    CHECK(module.posts == 3);
    module.fail = false;
    module.pre = Module::Status::Failure;
    module.load();
    CHECK(module.status() == Module::Status::Failure && module.loads == 3);
    CHECK(!module.accessible());
}
