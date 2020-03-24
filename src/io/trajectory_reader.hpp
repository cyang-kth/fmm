/**
 * Content
 * Definition of a TrajectoryReader which is a wrapper of
 * the standard shapefile reader in GDAL.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_TRAJECTORY_READER_HPP
#define MM_TRAJECTORY_READER_HPP

#include "core/gps.hpp"

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
  virtual bool has_next_feature() = 0;
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
                       const std::string & id_name);
  Trajectory read_next_trajectory() override;
  bool has_next_feature() override;
  void close() override;
  int get_num_trajectories();
private:
  int NUM_FEATURES=0;
  int id_idx;   // Index of the id column in shapefile
  int _cursor=0;   // Keep record of current features read
  GDALDataset *poDS;   // GDAL 2.1.0
  OGRLayer  *ogrlayer;
}; // TrajectoryReader


class CSVTrajectoryReader : public ITrajectoryReader {
public:
  CSVTrajectoryReader(const std::string &e_filename,
                      const std::string &id_name,
                      const std::string &geom_name);
  void reset_cursor();
  Trajectory read_next_trajectory() override;
  bool has_next_feature() override;
  void close() override;
private:
  std::fstream ifs;
  int id_idx = -1;
  int geom_idx = -1;
  char delim = ';';
}; // TrajectoryCSVReader

class TemporalGPSReader{
public:
  virtual TemporalTrajectory read_next_temporal_trajectory()=0;
  virtual Trajectory read_next_trajectory()=0;
  virtual bool has_time_stamp() const = 0;
  virtual bool has_next_feature()=0;
  virtual void close() = 0;
};

class CSVTemporalTrajectoryReader: public TemporalGPSReader {
public:
  CSVTemporalTrajectoryReader(const std::string &e_filename,
                              const std::string &id_name,
                              const std::string &geom_name,
                              const std::string &time_name);
  TemporalTrajectory read_next_temporal_trajectory() override;
  Trajectory read_next_trajectory() override;
  static std::vector<double> string2time(const std::string &str);
  bool has_next_feature() override;
  void reset_cursor();
  void close() override;
  bool has_time_stamp() const override;
private:
  std::fstream ifs;
  int id_idx = -1;
  int geom_idx = -1;
  int time_idx = -1;
  char delim = ';';
}; // CSVTemporalTrajectoryReader

class CSVTemporalPointReader : public TemporalGPSReader {
 public:
  CSVTemporalPointReader(
       const std::string &e_filename,
       const std::string &id_name,
       const std::string &x_name,
       const std::string &y_name,
       const std::string &time_name);
  TemporalTrajectory read_next_temporal_trajectory() override;
  Trajectory read_next_trajectory() override;
  bool has_next_feature() override;
  void reset_cursor();
  void close() override;
  bool has_time_stamp() const override;
 private:
  std::string prev_line="";
  std::fstream ifs;
  int id_idx = -1;
  int x_idx = -1;
  int y_idx = -1;
  int time_idx = -1;
  char delim = ';';
}; // CSVTemporalTrajectoryReader


} // IO
} // MM

#endif // MM_READER_HPP
