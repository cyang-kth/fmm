/**
 * Fast map matching.
 *
 * Definition of GPS reader classes 
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_GPS_READER_HPP
#define FMM_GPS_READER_HPP

#include "core/gps.hpp"
#include "config/gps_config.hpp"

#include <iostream>
#include <fstream>
#include <string>

namespace FMM {
/**
 * Classes related with input and output
 */
namespace IO {

/**
 * Trajectory Reader Interface.
 */
class ITrajectoryReader {
 public:
  /**
   * Read the next trajectory in the class
   * @return a trajectory
   */
  virtual FMM::CORE::Trajectory read_next_trajectory() = 0;
  /**
   * Check if the file contains a trajectory that is not read
   */
  virtual bool has_next_trajectory() = 0;
  /**
   * Check if the file contains timestamp information
   */
  virtual bool has_timestamp() = 0;
  /**
   * Close the file
   */
  virtual void close() = 0;
  /**
   * Read the next N trajectories in the file.
   *
   * The size of the vector can be smaller than N if there are fewer than
   * N trajectories in the file
   *
   * @param N the number of trajectories to read
   * @return a vector of trajectories.
   */
  std::vector<FMM::CORE::Trajectory> read_next_N_trajectories(int N = 1000);
  /**
   * Read all the remaining trajectories in a file
   * @return a vector of trajectories
   */
  std::vector<FMM::CORE::Trajectory> read_all_trajectories();
};

/**
 *  Trajectory Reader Class for Shapefile.
 *
 *  Each feauture in the file should store a linestring representing
 *  a trajectory.
 */
class GDALTrajectoryReader : public ITrajectoryReader {
 public:
  /**
   *  Constructor of GDALTrajectoryReader
   *  @param filename a GPS ESRI shapefile path
   *  @param id_name the ID field name
   *  @param timestamp_name the timestamp field name
   */
  GDALTrajectoryReader(const std::string &filename,
                       const std::string &id_name,
                       const std::string &timestamp_name);
  FMM::CORE::Trajectory read_next_trajectory() override;
  bool has_next_trajectory() override;
  bool has_timestamp() override;
  void close() override;
  /**
   * Get the number of trajectories in the file
   */
  int get_num_trajectories();
 private:
  int NUM_FEATURES = 0;
  int id_idx = -1;   // Index of the id column in shapefile
  int timestamp_idx = -1;   // Index of the id column in shapefile
  int _cursor = 0;   // Keep record of current features read
  GDALDataset *poDS;   // GDAL 2.1.0
  OGRLayer *ogrlayer;
}; // TrajectoryReader


/**
 * Trajectory Reader class for CSV trajectory file.
 * The file should contain a header with fields of id, geometry and
 * timestamp (optional). The delimiter is ;.
 * Each row in the trajectory file should store a linestring representing
 * a trajectory. The timestamp should be stored as a list of double values
 * delimited by ,. The number of timestamps should be the same as the points
 * in the trajectory. Timestamp is primarily used in STMATCH to limite the
 * search of shortest path queries. If it is not specified, it would be
 * estimated from the maximum speed.
 *
 * Example:
 *    id;geom;timestamp
 *    1;LineString(1 0,1 1);1,1
 */
class CSVTrajectoryReader : public ITrajectoryReader {
 public:
  /**
   * Constructor of CSVTrajectoryReader
   * @param e_filename input file name.
   * @param id_name ID column name
   * @param geom_name Geometry column name
   * @param timestamp_name Timestamp column name. If the timestamp column
   * is not found, an empty timestamp vector will be returned for
   * every trajectory.
   */
  CSVTrajectoryReader(const std::string &e_filename,
                      const std::string &id_name,
                      const std::string &geom_name,
                      const std::string &timestamp_name = "timestamp");
  /**
   * Reset cursor of the reader
   */
  void reset_cursor();
  /**
   * Read the next trajectory in the file.
   * @return A trajectory object
   */
  FMM::CORE::Trajectory read_next_trajectory() override;
  /**
   * Check if the file still contains trajectory not read
   * @return true if there is still any trajectory not read
   */
  bool has_next_trajectory() override;
  /**
   * Check if the file contains timestamp information
   * @return true if it contains timestamp
   */
  bool has_timestamp() override;
  /**
   * Close the reader object
   */
  void close() override;
  /**
   * Convert a string into a vector of timestamps
   * @param str input string, a list of double values separated by ,
   * @return a vector of timestamps
   */
  static std::vector<double> string2time(const std::string &str);
 private:
  std::fstream ifs;
  int id_idx = -1;
  int geom_idx = -1;
  int timestamp_idx = -1; // Index of the id column in shapefile
  char delim = ';';
}; // TrajectoryCSVReader

/**
 * Trajectory Reader class for CSV point file.
 *
 * The file should contain a header with fields delimited by ; and contain
 * fields of id x y and timestamp (optional). The order of column is not fixed.
 * Each row in the file represent a GPS point. The file must be sorted by the
 * id;timestamp in ascending order as trajectory will be extracted by
 * comparing id and timestamp information.
 *
 * Example:
 *    id;x;y;timestamp
 *    1;1;1;1
 *    1;1;2;2
 */
class CSVPointReader : public ITrajectoryReader {
 public:
  /**
   *  Reader class for CSV point data.
   * @param e_filename file name
   * @param id_name id column name
   * @param x_name x column name
   * @param y_name y column name
   * @param time_name timestamp name. If the timestamp column is not found,
   * an empty timestamp vector will be returned for every trajectory.
   */
  CSVPointReader(
      const std::string &e_filename,
      const std::string &id_name,
      const std::string &x_name,
      const std::string &y_name,
      const std::string &time_name);
  /**
   * Read the next trajectory in the file.
   * @return A trajectory object
   */
  FMM::CORE::Trajectory read_next_trajectory() override;
  /**
   * Check if the file still contains trajectory not read
   * @return true if there is still any trajectory not read
   */
  bool has_next_trajectory() override;
  /**
   * Reset cursor of the reader
   */
  void reset_cursor();
  /**
   * Check if the file contains timestamp information
   * @return true if it contains timestamp
   */
  bool has_timestamp() override;
  /**
   * Close the reader object
   */
  void close() override;
 private:
  std::string prev_line = "";
  std::fstream ifs;
  int id_idx = -1;
  int x_idx = -1;
  int y_idx = -1;
  int timestamp_idx = -1;
  char delim = ';';
}; // CSVTemporalTrajectoryReader

/**
 * %GPSReader class, a wrapper makes it easier to read data from
 * a file by specifying GPSConfig as input.
 */
class GPSReader {
 public:
  /**
   * Constructor
   * @param config configuration of GPS data, the file format will be
   * determined from it automatically.
   */
  GPSReader(const FMM::CONFIG::GPSConfig &config);
  /**
   * Read the next trajectory in the file.
   * @return A trajectory object
   */
  inline FMM::CORE::Trajectory read_next_trajectory() {
    return reader->read_next_trajectory();
  };
  /**
   * Check if the file still contains trajectory not read
   * @return true if there is still any trajectory not read
   */
  inline bool has_next_trajectory() {
    return reader->has_next_trajectory();
  };
  /**
   * Read next N trajectories from the file. If there are k trajectories left
   * k<N, then only k trajectories will be returned.
   *
   * @param N number of trajectories to read
   * @return A vector of k trajectories.
   */
  inline std::vector<FMM::CORE::Trajectory> read_next_N_trajectories(int N) {
    return reader->read_next_N_trajectories(N);
  };
  /**
   * Return all remaining trajectories from the file
   * @return a vector of trajectories.
   */
  inline std::vector<FMM::CORE::Trajectory> read_all_trajectories() {
    return reader->read_all_trajectories();
  };
 private:
  std::shared_ptr<ITrajectoryReader> reader;
  int mode; /**< Mode marking the type of GPS data stored in the file */
};

} // IO
} // FMM

#endif // FMM_READER_HPP
