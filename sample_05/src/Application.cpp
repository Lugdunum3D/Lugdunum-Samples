#include "Application.hpp"

#include <imgui.h>

#include <lug/Graphics/Builder/Camera.hpp>
#include <lug/Graphics/Builder/Light.hpp>
#include <lug/Graphics/Builder/Material.hpp>
#include <lug/Graphics/Builder/Mesh.hpp>
#include <lug/Graphics/Builder/Scene.hpp>
#include <lug/Graphics/Renderer.hpp>
#include <lug/Graphics/Vulkan/Renderer.hpp>
#include <lug/Math/Geometry/Trigonometry.hpp>

Application::Application() : lug::Core::Application::Application{{"sample_05", {0, 1, 0}}} {
    getRenderWindowInfo().windowInitInfo.title = "Sample 05";

    getRenderWindowInfo().renderViewsInitInfo.push_back({
        {                                                   // viewport
            {                                               // offset
                0.0f,                                       // x
                0.0f                                        // y
            },

            {                                               // extent
                0.5f,                                       // width
                1.0f                                        // height
            },

            0.0f,                                           // minDepth
            1.0f                                            // maxDepth
        },
        {                                                   // scissor
            {                                               // offset
                0.0f,                                       // x
                0.0f                                        // y
            },
            {                                               // extent
                1.0f,                                       // width
                1.0f                                        // height
            }
        },
        nullptr                                             // camera
    });

    getRenderWindowInfo().renderViewsInitInfo.push_back({
        {                                                   // viewport
            {                                               // offset
                0.5f,                                       // x
                0.0f                                        // y
            },

            {                                               // extent
                0.5f,                                       // width
                1.0f                                        // height
            },

            0.0f,                                           // minDepth
            1.0f                                            // maxDepth
        },
        {                                                   // scissor
            {                                               // offset
                0.0f,                                       // x
                0.0f                                        // y
            },
            {                                               // extent
                1.0f,                                       // width
                1.0f                                        // height
            }
        },
        nullptr                                             // camera
    });
}

bool Application::init(int argc, char* argv[]) {
    if (!lug::Core::Application::init(argc, argv)) {
        return false;
    }

    lug::Graphics::Renderer* renderer = _graphics.getRenderer();

    // Build the scene
    {
        lug::Graphics::Builder::Scene sceneBuilder(*renderer);
        sceneBuilder.setName("scene");

        _scene = sceneBuilder.build();
        if (!_scene) {
            LUG_LOG.error("Application: Can't create the scene");
            return false;
        }
    }

    // Attach cameras
    {
        lug::Graphics::Builder::Camera cameraBuilder(*renderer);

        cameraBuilder.setFovY(45.0f);
        cameraBuilder.setZNear(0.1f);
        cameraBuilder.setZFar(100.0f);

        // Camera 1
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

        // Camera 2
        lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Camera::Camera> camera2 = cameraBuilder.build();
        if (!camera) {
            LUG_LOG.error("Application: Can't create the camera");
            return false;
        }

        lug::Graphics::Scene::Node* node2 = _scene->createSceneNode("camera2");
        _scene->getRoot().attachChild(*node2);

        node2->attachCamera(camera2);

        // Attach camera to RenderView
        {
            auto& renderViews = _graphics.getRenderer()->getWindow()->getRenderViews();

            LUG_ASSERT(renderViews.size() > 1, "There should be at least 2 render view");

            renderViews[0]->attachCamera(camera);
            renderViews[1]->attachCamera(camera2);
        }
    }

    // Create the cube mesh
    if (!initCubeMesh()) {
        return false;
    }

    // Attach the cube mesh
    {
        // Create a default material
        lug::Graphics::Builder::Material materialBuilder(*renderer);

        lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Material> material = materialBuilder.build();
        if (!material) {
            LUG_LOG.error("Application: Can't create the material");
            return false;
        }

        // Create the node to attach the cube
        lug::Graphics::Scene::Node* node = _scene->createSceneNode("cube");
        _scene->getRoot().attachChild(*node);

        // Attach the cube
        node->attachMeshInstance(_cubeMesh, material);
    }

    // Attach a directional light
    {
        lug::Graphics::Builder::Light lightBuilder(*renderer);

        lightBuilder.setType(lug::Graphics::Render::Light::Type::Directional);
        lightBuilder.setColor({0.01f, 0.01f, 0.01f, 1.0f});
        lightBuilder.setDirection({2.0f, -3.0f, 2.0f});

        lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Light> light = lightBuilder.build();
        if (!light) {
            LUG_LOG.error("Application: Can't create the directional light");
            return false;
        }

        _scene->getRoot().attachLight(light);
    }

    // Attach a point light
    {
        lug::Graphics::Builder::Light lightBuilder(*renderer);

        lightBuilder.setType(lug::Graphics::Render::Light::Type::Point);
        lightBuilder.setColor({20.0f, 20.0f, 20.0f, 1.0f});

        lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Light> light = lightBuilder.build();
        if (!light) {
            LUG_LOG.error("Application: Can't create the point light");
            return false;
        }

        _scene->getSceneNode("camera")->attachLight(light);
    }

    // Set the position of the camera
    {
        _scene->getSceneNode("camera")->setPosition({3.0f, 3.0f, 3.0f}, lug::Graphics::Node::TransformSpace::World);
        _scene->getSceneNode("camera")->getCamera()->lookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, lug::Graphics::Node::TransformSpace::World);

        _scene->getSceneNode("camera2")->setPosition({3.0f, 3.0f, -3.0f}, lug::Graphics::Node::TransformSpace::World);
        _scene->getSceneNode("camera2")->getCamera()->lookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, lug::Graphics::Node::TransformSpace::World);
    }

    return true;
}

bool Application::initCubeMesh() {
    const std::vector<lug::Math::Vec3f> positions = {
        // Back
        {-1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f},
        {-1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f},

        // Front
        {-1.0f, -1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},

        // Left
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, 1.0f},

        // Right
        {1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, 1.0f},

        // Bottom
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, 1.0f},
        {1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, 1.0f},

        // Top
        {-1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, 1.0}
    };

    const std::vector<lug::Math::Vec3f> normals = {
        // Back
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},

        // Front
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},

        // Left
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},

        // Right
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},

        // Bottom
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},

        // Top
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}
    };

    const std::vector<lug::Math::Vec4f> colors = {
        // Back
        {0.0f, 0.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},

        // Front
        {1.0f, 0.0f, 1.0, 1.0f},
        {1.0f, 0.0f, 1.0, 1.0f},
        {1.0f, 0.0f, 1.0, 1.0f},
        {1.0f, 0.0f, 1.0, 1.0f},

        // Left
        {1.0f, 0.0f, 0.0, 1.0f},
        {1.0f, 0.0f, 0.0, 1.0f},
        {1.0f, 0.0f, 0.0, 1.0f},
        {1.0f, 0.0f, 0.0, 1.0f},

        // Right
        {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},

        // Bottom
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},

        // Top
        {0.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 1.0f, 1.0f}
    };

    const std::vector<uint16_t> indices = {
        // Back
        0, 2, 1,
        1, 2, 3,

        // Front
        6, 4, 5,
        7, 6, 5,

        // Left
        10, 8, 9,
        11, 10, 9,

        // Right
        14, 13, 12,
        15, 13, 14,

        // Bottom
        17, 16, 19,
        19, 16, 18,

        // Top
        23, 20, 21,
        22, 20, 23
    };

    // Build the mesh
    {
        lug::Graphics::Builder::Mesh meshBuilder(*_graphics.getRenderer());
        meshBuilder.setName("cube");

        lug::Graphics::Builder::Mesh::PrimitiveSet* primitiveSet = meshBuilder.addPrimitiveSet();

        primitiveSet->setMode(lug::Graphics::Render::Mesh::PrimitiveSet::Mode::Triangles);

        primitiveSet->addAttributeBuffer(
            indices.data(),
            sizeof(uint16_t),
            static_cast<uint32_t>(indices.size()),
            lug::Graphics::Render::Mesh::PrimitiveSet::Attribute::Type::Indice
        );

        primitiveSet->addAttributeBuffer(
            positions.data(),
            sizeof(lug::Math::Vec3f),
            static_cast<uint32_t>(positions.size()),
            lug::Graphics::Render::Mesh::PrimitiveSet::Attribute::Type::Position
        );

        primitiveSet->addAttributeBuffer(
            normals.data(),
            sizeof(lug::Math::Vec3f),
            static_cast<uint32_t>(normals.size()),
            lug::Graphics::Render::Mesh::PrimitiveSet::Attribute::Type::Normal
        );

        primitiveSet->addAttributeBuffer(
            colors.data(),
            sizeof(lug::Math::Vec4f),
            static_cast<uint32_t>(colors.size()),
            lug::Graphics::Render::Mesh::PrimitiveSet::Attribute::Type::Color
        );

        _cubeMesh = meshBuilder.build();

        if (!_cubeMesh) {
            LUG_LOG.error("Application: Can't create the cube mesh");
            return false;
        }
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

    _scene->getSceneNode("cube")->rotate(
        ::lug::Math::Geometry::radians(90.0f) * elapsedTime.getSeconds<float>(),
        {0.0f, 0.0f, 1.0f},
        lug::Graphics::Node::TransformSpace::World
    );

    ImGui::Begin("Light");
    {
        ImGui::SetWindowSize({200, 100});
        ImGui::SetWindowPos({590, 490});

        auto light = _scene->getSceneNode("camera")->getLight();

        float r = light->getColor().r();
        ImGui::SliderFloat("red", &r, 0.0f, 50.0f);

        float g = light->getColor().g();
        ImGui::SliderFloat("green", &g, 0.0f, 50.0f);

        float b = light->getColor().b();
        ImGui::SliderFloat("blue", &b, 0.0f, 50.0f);

        if (r != light->getColor().r() || g != light->getColor().g() || b != light->getColor().b()) {
            light->setColor({r, g, b, 1.0f});
        }
    }
    ImGui::End();
}
