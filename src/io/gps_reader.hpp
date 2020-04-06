/**
 * Content
 * Definition of a TrajectoryReader which is a wrapper of
 * the standard shapefile reader in GDAL.
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

namespace FMM
{
/**
 * Classes related with input and output
 */
namespace IO
{
using Trajectory = FMM::CORE::Trajectory;
/**
 * Trajectory Reader Interface.
 */
class ITrajectoryReader {
public:
  /**
   * Read the next trajectory in the class
   * @return a trajectory
   */
  virtual Trajectory read_next_trajectory() = 0;
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
  std::vector<Trajectory> read_next_N_trajectories(int N=1000);
  /**
   * Read all the remaining trajectories in a file
   * @return a vector of trajectories
   */
  std::vector<Trajectory> read_all_trajectories();
};

/**
 *  Trajectory Reader Class for Shapefile.
 *
 *  Each feauture in the file should store a linestring representing
 *  a trajectory.
 */
class GDALTrajectoryReader : public ITrajectoryReader
{
public:
  /**
   *  Constructor of GDALTrajectoryReader
   *  @param filename a GPS ESRI shapefile path
   *  @param id_name the ID field name
   *  @param timestamp_name the timestamp field name
   */
  GDALTrajectoryReader(const std::string & filename,
                       const std::string & id_name,
                       const std::string & timestamp_name);
  Trajectory read_next_trajectory() override;
  bool has_next_trajectory() override;
  bool has_timestamp() override;
  void close() override;
  /**
   * Get the number of trajectories in the file
   */
  int get_num_trajectories();
private:
  int NUM_FEATURES=0;
  int id_idx = -1;   // Index of the id column in shapefile
  int timestamp_idx = -1;   // Index of the id column in shapefile
  int _cursor=0;   // Keep record of current features read
  GDALDataset *poDS;   // GDAL 2.1.0
  OGRLayer  *ogrlayer;
}; // TrajectoryReader


/**
 * Trajectory Reader class for CSV trajectory file.
 *
 * Each row in the trajectory file should store a linestring representing
 * a trajectory.
 */
class CSVTrajectoryReader : public ITrajectoryReader {
public:
  CSVTrajectoryReader(const std::string &e_filename,
                      const std::string &id_name,
                      const std::string &geom_name,
                      const std::string &timestamp_name="timestamp");
  void reset_cursor();
  Trajectory read_next_trajectory() override;
  bool has_next_trajectory() override;
  bool has_timestamp() override;
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
  int timestamp_idx =-1;   // Index of the id column in shapefile
  char delim = ';';
}; // TrajectoryCSVReader

/**
 * Trajectory Reader class for CSV point file.
 *
 * Each row in the file represent a GPS point with id;x;y;timestamp
 */
class CSVPointReader : public ITrajectoryReader {
 public:
  /**
   *
   * @param e_filename file name
   * @param id_name id column name
   * @param x_name x column name
   * @param y_name y column name
   * @param time_name timestamp name
   */
  CSVPointReader(
       const std::string &e_filename,
       const std::string &id_name,
       const std::string &x_name,
       const std::string &y_name,
       const std::string &time_name);
  Trajectory read_next_trajectory() override;
  bool has_next_trajectory() override;
  void reset_cursor();
  bool has_timestamp() override;
  void close() override;
 private:
  std::string prev_line="";
  std::fstream ifs;
  int id_idx = -1;
  int x_idx = -1;
  int y_idx = -1;
  int timestamp_idx = -1;
  char delim = ';';
}; // CSVTemporalTrajectoryReader

/**
 * %GPSReader class, a wrapper makes it easier to read data from
 * a file.
 */
class GPSReader {
 public:
  /**
   * Constructor
   * @param config configuration of GPS data, the file format will be
   * determined from it automatically.
   */
  GPSReader(const FMM::CONFIG::GPSConfig &config);
  inline Trajectory read_next_trajectory(){
    return reader->read_next_trajectory();
  };
  inline bool has_next_trajectory(){
    return reader->has_next_trajectory();
  };
  inline std::vector<Trajectory> read_next_N_trajectories(int N){
    return reader->read_next_N_trajectories(N);
  };
  inline std::vector<Trajectory> read_all_trajectories(){
    return reader->read_all_trajectories();
  };

 private:
  std::shared_ptr<ITrajectoryReader> reader;
  // 0 for GDAL, 1 for CSV, -1 for unknown
  int mode;
};

} // IO
} // FMM

#endif // FMM_READER_HPP
