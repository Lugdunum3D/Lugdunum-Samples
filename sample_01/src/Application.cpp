#include "Application.hpp"

#include <lug/Graphics/Builder/Camera.hpp>
#include <lug/Graphics/Builder/Light.hpp>
#include <lug/Graphics/Builder/Material.hpp>
#include <lug/Graphics/Builder/Mesh.hpp>
#include <lug/Graphics/Builder/Scene.hpp>
#include <lug/Graphics/Renderer.hpp>
#include <lug/Graphics/Vulkan/Renderer.hpp>

Application::Application() : lug::Core::Application::Application{{"sample_01", {0, 1, 0}}} {
    getRenderWindowInfo().windowInitInfo.title = "Sample 01";
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

    // Attach camera
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

        // Attach camera to RenderView
        {
            auto& renderViews = _graphics.getRenderer()->getWindow()->getRenderViews();

            LUG_ASSERT(renderViews.size() > 0, "There should be at least 1 render view");

            renderViews[0]->attachCamera(camera);
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

    // Attach an ambient light
    {
        lug::Graphics::Builder::Light lightBuilder(*renderer);

        lightBuilder.setType(lug::Graphics::Render::Light::Type::Ambient);
        lightBuilder.setColor({1.0f, 1.0f, 1.0f, 1.0f});

        lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Light> light = lightBuilder.build();
        if (!light) {
            LUG_LOG.error("Application: Can't create the ambient light");
            return false;
        }

        _scene->getRoot().attachLight(light);
    }

    // Set the position of the camera
    {
        _scene->getSceneNode("camera")->setPosition({3.0f, 3.0f, 3.0f}, lug::Graphics::Node::TransformSpace::World);
        _scene->getSceneNode("camera")->getCamera()->lookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, lug::Graphics::Node::TransformSpace::World);
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

void Application::onFrame(const lug::System::Time&) {

}
