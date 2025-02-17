/*
* This file is part of liodom.
*
* Copyright (C) 2020 Emilio Garcia-Fidalgo <emilio.garcia@uib.es> (University of the Balearic Islands)
*
* liodom is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* liodom is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with liodom. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INCLUDE_LIODOM_LASER_ODOMETRY_H
#define INCLUDE_LIODOM_LASER_ODOMETRY_H

#include <numeric>
#include <thread>

// Ceres
#include <ceres/ceres.h>

// Eigen
#include <Eigen/Dense>
#include <Eigen/Geometry>

// ROS
#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/TwistStamped.h>

// PCL
#include <pcl/common/transforms.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/filter.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/impl/kdtree_flann.hpp>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/search/search.h>

// Liodom
#include <liodom/defs.h>
#include <liodom/factors.hpp>
#include <liodom/params.h>
#include <liodom/shared_data.h>
#include <liodom/stats.h>

namespace liodom {

// Local Map manager
class LocalMapManager {
 public:
  explicit LocalMapManager(const size_t max_frames);
  virtual ~LocalMapManager();

  void addPointCloud(const PointCloud::Ptr& pc);
  size_t getLocalMap(PointCloud::Ptr& map);
  void setMaxFrames(const size_t max_nframes);

 private:
  PointCloud::Ptr total_points_;
  size_t nframes_;
  size_t max_nframes_;
  std::queue<size_t> sizes_;  
};

// Laser Odometry
class LaserOdometer {
 public:
  explicit LaserOdometer(const ros::NodeHandle& nh);
  virtual ~LaserOdometer();

  void operator()(std::atomic<bool>& running);

 private:
  // ROS variables
  ros::NodeHandle nh_;
  ros::Publisher pc_edges_pub_;
  ros::Publisher odom_pub_;
  ros::Publisher twist_pub_;
  tf::TransformBroadcaster tf_broadcaster_;  

  // Variables
  bool init_;
  Eigen::Isometry3d prev_odom_;
  Eigen::Isometry3d odom_;
  double prev_stamp_;  
  double param_q[4] = {0, 0, 0, 1};
  double param_t[3] = {0, 0, 0};
  SharedData* sdata;
  Stats* stats;
  Params* params;
  LocalMapManager lmap_manager;
  Eigen::Isometry3d laser_to_base_;

  void computeLocalMap(PointCloud::Ptr& local_map_gen, PointCloud::Ptr& local_map_rec);
  void addEdgeConstraints(const PointCloud::Ptr& edges,
                          const PointCloud::Ptr& local_map_gen,
                          const PointCloud::Ptr& local_map_rec,
                          const Eigen::Isometry3d& pose,
                          ceres::Problem* problem,
                          ceres::LossFunction* loss);
  bool getBaseToLaserTf(const std::string& frame_id);
};

}  // namespace liodom

#endif // INCLUDE_LIODOM_LASER_ODOMETRY_H
