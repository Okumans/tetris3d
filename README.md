# 3D Tetris

A 3D implementation of the classic Tetris game, built with C++23 and OpenGL.

https://github.com/user-attachments/assets/a31f58f4-0060-4e51-a127-0b489aa70b2b

## Features

- **3D Gameplay:** Play Tetris in a 3D grid (10x10x20).
- **Camera Controls:** Switch between different camera views (Front, Top, Isometric) and rotate around the grid.
- **Classic Mechanics:** Move, rotate, soft drop, hard drop, and hold pieces.
- **Modern Rendering:** Shader-based rendering using OpenGL.

## Controls

| Action                     | Key / Input                     |
| :------------------------- | :------------------------------ |
| **Move Piece**             | Arrow Keys (Relative to Camera) |
| **Rotate Piece (Pitch)**   | `Shift` + `Up` / `Down`         |
| **Rotate Piece (Roll)**    | `Shift` + `Left` / `Right`      |
| **Rotate Piece (Yaw)**     | `Ctrl` + `Left` / `Right`       |
| **Hard Drop**              | `Enter`                         |
| **Soft Drop**              | `Space`                         |
| **Hold Piece**             | `H`                             |
| **Camera View 1 (Front)**  | `1`                             |
| **Camera View 2 (Top)**    | `2`                             |
| **Camera View 3 (Iso)**    | `3`                             |
| **Rotate Camera**          | `W`, `A`, `S`, `D`              |
| **Interact UI (None yet)** | `Left Click`                    |

## Build Instructions

This project uses CMake and requires a C++23 compliant compiler.

### Prerequisites

- **C++ Compiler:** Must support C++23 (specifically `<generator>` and ranges).
  - GCC 13+
  - Clang 16+ (with libc++)
  - MSVC 2022 (17.6+)
- **CMake:** 3.14 or newer.
- **OpenGL:** 3.3+ compatible drivers.
- **Git:** For cloning the repository.

### Building

1. Clone the repository:

    ```bash
    git clone <repository_url>
    cd tetris_3d
    ```

2. Create a build directory:

    ```bash
    mkdir build
    cd build
    ```

3. Configure the project:

    ```bash
    cmake ..
    ```

4. Build the project:

    ```bash
    cmake --build .
    ```

5. Run the game:

    ```bash
    ./bin/tetris-3d
    ```

## Project Structure

- **`src/core`**: Contains the entry point, application loop (`App`), camera controller, and shader manager.
- **`src/game`**: Implements the core game logic, including the `TetrisManager`, `Tetromino` logic, and grid management (`Space`).
- **`src/ui`**: Handles user interface elements and rendering.
- **`assets/shaders`**: GLSL shaders for rendering the game objects and UI.
- **`include`**: Shared header files.

## Dependencies

The project automatically handles dependencies via CMake (using CPM or standard `find_package`):

- **GLFW:** For window creation, context initialization, and input handling.
- **GLAD:** For loading OpenGL functions.
- **GLM:** For linear algebra and math operations.

