#pragma once

// A timer that will be associated to the dying
struct DeathTimer {
  float counter_ms = 0.f;
};

// Will wander aimlessly
struct Wander {
  float active_dir_cd = 0.f;
  float change_dir_cd = 0.f;
};


struct WanderLine {
  float active_dir_cd = 0.f;
  float change_dir_cd = 0.f;
};

// Will wander in a square
struct WanderSquare {
  bool        clockwise     = false;
  float       active_dir_cd = 0.f;
  float       change_dir_cd = 0.f;
};

struct TracksPlayer {
  bool  active_track = false;
  float curr_cd      = 0.f;
  float tracking_cd  = 0.f;
  float spot_radius  = 0.f;
  float leash_radius = 0.f;
  float acceleration = 0.f;
};
