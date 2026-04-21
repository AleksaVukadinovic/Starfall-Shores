/**
 * @file Engine.hpp
 * @brief Includes all the engine headers.
 */

#pragma once

#include <engine/core/App.hpp>

#include <engine/core/Controller.hpp>


#include <engine/platform/Window.hpp>
#include <engine/platform/Input.hpp>
#include <engine/platform/PlatformController.hpp>

#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/Camera.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/graphics/FPSCameraController.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/Renderer.hpp>

#include <engine/util/Utils.hpp>
#include <engine/util/Transform.hpp>
#include <engine/util/Configuration.hpp>
#include <engine/util/ArgParser.hpp>
#include <engine/util/Errors.hpp>

#include <engine/resources/ShaderCompiler.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>
#include <engine/resources/Texture.hpp>
#include <engine/resources/Skybox.hpp>
#include <engine/platform/TimeController.hpp>
