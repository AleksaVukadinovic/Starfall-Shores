#ifndef MYAPP_HPP
#define MYAPP_HPP
#include <engine/core/App.hpp>

namespace app {
    class MyApp : public engine::core::App {
        void app_setup() override;
    };
}

#endif //MYAPP_HPP
