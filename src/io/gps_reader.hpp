/**
 * Content
 * Definition of a TrajectoryReader which is a wrapper of
 * the standard shapefile reader in GDAL.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_GPS_READER_HPP
#define MM_GPS_READER_HPP

#include "core/gps.hpp"
#include "config/gps_config.hpp"

#include <iostream>
#include <fstream>
#include <string>

namespace MM
{
namespace IO
{
/**
 *  According to the documentation at http://gdal.org/1.11/ogr/ogr_apitut.html
 *
 *  Note that OGRFeature::GetGeometryRef() and OGRFeature::GetGeomFieldRef()
 *  return a pointer to the internal geometry owned by the OGRFeature.
 *  We don't actually need to delete the return geometry. However, the
 *  OGRLayer::GetNextFeature() method returns a copy of the feature that is
 *  now owned by us. So at the end of use we must free the feature.
 *
 *  It implies that when we delete the feature, the geometry returned by
 *  OGRFeature::GetGeometryRef() is also deleted. Therefore, we need to
 *  create a copy of the geometry and free it with
 *      OGRGeometryFactory::destroyGeometry(geometry_pointer);
 *
 */

class ITrajectoryReader {
public:
  virtual Trajectory read_next_trajectory() = 0;
  virtual bool has_next_trajectory() = 0;
  virtual bool has_timestamp() = 0;
  virtual void close() = 0;
  std::vector<Trajectory> read_next_N_trajectories(int N=1000);
  std::vector<Trajectory> read_all_trajectories();
};

class GDALTrajectoryReader : public ITrajectoryReader
{
public:
  /**
   *  Constructor of TrajectoryReader
   *  @param filename, a GPS ESRI shapefile path
   *  @param id_name, the ID column name in the GPS shapefile
   */
  GDALTrajectoryReader(const std::string & filename,
                       const std::string & id_name,
                       const std::string & timestamp_name);
  Trajectory read_next_trajectory() override;
  bool has_next_trajectory() override;
  bool has_timestamp() override;
  void close() override;
  int get_num_trajectories();
private:
  int NUM_FEATURES=0;
  int id_idx = -1;   // Index of the id column in shapefile
  int timestamp_idx = -1;   // Index of the id column in shapefile
  int _cursor=0;   // Keep record of current features read
  GDALDataset *poDS;   // GDAL 2.1.0
  OGRLayer  *ogrlayer;
}; // TrajectoryReader


class CSVTrajectoryReader : public ITrajectoryReader {
public:
  CSVTrajectoryReader(const std::string &e_filename,
                      const std::string &id_name,
                      const std::string &geom_name,
                      const std::string &timestamp_name);
  void reset_cursor();
  Trajectory read_next_trajectory() override;
  bool has_next_trajectory() override;
  bool has_timestamp() override;
  void close() override;
  static std::vector<double> string2time(const std::string &str);
 private:
  std::fstream ifs;
  int id_idx = -1;
  int geom_idx = -1;
  int timestamp_idx =-1;   // Index of the id column in shapefile
  char delim = ';';
}; // TrajectoryCSVReader

class CSVPointReader : public ITrajectoryReader {
 public:
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

class GPSReader {
 public:
  GPSReader(const GPSConfig &config);
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
} // MM

#endif // MM_READER_HPP
