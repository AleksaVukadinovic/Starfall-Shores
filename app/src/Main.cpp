#include <engine/core/Engine.hpp>
#include <MyApp.hpp>

int main(int argc, char** argv) {
    const auto app = std::make_unique<app::MyApp>();
    return app->run(argc, argv);
}
