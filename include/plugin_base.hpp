/*
 *  Copyright © 2017-2023 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <pipewire/filter.h>
#include <spa/param/latency-utils.h>
#include <mutex>
#include <ranges>
#include <span>
#include "pipe_manager.hpp"
#include "tags_plugin_name.hpp"

class PluginBase {
 public:
  PluginBase(std::string tag,
             std::string plugin_name,
             std::string plugin_package,
             const std::string& schema,
             const std::string& schema_path,
             PipeManager* pipe_manager,
             const bool& enable_probe = false);
  PluginBase(const PluginBase&) = delete;
  auto operator=(const PluginBase&) -> PluginBase& = delete;
  PluginBase(const PluginBase&&) = delete;
  auto operator=(const PluginBase&&) -> PluginBase& = delete;
  virtual ~PluginBase();

  struct data;

  struct port {
    struct data* data;
  };

  struct data {
    struct port* in_left = nullptr;
    struct port* in_right = nullptr;

    struct port* out_left = nullptr;
    struct port* out_right = nullptr;

    struct port* probe_left = nullptr;
    struct port* probe_right = nullptr;

    PluginBase* pb = nullptr;
  };

  const std::string log_tag;

  std::string name, package;

  pw_filter* filter = nullptr;

  pw_filter_state state = PW_FILTER_STATE_UNCONNECTED;

  bool can_get_node_id = false;

  bool enable_probe = false;

  uint n_samples = 0U;

  uint rate = 0U;

  bool package_installed = true;

  bool bypass = false;

  bool connected_to_pw = false;

  bool send_notifications = false;

  float delta_t = 0.0F;

  float notification_time_window = 1.0F / 20.0F;  // seconds

  float latency_value = 0.0F;  // seconds

  std::chrono::time_point<std::chrono::system_clock> clock_start;

  std::vector<float> dummy_left, dummy_right;

  [[nodiscard]] auto get_node_id() const -> uint;

  void set_active(const bool& state) const;

  void set_post_messages(const bool& state);

  auto connect_to_pw() -> bool;

  void disconnect_from_pw();

  void reset_settings();

  virtual void setup();

  virtual void process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out);

  virtual void process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out,
                       std::span<float>& probe_left,
                       std::span<float>& probe_right);

  virtual void update_probe_links();

  virtual auto get_latency_seconds() -> float;

  sigc::signal<void(const float, const float)> input_level;
  sigc::signal<void(const float, const float)> output_level;
  sigc::signal<void()> latency;

 protected:
  std::mutex data_mutex;

  GSettings* settings = nullptr;

  PipeManager* pm = nullptr;

  spa_hook listener{};

  data pf_data = {};

  bool post_messages = false;

  uint n_ports = 4;

  float input_gain = 1.0F;
  float output_gain = 1.0F;

  std::vector<gulong> gconnections;

  void setup_input_output_gain();

  void initialize_listener();

  void notify();

  void get_peaks(const std::span<float>& left_in,
                 const std::span<float>& right_in,
                 std::span<float>& left_out,
                 std::span<float>& right_out);

  static void apply_gain(std::span<float>& left, std::span<float>& right, const float& gain);

  void update_filter_params();

 private:
  uint node_id = 0U;

  float input_peak_left = util::minimum_linear_level, input_peak_right = util::minimum_linear_level;
  float output_peak_left = util::minimum_linear_level, output_peak_right = util::minimum_linear_level;
};
