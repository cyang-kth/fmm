#include "mm/h3mm/h3mm_app.hpp"

using namespace FMM;
using namespace FMM::MM;
int main(int argc, char **argv)
{
    H3MMAppConfig config(argc, argv);
    if (config.help_specified) {
        H3MMAppConfig::print_help();
        return 0;
    }
    if (!config.validate()) {
        return 0;
    }
    H3MMApp app(config);
    app.run();
    return 0;
};
