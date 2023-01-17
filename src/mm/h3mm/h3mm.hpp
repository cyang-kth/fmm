#ifndef H3MM_HEADER
#define H3MM_HEADER

#include "util/debug.hpp"
#include "h3_type.hpp"
#include "h3_util.hpp"
#include "h3mm_writer.hpp"
#include "io/gps_reader.hpp"

namespace FMM
{
namespace MM
{

/**
 * Configuration class of stmatch command line program
 */
struct H3MMConfig
{
    H3MMConfig(int h3level_arg = 9, bool interpolate_arg = false)
    {
        h3level = h3level_arg;
        interpolate = interpolate_arg;
    };
    bool validate() const { return (h3level <= 15 && h3level >= 0); };
    std::string to_string() const
    {
        std::ostringstream oss;
        oss << "h3 level : " << h3level << "\n";
        oss << "interpolate : " << (interpolate ? "true" : "false") << "\n";
        return oss.str();
    };
    void print() const
    {
        SPDLOG_INFO("h3level: {} ", h3level);
        SPDLOG_INFO("interpolate: {} ", (interpolate ? "true" : "false"));
    };
    /**
     * Load from argument parsed data
     */
    static H3MMConfig load_from_arg(const cxxopts::ParseResult &arg_data)
    {
        int level = 0;
        bool interpolate = false;
        level = arg_data["h3level"].as<int>();
        if (arg_data.count("interpolate") > 0)
            interpolate = true;
        return H3MMConfig(level, interpolate);
    };
    /**
     * Register arguments to an option object
     */
    static void register_arg(cxxopts::Options &options)
    {
        options.add_options()("h3level", "Level of hex",
                              cxxopts::value<int>()->default_value("15"))(
            "interpolate", "Interpolate hex between points");
    };
    /**
     * Register help information to a string stream
     */
    static void register_help(std::ostringstream &oss)
    {
        oss << "--h3level (optional) <int>: level of hex (15)\n";
        oss << "--interpolate (optional): if specified, interpolate between "
               "points\n";
    };
    int h3level;
    bool interpolate;
};

class H3MM
{
  public:
    static H3MatchResult match_wkt(const std::string &wkt, int h3level,
                                   bool interpolate)
    {
        FMM::CORE::LineString line = FMM::CORE::wkt2linestring(wkt);
        std::vector<double> timestamps;
        FMM::CORE::Trajectory traj{0, line, timestamps};
        H3MMConfig config{h3level, interpolate};
        return match_traj(traj, config);
    };
    static H3MatchResult match_wkt(const std::string &wkt,
                                   const H3MMConfig &config)
    {
        FMM::CORE::LineString line = FMM::CORE::wkt2linestring(wkt);
        std::vector<double> timestamps;
        FMM::CORE::Trajectory traj{0, line, timestamps};
        return match_traj(traj, config);
    };

    static H3MatchResult match_traj(const FMM::CORE::Trajectory &traj,
                                    const H3MMConfig &config)
    {
        int NumberPoints = traj.geom.get_num_points();
        SPDLOG_DEBUG("Count of points in trajectory {}", NumberPoints);
        SPDLOG_DEBUG("Search candidates");
        std::vector<HexIndex> hexs;
        if (config.validate()) {
            if (config.interpolate) {
                SPDLOG_DEBUG("Interpolate hex");
                HexIndex prev_hex = 0;
                for (int i = 0; i < NumberPoints; ++i) {
                    SPDLOG_TRACE("Search candidates for point index {}", i);
                    // Construct a bounding boost_box
                    double px = traj.geom.get_x(i);
                    double py = traj.geom.get_y(i);
                    HexIndex hex = xy2hex(px, py, config.h3level);
                    SPDLOG_TRACE("Prev hex {} hex {}", prev_hex, hex);
                    if (i != 0 && hex != prev_hex) {
                        if (h3IndexesAreNeighbors(prev_hex, hex)) {
                            SPDLOG_TRACE("Prev hex and hex are neighbors");
                            hexs.push_back(hex);
                        } else {
                            auto seq_hex = hexpath(prev_hex, hex);
                            SPDLOG_TRACE(
                                "Hex between {} {} is with size {}: {}",
                                prev_hex, hex, seq_hex.size(), seq_hex);
                            int N = seq_hex.size();
                            if (N > 1) {
                                for (int j = 1; j < N; ++j) {
                                    hexs.push_back(seq_hex[j]);
                                }
                            } else {
                                hexs.push_back(hex);
                            }
                        }
                    } else {
                        if (i == 0)
                            hexs.push_back(hex);
                    }
                    prev_hex = hex;
                }
            } else {
                SPDLOG_DEBUG("No interpolate");
                HexIndex prev_hex = 0;
                for (int i = 0; i < NumberPoints; ++i) {
                    // SPDLOG_DEBUG("Search candidates for point index {}",i);
                    // Construct a bounding boost_box
                    double px = traj.geom.get_x(i);
                    double py = traj.geom.get_y(i);
                    HexIndex hex = xy2hex(px, py, config.h3level);
                    if (i == 0 || hex != prev_hex) {
                        hexs.push_back(hex);
                    }
                    prev_hex = hex;
                }
            }
        } else {
            SPDLOG_ERROR("Configuration invalid");
        }
        return H3MatchResult{traj.id, hexs};
    };
    static std::string
    match_gps_file(const FMM::CONFIG::GPSConfig &gps_config,
                   const FMM::MM::H3MMConfig &config,
                   const FMM::MM::H3MatchResultConfig &output_config,
                   bool use_omp = true)
    {
        std::ostringstream oss;
        std::string status;
        bool validate = true;
        if (!gps_config.validate()) {
            oss << "gps_config invalid\n";
            validate = false;
        }
        if (!config.validate()) {
            oss << "h3mm invalid\n";
            validate = false;
        }
        if (!validate) {
            oss << "match_gps_file canceled\n";
            return oss.str();
        }
        // Start map matching
        int progress = 0;
        int points_matched = 0;
        int total_points = 0;
        int step_size = 1000;
        UTIL::TimePoint begin_time = UTIL::get_current_time();
        FMM::IO::GPSReader reader(gps_config);
        H3MatchResultWriter writer(output_config);
        if (use_omp) {
            int buffer_trajectories_size = 100000;
            while (reader.has_next_trajectory()) {
                std::vector<FMM::CORE::Trajectory> trajectories =
                    reader.read_next_N_trajectories(buffer_trajectories_size);
                int trajectories_fetched = trajectories.size();
#pragma omp parallel for
                for (int i = 0; i < trajectories_fetched; ++i) {
                    FMM::CORE::Trajectory &trajectory = trajectories[i];
                    int points_in_tr = trajectory.geom.get_num_points();
                    auto result = match_traj(trajectory, config);
                    writer.write_result(trajectory, result);
#pragma omp critical
                    total_points += points_in_tr;
                    ++progress;
                    if (progress % step_size == 0) {
                        std::stringstream buf;
                        buf << "Progress " << progress << '\n';
                        std::cout << buf.rdbuf();
                    }
                }
            }
        } else {
            while (reader.has_next_trajectory()) {
                if (progress % step_size == 0) {
                    SPDLOG_INFO("Progress {}", progress);
                }
                FMM::CORE::Trajectory trajectory =
                    reader.read_next_trajectory();
                int points_in_tr = trajectory.geom.get_num_points();
                auto result = match_traj(trajectory, config);
                writer.write_result(trajectory, result);
                total_points += points_in_tr;
                ++progress;
            }
        }
        UTIL::TimePoint end_time = UTIL::get_current_time();
        double duration = UTIL::get_duration(begin_time, end_time);
        SPDLOG_INFO("Program finish in time {}", duration);
        oss << "Status: success\n";
        oss << "Time takes " << duration << " seconds\n";
        return oss.str();
    };
}; // H3MM
} // namespace MM
} // namespace FMM
#endif
