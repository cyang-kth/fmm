/**
 *  Implementation of GPS reader classes
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */
#include "io/gps_reader.hpp"
#include "util/debug.hpp"
#include "config/gps_config.hpp"
#include <iostream>
#include <string>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::IO;

std::vector<Trajectory> ITrajectoryReader::read_next_N_trajectories(int N) {
  std::vector<Trajectory> trajectories;
  int i = 0;
  while (i < N && has_next_trajectory()) {
    trajectories.push_back(read_next_trajectory());
    ++i;
  }
  return trajectories;
}

std::vector<Trajectory> ITrajectoryReader::read_all_trajectories() {
  std::vector<Trajectory> trajectories;
  int i = 0;
  while (has_next_trajectory()) {
    trajectories.push_back(read_next_trajectory());
    ++i;
  }
  return trajectories;
}

GDALTrajectoryReader::GDALTrajectoryReader(const std::string &filename,
                                           const std::string &id_name,
                                           const std::string &timestamp_name) {
  SPDLOG_INFO("Read trajectory from file {}",filename);
  OGRRegisterAll();
  poDS = (GDALDataset *) GDALOpenEx(filename.c_str(),
                                    GDAL_OF_VECTOR, NULL, NULL, NULL);
  if (poDS == NULL) {
    SPDLOG_CRITICAL("Open data source fail");
    exit(1);
  }
  ogrlayer = poDS->GetLayer(0);
  _cursor = 0;
  // Get the number of features first
  OGRFeatureDefn *ogrFDefn = ogrlayer->GetLayerDefn();
  NUM_FEATURES = ogrlayer->GetFeatureCount();
  // This should be a local field rather than a new variable
  id_idx = ogrFDefn->GetFieldIndex(id_name.c_str());
  if (id_idx < 0) {
    SPDLOG_CRITICAL("Id column {} not found", id_name);
    GDALClose(poDS);
    std::exit(EXIT_FAILURE);
  }
  timestamp_idx = ogrFDefn->GetFieldIndex(timestamp_name.c_str());
  if (timestamp_idx < 0) {
    SPDLOG_WARN("Timestamp column {} not found", timestamp_name);
  }
  if (wkbFlatten(ogrFDefn->GetGeomType()) != wkbLineString) {
    SPDLOG_CRITICAL("Geometry type is {}, which should be linestring",
                    OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
    GDALClose(poDS);
    std::exit(EXIT_FAILURE);
  } else {
    SPDLOG_DEBUG("Geometry type is {}",
                OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
  }
  SPDLOG_INFO("Total number of trajectories {}", NUM_FEATURES);
  SPDLOG_INFO("Finish reading meta data");
}

bool GDALTrajectoryReader::has_next_trajectory() {
  return _cursor < NUM_FEATURES;
}

bool GDALTrajectoryReader::has_timestamp() {
  return timestamp_idx > 0;
}

Trajectory GDALTrajectoryReader::read_next_trajectory() {
  OGRFeature *ogrFeature = ogrlayer->GetNextFeature();
  int trid = ogrFeature->GetFieldAsInteger(id_idx);
  OGRGeometry *rawgeometry = ogrFeature->GetGeometryRef();
  FMM::CORE::LineString linestring =
      FMM::CORE::ogr2linestring((OGRLineString *) rawgeometry);
  OGRFeature::DestroyFeature(ogrFeature);
  ++_cursor;
  return Trajectory{trid, linestring};
}

int GDALTrajectoryReader::get_num_trajectories() {
  return NUM_FEATURES;
}

void GDALTrajectoryReader::close() {
  GDALClose(poDS);
}

CSVTrajectoryReader::CSVTrajectoryReader(const std::string &e_filename,
                                         const std::string &id_name,
                                         const std::string &geom_name,
                                         const std::string &timestamp_name) :
    ifs(e_filename) {
  std::string line;
  std::getline(ifs, line);
  std::stringstream check1(line);
  std::string intermediate;
  // Tokenizing w.r.t. space ' '
  int i = 0;
  while (getline(check1, intermediate, delim)) {
    if (intermediate == id_name) {
      id_idx = i;
    }
    if (intermediate == geom_name) {
      geom_idx = i;
    }
    if (intermediate == timestamp_name) {
      timestamp_idx = i;
    }
    ++i;
  }
  if (id_idx < 0 || geom_idx < 0) {
    SPDLOG_CRITICAL("Id {} or Geometry column {} not found",
                    id_name, geom_name);
    std::exit(EXIT_FAILURE);
  }
  if (timestamp_idx < 0) {
    SPDLOG_WARN("Timestamp column {} not found", timestamp_name);
  }
  SPDLOG_INFO("Id index {} Geometry index {} Timstamp index {}",
              id_idx, geom_idx, timestamp_idx);
}

std::vector<double> CSVTrajectoryReader::string2time(
    const std::string &str) {
  std::vector<double> values;
  std::stringstream ss(str);
  double v;
  while (ss >> v) {
    values.push_back(v);
    if (ss.peek() == ',')
      ss.ignore();
  }
  return values;
}

bool CSVTrajectoryReader::has_timestamp() {
  return timestamp_idx > 0;
}

Trajectory CSVTrajectoryReader::read_next_trajectory() {
  // Read the geom idx column into a trajectory
  std::string line;
  std::getline(ifs, line);
  std::stringstream ss(line);
  int trid = 0;
  int index = 0;
  std::string intermediate;
  FMM::CORE::LineString geom;
  std::vector<double> timestamps;
  while (std::getline(ss, intermediate, delim)) {
    if (index == id_idx) {
      trid = std::stoi(intermediate);
    }
    if (index == geom_idx) {
      // intermediate
      boost::geometry::read_wkt(intermediate, geom.get_geometry());
    }
    if (index == timestamp_idx) {
      // intermediate
      timestamps = string2time(intermediate);
    }
    ++index;
  }
  return Trajectory{trid, geom, timestamps};
}

bool CSVTrajectoryReader::has_next_trajectory() {
  return ifs.peek() != EOF;
}

void CSVTrajectoryReader::reset_cursor() {
  ifs.clear();
  ifs.seekg(0, std::ios::beg);
  std::string line;
  std::getline(ifs, line);
}
void CSVTrajectoryReader::close() {
  ifs.close();
}

CSVPointReader::CSVPointReader(const std::string &e_filename,
                               const std::string &id_name,
                               const std::string &x_name,
                               const std::string &y_name,
                               const std::string &time_name) :
    ifs(e_filename) {
  std::string line;
  std::getline(ifs, line);
  std::stringstream check1(line);
  std::string intermediate;
  // Tokenizing w.r.t. space ' '
  int i = 0;
  while (getline(check1, intermediate, delim)) {
    if (intermediate == id_name) {
      id_idx = i;
    }
    if (intermediate == x_name) {
      x_idx = i;
    }
    if (intermediate == y_name) {
      y_idx = i;
    }
    if (intermediate == time_name) {
      timestamp_idx = i;
    }
    ++i;
  }
  if (id_idx < 0 || x_idx < 0 || y_idx < 0) {
    if (id_idx < 0) {
      SPDLOG_CRITICAL("Id column {} not found", id_name);
    }
    if (x_idx < 0) {
      SPDLOG_CRITICAL("Geom column {} not found", x_name);
    }
    if (y_idx < 0) {
      SPDLOG_CRITICAL("Geom column {} not found", y_name);
    }
    std::exit(EXIT_FAILURE);
  }
  if (timestamp_idx < 0) {
    SPDLOG_WARN("Time stamp {} not found, will be estimated ", time_name);
  }
  SPDLOG_INFO("Id index {} x index {} y index {} time index {}",
              id_idx, x_idx, y_idx, timestamp_idx);
}

Trajectory CSVPointReader::read_next_trajectory() {
  // Read the geom idx column into a trajectory
  std::string intermediate;
  FMM::CORE::LineString geom;
  std::vector<double> timestamps;
  bool on_same_trajectory = true;
  bool first_observation = true;
  int trid = -1;
  int prev_id = -1;
  double prev_timestamp = -1.0;
  std::string line;
  while (on_same_trajectory && has_next_trajectory()) {
    if (prev_line.empty()) {
      std::getline(ifs, line);
    } else {
      line = prev_line;
      prev_line.clear();
    }
    std::stringstream ss(line);
    int id = 0;
    double x = 0, y = 0;
    double timestamp = 0;
    int index = 0;
    while (std::getline(ss, intermediate, delim)) {
      if (index == id_idx) {
        id = std::stoi(intermediate);
      }
      if (index == x_idx) {
        x = std::stof(intermediate);
      }
      if (index == y_idx) {
        y = std::stof(intermediate);
      }
      if (index == timestamp_idx) {
        timestamp = std::stof(intermediate);
      }
      ++index;
    }
    if (prev_id == id || first_observation) {
      geom.add_point(x, y);
      if (has_timestamp())
        timestamps.push_back(timestamp);
    }
    if (prev_id != id && !first_observation) {
      on_same_trajectory = false;
      trid = prev_id;
    }
    first_observation = false;
    prev_id = id;
    if (!on_same_trajectory) {
      prev_line = line;
    }
  }
  if (!has_next_trajectory()){
    trid = prev_id;
  }
  return Trajectory{trid, geom, timestamps};
}

bool CSVPointReader::has_next_trajectory() {
  return ifs.peek() != EOF;
}

void CSVPointReader::reset_cursor() {
  ifs.clear();
  ifs.seekg(0, std::ios::beg);
  std::string line;
  std::getline(ifs, line);
}

void CSVPointReader::close() {
  ifs.close();
}

bool CSVPointReader::has_timestamp() {
  return timestamp_idx > 0;
}

GPSReader::GPSReader(const FMM::CONFIG::GPSConfig &config) {
  mode = config.get_gps_format();
  if (mode == 0) {
    reader = std::make_shared<GDALTrajectoryReader>
        (config.file, config.id,config.timestamp);
  } else if (mode == 1) {
    reader = std::make_shared<CSVTrajectoryReader>
        (config.file, config.id, config.geom, config.timestamp);
  } else if (mode == 2) {
    reader = std::make_shared<CSVPointReader>
        (config.file, config.id, config.x, config.y, config.timestamp);
  } else {
    SPDLOG_CRITICAL("Unrecognized GPS format");
    std::exit(EXIT_FAILURE);
  }
};

