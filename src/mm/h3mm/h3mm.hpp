// typedef boost::geometry::polygon Polygon;
#include <h3api.h>
namespace FMM {
namespace MM {

struct H3MMResult {
  int traj_id;
  std::vector<H3Index> hexs;
};

/**
 * Configuration class of stmatch command line program
 */
struct H3MMConfig {
  H3MMConfig(int h3level_arg, bool interpolate);
  int h3level;
  bool interpolate;
  bool validate() const {
    return (h3level<=15 && h3level>=0);
  };
  void print() const {
    SPDLOG_INFO("h3level: {} ",h3level);
    SPDLOG_INFO("interpolate: {} ",(interpolate?"true":"false"));
  };
  /**
   * Load from argument parsed data
   */
  static H3MMConfig load_from_arg(
    const cxxopts::ParseResult &arg_data){
    int level = 0;
    bool interpolate = false;
    level = arg_data["h3level"].as<int>();
    if (arg_data.count("interpolate")>0)
      interpolate = true;
    return H3MMConfig(level,interpolate);
  };
  /**
   * Register arguments to an option object
   */
  static void register_arg(cxxopts::Options &options){
    options.add_options()
      ("h3level","Level of hex",
      cxxopts::value<int>()->default_value("15"))
      ("interpolate","Interpolate hex between points");
  };
  /**
   * Register help information to a string stream
   */
  static void register_help(std::ostringstream &oss){
    oss<<"--h3level (optional) <int>: level of hex (15)\n";
    oss<<"--interpolate (optional): if specified, interpolate between points\n";
  };
};


class H3MM {
public:
  H3MMResult match_wkt(
    const std::string &wkt,const H3MMConfig &config){
    LineString line = wkt2linestring(wkt);
    std::vector<double> timestamps;
    Trajectory traj{0, line, timestamps};
    return match_traj(traj,config);
  };
  static std::string hexs2wkt(const std::vector<H3Index> &hexs){
    oss << "MULTIPOLYGON(";
    for (auto index:hexs){
      GeoBoundary boundary;
      oss<<"(";
      h3ToGeoBoundary(indexed, &boundary);
      for (int v = 0; v < boundary.numVerts; v++) {
        oss << boundary.verts[v].lat << " " <<
          boundary.verts[v].lon;
      }
      oss<<")";
    }
    oss << ")";
  };
  static std::string hex2wkt(H3Index &index){
    std::ostringstream oss;
    GeoBoundary boundary;
    oss << "POLYGON(";
    h3ToGeoBoundary(indexed, &boundary);
    for (int v = 0; v < boundary.numVerts; v++) {
      oss << boundary.verts[v].lat << " " <<
        boundary.verts[v].lon;
    }
    oss << ")";
  };
  static H3Index xy2hex(double x, double y){
    double px = geom.get_x(i);
    double py = geom.get_y(i);
    GeoCoord location;
    setGeoDegs(&location,py,px);
    H3Index indexed = geoToH3(&location, config.h3level);
    return indexed;
  };
  H3MMResult match_traj(const CORE::Trajectory &traj,
                        const H3MMConfig &config){
    int NumberPoints = traj.geom.get_num_points();
    SPDLOG_DEBUG("Count of points in trajectory {}", );
    SPDLOG_DEBUG("Search candidates");
    std::vector<H3Index> hexs;
    // MultiPolygon
    int prev_hex = -1;
    for (int i = 0; i < NumberPoints; ++i) {
      // SPDLOG_DEBUG("Search candidates for point index {}",i);
      // Construct a bounding boost_box
      double px = geom.get_x(i);
      double py = geom.get_y(i);
      GeoCoord location;
      setGeoDegs(&location,py,px);
      H3Index indexed = geoToH3(&location, config.h3level);
      hexs.push_back(indexed);
    }
    return H3MatchResult{
      traj.id, hexs};
  };

  std::string match_gps_file(
    const FMM::CONFIG::GPSConfig &gps_config,
    const H3MMConfig &config,
    const H3MatchResultConfig &output_config,
    bool use_omp = true){
    std::ostringstream oss;
    std::string status;
    bool validate = true;
    if (!gps_config.validate()) {
      oss<<"gps_config invalid\n";
      validate = false;
    }
    if (!config.validate()) {
      oss<<"h3mm invalid\n";
      validate = false;
    }
    if (!validate) {
      oss<<"match_gps_file canceled\n";
      return oss.str();
    }
    // Start map matching
    int progress = 0;
    int points_matched = 0;
    int total_points = 0;
    int step_size = 1000;
    UTIL::TimePoint begin_time = std::chrono::steady_clock::now();
    FMM::IO::GPSReader reader(gps_config);
    H3MMResultWriter writer(output_config);
    if (use_omp) {
      int buffer_trajectories_size = 100000;
      while (reader.has_next_trajectory()) {
        std::vector<Trajectory> trajectories =
          reader.read_next_N_trajectories(buffer_trajectories_size);
        int trajectories_fetched = trajectories.size();
        #pragma omp parallel for
        for (int i = 0; i < trajectories_fetched; ++i) {
          Trajectory &trajectory = trajectories[i];
          int points_in_tr = trajectory.geom.get_num_points();
          auto result = match_traj(
            trajectory, config);
          writer.write_result(trajectory,result);
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
        Trajectory trajectory = reader.read_next_trajectory();
        int points_in_tr = trajectory.geom.get_num_points();
        auto result = match_traj(
          trajectory, config);
        writer.write_result(trajectory,result);
        total_points += points_in_tr;
        ++progress;
      }
    }
    UTIL::TimePoint end_time = std::chrono::steady_clock::now();
    double duration = std::chrono::duration_cast<
      std::chrono::milliseconds>(end_time - begin_time).count() / 1000.;
    oss<<"Status: success\n";
    oss<<"Time takes " << duration << " seconds\n";
    return oss.str();
  };
};     // H3MM
}
}
