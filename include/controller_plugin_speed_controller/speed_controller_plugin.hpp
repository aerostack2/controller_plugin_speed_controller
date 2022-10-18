/*!*******************************************************************************************
 *  \file       speed_controller_plugin.hpp
 *  \brief      Speed PID controller plugin for the Aerostack framework.
 *  \authors    Miguel Fernández Cortizas
 *              Pedro Arias Pérez
 *              David Pérez Saura
 *              Rafael Pérez Seguí
 *
 *  \copyright  Copyright (c) 2022 Universidad Politécnica de Madrid
 *              All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

#ifndef __DF_PLUGIN_H__
#define __DF_PLUGIN_H__

#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>
#include <vector>

#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <as2_core/utils/frame_utils.hpp>
#include <as2_core/utils/tf_utils.hpp>
#include <as2_msgs/msg/thrust.hpp>
#include <controller_plugin_base/controller_base.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <trajectory_msgs/msg/joint_trajectory_point.hpp>

#include <pid_controller/PID.hpp>
#include <pid_controller/PID_3D.hpp>

namespace controller_plugin_speed_controller {

struct UAV_state {
  std_msgs::msg::Header position_header = std_msgs::msg::Header();
  Eigen::Vector3d position              = Eigen::Vector3d::Zero();
  std_msgs::msg::Header velocity_header = std_msgs::msg::Header();
  Eigen::Vector3d velocity              = Eigen::Vector3d::Zero();
  Eigen::Vector3d yaw                   = Eigen::Vector3d::Zero();
};

struct UAV_command {
  std_msgs::msg::Header velocity_header = std_msgs::msg::Header();
  Eigen::Vector3d velocity              = Eigen::Vector3d::Zero();
  double yaw_speed                      = 0.0;
};

struct Control_flags {
  bool state_received;
  bool ref_received;
  bool plugin_parameters_read;
  bool position_controller_parameters_read;
  bool velocity_controller_parameters_read;
  bool trajectory_controller_parameters_read;
  bool yaw_controller_parameters_read;
};

class Plugin : public controller_plugin_base::ControllerBase {
public:
  Plugin(){};
  ~Plugin(){};

public:
  void ownInitialize() override;
  void updateState(const geometry_msgs::msg::PoseStamped &pose_msg,
                   const geometry_msgs::msg::TwistStamped &twist_msg) override;

  void updateReference(const geometry_msgs::msg::PoseStamped &ref) override;
  void updateReference(const geometry_msgs::msg::TwistStamped &ref) override;
  void updateReference(const trajectory_msgs::msg::JointTrajectoryPoint &ref) override;

  bool setMode(const as2_msgs::msg::ControlMode &mode_in,
               const as2_msgs::msg::ControlMode &mode_out) override;

  void computeOutput(geometry_msgs::msg::PoseStamped &pose,
                     geometry_msgs::msg::TwistStamped &twist,
                     as2_msgs::msg::Thrust &thrust) override;

  bool updateParams(const std::vector<std::string> &_params_list) override;
  void reset() override;

  rcl_interfaces::msg::SetParametersResult parametersCallback(
      const std::vector<rclcpp::Parameter> &parameters);

private:
  rclcpp::Time last_time_;

  as2_msgs::msg::ControlMode control_mode_in_;
  as2_msgs::msg::ControlMode control_mode_out_;

  Control_flags flags_;

  std::shared_ptr<pid_controller::PIDController> pid_yaw_handler_;
  std::shared_ptr<pid_controller::PIDController3D> pid_3D_position_handler_;
  std::shared_ptr<pid_controller::PIDController3D> pid_3D_velocity_handler_;
  std::shared_ptr<pid_controller::PIDController3D> pid_3D_trajectory_handler_;

  std::shared_ptr<as2::tf::TfHandler> tf_handler_;

  std::vector<std::string> plugin_parameters_list_ = {
      "proportional_limitation",
  };

  std::vector<std::string> position_control_parameters_list_ = {
      "position_control.reset_integral", "position_control.antiwindup_cte",
      "position_control.alpha",          "position_control.kp.x",
      "position_control.kp.y",           "position_control.kp.z",
      "position_control.ki.x",           "position_control.ki.y",
      "position_control.ki.z",           "position_control.kd.x",
      "position_control.kd.y",           "position_control.kd.z"};

  std::vector<std::string> velocity_control_parameters_list_ = {
      "speed_control.reset_integral", "speed_control.antiwindup_cte", "speed_control.alpha",
      "speed_control.kp.x",           "speed_control.kp.y",           "speed_control.kp.z",
      "speed_control.ki.x",           "speed_control.ki.y",           "speed_control.ki.z",
      "speed_control.kd.x",           "speed_control.kd.y",           "speed_control.kd.z"};

  std::vector<std::string> trajectory_control_parameters_list_ = {
      "trajectory_control.reset_integral", "trajectory_control.antiwindup_cte",
      "trajectory_control.alpha",          "trajectory_control.kp.x",
      "trajectory_control.kp.y",           "trajectory_control.kp.z",
      "trajectory_control.ki.x",           "trajectory_control.ki.y",
      "trajectory_control.ki.z",           "trajectory_control.kd.x",
      "trajectory_control.kd.y",           "trajectory_control.kd.z"};

  std::vector<std::string> yaw_control_parameters_list_ = {"yaw_control.reset_integral",
                                                           "yaw_control.antiwindup_cte",
                                                           "yaw_control.alpha",
                                                           "yaw_control.kp",
                                                           "yaw_control.ki",
                                                           "yaw_control.kd"};

  std::vector<std::string> plugin_parameters_to_read_;
  std::vector<std::string> position_control_parameters_to_read_;
  std::vector<std::string> velocity_control_parameters_to_read_;
  std::vector<std::string> trajectory_control_parameters_to_read_;
  std::vector<std::string> yaw_control_parameters_to_read_;

  UAV_state uav_state_;
  UAV_state control_ref_;
  UAV_command control_command_;
  Eigen::Vector3d command_speed_;
  double command_yaw_speed_;

  Eigen::Vector3d speed_limits_;
  double yaw_speed_limit_;

  bool proportional_limitation_ = false;
  std::string enu_frame_id_     = "odom";
  std::string flu_frame_id_     = "base_link";

private:
  void checkParamList(const std::string &param,
                      std::vector<std::string> &_params_list,
                      bool &_all_params_read);

  void updateControllerParameter(const std::shared_ptr<pid_controller::PIDController> &_pid_handler,
                                 const std::string &_parameter_name,
                                 const rclcpp::Parameter &_param);

  void updateController3DParameter(
      const std::shared_ptr<pid_controller::PIDController3D> &_pid_handler,
      const std::string &_parameter_name,
      const rclcpp::Parameter &_param);

  void resetState();
  void resetReferences();
  void resetCommands();

  void computeActions(geometry_msgs::msg::PoseStamped &pose,
                      geometry_msgs::msg::TwistStamped &twist,
                      as2_msgs::msg::Thrust &thrust);

  void getOutput(geometry_msgs::msg::TwistStamped &twist_msg, const std::string &frame_id);
};
};  // namespace controller_plugin_speed_controller

#endif
