#ifndef CAMERA_H
#define CAMERA_H

#include "glm/fwd.hpp"
#include <cmath>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
  // Camera Attributes
  glm::vec3 Position;
  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

private:
  // Euler Angles
  float m_yaw;
  float m_pitch;

  // Camera Vectors
  glm::vec3 m_front;
  glm::vec3 m_up;
  glm::vec3 m_right;
  glm::vec3 m_worldUp;

  float m_sceneWidth = 800.0f;
  float m_sceneHeight = 600.0f;

public:
  // Constructor with vectors
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
         float pitch = PITCH)
      : m_front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
        MouseSensitivity(SENSITIVITY), Zoom(ZOOM), m_yaw(yaw), m_pitch(pitch),
        m_worldUp(up) {
    Position = position;
    updateCameraVectors();
  }

  // Constructor with scalar values
  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
         float yaw, float pitch)
      : m_front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
        MouseSensitivity(SENSITIVITY), Zoom(ZOOM), m_yaw(yaw), m_pitch(pitch) {
    Position = glm::vec3(posX, posY, posZ);
    m_worldUp = glm::vec3(upX, upY, upZ);
    updateCameraVectors();
  }

  // Matrix Getters
  glm::mat4 GetViewMatrix() const {
    return glm::lookAt(Position, Position + m_front, m_up);
  }

  glm::mat4 GetProjectionMatrix() const {
    return glm::perspective(glm::radians(Zoom), GetAspect(), 0.1f, 100.0f);
  }

  float GetAspect() const {
    return (m_sceneHeight > 0) ? (m_sceneWidth / m_sceneHeight) : 1.0f;
  }

  // Input Processing
  void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
      Position += m_front * velocity;
    if (direction == BACKWARD)
      Position -= m_front * velocity;
    if (direction == LEFT)
      Position -= m_right * velocity;
    if (direction == RIGHT)
      Position += m_right * velocity;
  }

  void ProcessMouseMovement(float xoffset, float yoffset,
                            GLboolean constrainPitch = true) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    if (constrainPitch) {
      if (m_pitch > 89.0f)
        m_pitch = 89.0f;
      if (m_pitch < -89.0f)
        m_pitch = -89.0f;
    }

    updateCameraVectors();
  }

  void ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
      Zoom = 1.0f;
    if (Zoom > 45.0f)
      Zoom = 45.0f;
  }

  // Setters & Getters
  void SetYaw(float yaw, bool update_camera_vectors = true) {
    m_yaw = yaw;

    if (update_camera_vectors)
      updateCameraVectors();
  }

  void SetPitch(float pitch, bool update_camera_vectors = true) {
    m_pitch = pitch;
    if (m_pitch > 89.0f)
      m_pitch = 89.0f;
    if (m_pitch < -89.0f)
      m_pitch = -89.0f;

    if (update_camera_vectors)
      updateCameraVectors();
  }

  float GetYaw() const { return m_yaw; }
  float GetPitch() const { return m_pitch; }

  glm::vec3 GetRight() const { return m_right; }
  glm::vec3 GetFront() const { return m_front; }

  void UpdateSceneSize(float width, float height) {
    m_sceneWidth = width;
    m_sceneHeight = height;
  }

private:
  // Internal update logic
  void updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
  }
};

#endif
