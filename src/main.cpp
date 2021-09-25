#include "engine.h"

int main() {
    Engine* engine = new Engine();

    engine->Init();
    
    try {
        engine->Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    delete engine;

    return EXIT_SUCCESS;
}