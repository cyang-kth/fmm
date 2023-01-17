//
// Created by Can Yang on 2020/4/1.
//

#include "mm/fmm/ubodt_gen_app.hpp"
#include "mm/fmm/ubodt_gen_algorithm.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"
#include "util/debug.hpp"
#include <omp.h>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;

void UBODTGenApp::run() const
{
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();
    SPDLOG_INFO("Write UBODT to file {}", config_.result_file);
    UBODTGenAlgorithm model(network_, ng_);
    bool binary = config_.is_binary_output();
    std::string status = model.generate_ubodt(
        config_.result_file, config_.delta, binary, config_.use_omp);
    std::chrono::steady_clock::time_point end =
        std::chrono::steady_clock::now();
    double time_spent =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
            .count() /
        1000.;
    SPDLOG_INFO("Time takes {}", time_spent);
};
