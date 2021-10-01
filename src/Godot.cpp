#include <Godot.hpp>
#include <Node.hpp>
#include <memory>
#include <set>

namespace godot 
{
class Example : public Node
{
    GODOT_CLASS(Example, Node)

public:
    static void _register_methods()
    {
        register_method("some_optimized_func", &Example::some_optimized_func);
    }

    void _init()
    {
    }

    String some_optimized_func()
    {
        return "ciao";
    }
};

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) 
{
    godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) 
{
    godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) 
{
    godot::Godot::nativescript_init(handle);
    godot::register_class<godot::Example>();
}
}
