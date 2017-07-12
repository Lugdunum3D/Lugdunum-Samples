#include "Application.hpp"

#include <imgui.h>

#include <lug/Graphics/Builder/Camera.hpp>
#include <lug/Graphics/Builder/Light.hpp>
#include <lug/Graphics/Builder/Material.hpp>
#include <lug/Graphics/Builder/Mesh.hpp>
#include <lug/Graphics/Builder/Scene.hpp>
#include <lug/Graphics/Builder/Texture.hpp>
#include <lug/Graphics/Renderer.hpp>
#include <lug/Graphics/Vulkan/Renderer.hpp>
#include <lug/Math/Geometry/Trigonometry.hpp>

Application::Application() : lug::Core::Application::Application{{"sample_07", {0, 1, 0}}} {
    getRenderWindowInfo().windowInitInfo.title = "Sample 07";
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

    // Create the sphere mesh
    if (!initSphereMesh()) {
        return false;
    }

    // Load the base color texture
    lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Texture> baseColorTexture;
    {
        lug::Graphics::Builder::Texture textureBuilder(*renderer);

        textureBuilder.addLayer("textures/rustediron2_basecolor.jpg");
        textureBuilder.setMinFilter(lug::Graphics::Render::Texture::Filter::Linear);
        textureBuilder.setMagFilter(lug::Graphics::Render::Texture::Filter::Linear);

        baseColorTexture = textureBuilder.build();
        if (!baseColorTexture) {
            LUG_LOG.error("Application: Can't create the base color texture");
            return false;
        }
    }

    // Load the metallic roughness texture
    lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Texture> metallicRoughnessTexture;
    {
        lug::Graphics::Builder::Texture textureBuilder(*renderer);

        textureBuilder.addLayer("textures/rustediron2_metallic_roughness.jpg");
        textureBuilder.setMinFilter(lug::Graphics::Render::Texture::Filter::Linear);
        textureBuilder.setMagFilter(lug::Graphics::Render::Texture::Filter::Linear);

        metallicRoughnessTexture = textureBuilder.build();
        if (!metallicRoughnessTexture) {
            LUG_LOG.error("Application: Can't create the metallic roughness texture");
            return false;
        }
    }

    // Load the normal texture
    lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Texture> normalTexture;
    {
        lug::Graphics::Builder::Texture textureBuilder(*renderer);

        textureBuilder.addLayer("textures/rustediron2_normal.jpg");
        textureBuilder.setMinFilter(lug::Graphics::Render::Texture::Filter::Linear);
        textureBuilder.setMagFilter(lug::Graphics::Render::Texture::Filter::Linear);

        normalTexture = textureBuilder.build();
        if (!normalTexture) {
            LUG_LOG.error("Application: Can't create the normal texture");
            return false;
        }
    }

    // Load the emissive texture
    lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Texture> emissiveTexture;
    {
        lug::Graphics::Builder::Texture textureBuilder(*renderer);

        textureBuilder.addLayer("textures/rustediron2_emissive.jpg");
        textureBuilder.setMinFilter(lug::Graphics::Render::Texture::Filter::Linear);
        textureBuilder.setMagFilter(lug::Graphics::Render::Texture::Filter::Linear);

        emissiveTexture = textureBuilder.build();
        if (!emissiveTexture) {
            LUG_LOG.error("Application: Can't create the emissive texture");
            return false;
        }
    }

    // Attach the spheres
    {
        lug::Graphics::Builder::Material materialBuilder(*renderer);
        materialBuilder.setBaseColorFactor({1.0f, 1.0f, 1.0f, 1.0f});

        {
            lug::Graphics::Scene::Node* node = _scene->createSceneNode("sphere0");
            _scene->getRoot().attachChild(*node);

            lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Material> material = materialBuilder.build();
            if (!material) {
                LUG_LOG.error("Application: Can't create the material");
                return false;
            }

            node->attachMeshInstance(_sphereMesh, material);

            node->setPosition({
                -6.0f,
                0.0f,
                0.0f
            }, lug::Graphics::Node::TransformSpace::World);
        }

        {
            materialBuilder.setBaseColorTexture(baseColorTexture, 0);

            lug::Graphics::Scene::Node* node = _scene->createSceneNode("sphere1");
            _scene->getRoot().attachChild(*node);

            lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Material> material = materialBuilder.build();
            if (!material) {
                LUG_LOG.error("Application: Can't create the material");
                return false;
            }

            node->attachMeshInstance(_sphereMesh, material);

            node->setPosition({
                -3.0f,
                0.0f,
                0.0f
            }, lug::Graphics::Node::TransformSpace::World);
        }

        {
            materialBuilder.setMetallicRoughnessTexture(metallicRoughnessTexture, 0);

            lug::Graphics::Scene::Node* node = _scene->createSceneNode("sphere2");
            _scene->getRoot().attachChild(*node);

            lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Material> material = materialBuilder.build();
            if (!material) {
                LUG_LOG.error("Application: Can't create the material");
                return false;
            }

            node->attachMeshInstance(_sphereMesh, material);

            node->setPosition({
                0.0f,
                0.0f,
                0.0f
            }, lug::Graphics::Node::TransformSpace::World);
        }

        {
            materialBuilder.setNormalTexture(normalTexture, 0);

            lug::Graphics::Scene::Node* node = _scene->createSceneNode("sphere3");
            _scene->getRoot().attachChild(*node);

            lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Material> material = materialBuilder.build();
            if (!material) {
                LUG_LOG.error("Application: Can't create the material");
                return false;
            }

            node->attachMeshInstance(_sphereMesh, material);

            node->setPosition({
                3.0f,
                0.0f,
                0.0f
            }, lug::Graphics::Node::TransformSpace::World);
        }

        {
            materialBuilder.setEmissiveFactor({1.0f, 1.0f, 1.0f});
            materialBuilder.setEmissiveTexture(emissiveTexture, 0);

            lug::Graphics::Scene::Node* node = _scene->createSceneNode("sphere4");
            _scene->getRoot().attachChild(*node);

            lug::Graphics::Resource::SharedPtr<lug::Graphics::Render::Material> material = materialBuilder.build();
            if (!material) {
                LUG_LOG.error("Application: Can't create the material");
                return false;
            }

            node->attachMeshInstance(_sphereMesh, material);

            node->setPosition({
                6.0f,
                0.0f,
                0.0f
            }, lug::Graphics::Node::TransformSpace::World);
        }
    }

    // Set the position of the camera
    {
        _scene->getSceneNode("camera")->setPosition({0.0f, 0.0f, 15.0f}, lug::Graphics::Node::TransformSpace::World);
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

bool Application::initSphereMesh() {
    std::vector<lug::Math::Vec3f> positions;
    std::vector<lug::Math::Vec3f> normals;
    std::vector<lug::Math::Vec2f> uv;
    std::vector<uint16_t> indices;

    // Generate positions / normals / indices
    {
        const int X_SEGMENTS = 64;
        const int Y_SEGMENTS = 64;
        for (int y = 0; y <= Y_SEGMENTS; ++y) {
            for (int x = 0; x <= X_SEGMENTS; ++x) {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * lug::Math::pi<float>()) * std::sin(ySegment * lug::Math::pi<float>());
                float yPos = std::cos(ySegment * lug::Math::pi<float>());
                float zPos = std::sin(xSegment * 2.0f * lug::Math::pi<float>()) * std::sin(ySegment * lug::Math::pi<float>());

                positions.push_back({xPos, yPos, zPos});
                uv.push_back({xSegment, ySegment});
                normals.push_back({xPos, yPos, zPos});
            }
        }

        bool oddRow = false;
        for (int y = 0; y < Y_SEGMENTS; ++y) {
            if (!oddRow) { // even rows: y == 0, y == 2; and so on
                for (int x = 0; x <= X_SEGMENTS; ++x) {
                    indices.push_back(static_cast<uint16_t>((y + 1) * (X_SEGMENTS + 1) + x));
                    indices.push_back(static_cast<uint16_t>(y       * (X_SEGMENTS + 1) + x));
                }
            } else {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back(static_cast<uint16_t>(y       * (X_SEGMENTS + 1) + x));
                    indices.push_back(static_cast<uint16_t>((y + 1) * (X_SEGMENTS + 1) + x));
                }
            }

            oddRow = !oddRow;
        }
    }

    // Build the mesh
    {
        lug::Graphics::Builder::Mesh meshBuilder(*_graphics.getRenderer());
        meshBuilder.setName("sphere");

        lug::Graphics::Builder::Mesh::PrimitiveSet* primitiveSet = meshBuilder.addPrimitiveSet();

        primitiveSet->setMode(lug::Graphics::Render::Mesh::PrimitiveSet::Mode::TriangleStrip);

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
            uv.data(),
            sizeof(lug::Math::Vec2f),
            static_cast<uint32_t>(uv.size()),
            lug::Graphics::Render::Mesh::PrimitiveSet::Attribute::Type::TexCoord
        );

        _sphereMesh = meshBuilder.build();

        if (!_sphereMesh) {
            LUG_LOG.error("Application: Can't create the sphere mesh");
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
