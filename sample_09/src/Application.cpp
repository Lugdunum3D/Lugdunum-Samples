#include "Application.hpp"

#include <imgui.h>

#include <lug/Graphics/Builder/Camera.hpp>
#include <lug/Graphics/Builder/Light.hpp>
#include <lug/Graphics/Builder/Material.hpp>
#include <lug/Graphics/Builder/Mesh.hpp>
#include <lug/Graphics/Builder/Scene.hpp>
#include <lug/Graphics/Builder/SkyBox.hpp>
#include <lug/Graphics/Builder/Texture.hpp>
#include <lug/Graphics/Renderer.hpp>
#include <lug/Graphics/Vulkan/Renderer.hpp>
#include <lug/Math/Geometry/Trigonometry.hpp>

Application::Application() : lug::Core::Application::Application{{"sample_09", {0, 1, 0}}} {
    getRenderWindowInfo().windowInitInfo.title = "Sample 09";
}

bool Application::init(int argc, char* argv[]) {
    if (!lug::Core::Application::init(argc, argv)) {
        return false;
    }

    lug::Graphics::Renderer* renderer = _graphics.getRenderer();

    // Load scene
    lug::Graphics::Resource::SharedPtr<lug::Graphics::Resource> sceneResource = renderer->getResourceManager()->loadFile("models/DamagedHelmet/DamagedHelmet.gltf");
    if (!sceneResource) {
        LUG_LOG.error("Application: Can't load the model");
        return false;
    }

    _scene = lug::Graphics::Resource::SharedPtr<lug::Graphics::Scene::Scene>::cast(sceneResource);

    // Attach cameras
    {
        lug::Graphics::Builder::Camera cameraBuilder(*renderer);

        cameraBuilder.setFovY(45.0f);
        cameraBuilder.setZNear(0.1f);
        cameraBuilder.setZFar(100.0f);

        lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Camera::Camera> camera = cameraBuilder.build();
        if (!camera) {
            LUG_LOG.error("Application: Can't create the camera");
            return false;
        }

        lug::Graphics::Scene::Node* node = _scene->createSceneNode("camera");
        _scene->getRoot().attachChild(*node);

        node->attachCamera(camera);

        // Attach a mover to the first camera
        _mover.setTargetNode(*node);
        _mover.setEventSource(*renderer->getWindow());

        // Attach camera to RenderView
        {
            auto& renderViews = _graphics.getRenderer()->getWindow()->getRenderViews();

            LUG_ASSERT(renderViews.size() > 0, "There should be at least 1 render view");

            renderViews[0]->attachCamera(camera);
        }
    }

    // Attach skyBox
    {
        lug::Graphics::Builder::SkyBox skyBoxBuilder(*renderer);

        skyBoxBuilder.setFaceFilename(lug::Graphics::Builder::SkyBox::Face::PositiveX, "textures/skybox/right.jpg");
        skyBoxBuilder.setFaceFilename(lug::Graphics::Builder::SkyBox::Face::NegativeX, "textures/skybox/left.jpg");
        skyBoxBuilder.setFaceFilename(lug::Graphics::Builder::SkyBox::Face::PositiveY, "textures/skybox/top.jpg");
        skyBoxBuilder.setFaceFilename(lug::Graphics::Builder::SkyBox::Face::NegativeY, "textures/skybox/bottom.jpg");
        skyBoxBuilder.setFaceFilename(lug::Graphics::Builder::SkyBox::Face::PositiveZ, "textures/skybox/back.jpg");
        skyBoxBuilder.setFaceFilename(lug::Graphics::Builder::SkyBox::Face::NegativeZ, "textures/skybox/front.jpg");

        lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::SkyBox> skyBox = skyBoxBuilder.build();
        if (!skyBox) {
            LUG_LOG.error("Application: Can't create skyBox");
            return false;
        }

        _scene->setSkyBox(skyBox);
    }

    // Set the position of the camera
    {
        _scene->getSceneNode("camera")->setPosition({0.0f, 0.0f, 5.0f}, lug::Graphics::Node::TransformSpace::World);
        _scene->getSceneNode("camera")->getCamera()->lookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, lug::Graphics::Node::TransformSpace::World);
    }

    // Attach 4 lights
    const lug::Math::Vec3f lightPositions[] = {
        lug::Math::Vec3f{-10.0f,  10.0f, 10.0f},
        lug::Math::Vec3f{ 10.0f,  10.0f, 10.0f},
        lug::Math::Vec3f{-10.0f, -10.0f, 10.0f},
        lug::Math::Vec3f{ 10.0f, -10.0f, 10.0f},
    };

    for (uint32_t i = 0; i < 4; ++i) {
        lug::Graphics::Builder::Light lightBuilder(*renderer);

        lightBuilder.setType(lug::Graphics::Render::Light::Type::Point);
        lightBuilder.setColor({300.0f, 300.0f, 300.0f, 1.0f});
        lightBuilder.setLinearAttenuation(0.0f);

        lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Light> light = lightBuilder.build();
        if (!light) {
            LUG_LOG.error("Application: Can't create the point light {}", i);
            return false;
        }

        lug::Graphics::Scene::Node* node = _scene->createSceneNode("light" + std::to_string(i));
        _scene->getRoot().attachChild(*node);

        node->setPosition(lightPositions[i]);
        node->attachLight(light);
    }

    return true;
}

void Application::onEvent(const lug::Window::Event& event) {
    if (event.type == lug::Window::Event::Type::Close) {
        close();
    }
}

void Application::onFrame(const lug::System::Time& elapsedTime) {
    _mover.onFrame(elapsedTime);

    ImGui::Begin("Light");
    {
        ImGui::SetWindowSize({200, 100});
        ImGui::SetWindowPos({590, 490});

        auto light = _scene->getSceneNode("light0")->getLight();

        float r = light->getColor().r();
        ImGui::SliderFloat("red", &r, 0.0f, 600.0f);

        float g = light->getColor().g();
        ImGui::SliderFloat("green", &g, 0.0f, 600.0f);

        float b = light->getColor().b();
        ImGui::SliderFloat("blue", &b, 0.0f, 600.0f);

        if (r != light->getColor().r() || g != light->getColor().g() || b != light->getColor().b()) {
            for (uint32_t i = 0; i < 4; ++i) {
                _scene->getSceneNode("light" + std::to_string(i))->getLight()->setColor({r, g, b, 1.0f});
            }
        }
    }
    ImGui::End();
}
