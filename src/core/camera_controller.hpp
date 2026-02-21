#pragma once

#include "camera.h"
#include "glm/common.hpp"
#include <glm/gtx/compatibility.hpp>

enum class CameraPreset { FRONT, TOP, ISOMETRIC };

class CameraController {
private:
  Camera &m_camera;
  CameraPreset m_activePreset = CameraPreset::FRONT;

  glm::vec3 m_target;

  float m_targetYaw, m_targetPitch, m_targetDistance;
  float m_curYaw, m_curPitch, m_curDistance;

public:
  CameraController(Camera &camera)
      : m_camera(camera), m_target(0.0, 10.0f, 0.0f) {

    m_curYaw = m_targetYaw = camera.GetYaw();
    m_curPitch = m_targetPitch = camera.GetPitch();
    m_curDistance = m_targetDistance = 50.0f;
  }

  void SetPreset(CameraPreset preset) {
    m_activePreset = preset;
    switch (preset) {
    case CameraPreset::FRONT:
      m_targetYaw = -90.0f;
      m_targetPitch = 0.0f;
      m_targetDistance = -35.0f;
      break;
    case CameraPreset::TOP:
      m_targetPitch = -89.0f; // Stay near top
      m_targetDistance = -35.0f;
      break;
    case CameraPreset::ISOMETRIC:
      m_targetYaw = -135.0f;
      m_targetPitch = -30.0f;
      m_targetDistance = -35.0f;
      break;
    }
  }

  void Update(float delta_time) {
    // Smoothly interpolate values toward targets
    // TODO: move this into camera controller properties
    float lerpSpeed = 5.0f;

    m_curYaw = glm::lerp(m_curYaw, m_targetYaw, lerpSpeed * delta_time);
    m_curPitch = glm::lerp(m_curPitch, m_targetPitch, lerpSpeed * delta_time);
    m_curDistance =
        glm::lerp(m_curDistance, m_targetDistance, lerpSpeed * delta_time);

    // Update Camera orientation
    m_camera.SetYaw(m_curYaw);
    m_camera.SetPitch(m_curPitch);

    // Calculate Position based on Orbit math (Spherical Coordinates)
    // We calculate position relative to the m_target (center of game)
    glm::vec3 offset;
    offset.x = m_curDistance * cos(glm::radians(m_curYaw)) *
               cos(glm::radians(m_curPitch));
    offset.y = m_curDistance * sin(glm::radians(m_curPitch));
    offset.z = m_curDistance * sin(glm::radians(m_curYaw)) *
               cos(glm::radians(m_curPitch));

    m_camera.Position = m_target + offset;
  }

  void HandleRotationInput(bool left, bool right, bool up, bool down,
                           float delta_time) {
    // TODO: move this into camera controller properties
    float orbitSpeed = 90.0f; // Degrees per second

    if (left)
      m_targetYaw += orbitSpeed * delta_time;
    if (right)
      m_targetYaw -= orbitSpeed * delta_time;
    if (up)
      m_targetPitch -= orbitSpeed * delta_time;
    if (down)
      m_targetPitch += orbitSpeed * delta_time;

    m_targetPitch = glm::clamp(m_targetPitch, -89.0f, 89.0f);
  }
};
