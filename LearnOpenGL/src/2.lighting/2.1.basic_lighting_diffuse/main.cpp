#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <functional>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <glad/glad.h>
#include <SFML/OpenGL.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

#include <fmt/core.h>

#include <Player.h>

#include <VertexArray.h>
#include <Buffer.h>
#include <Shader.h>
#include <Debug.h>
#include <Texture.h>


#include "cube_vertices_normal.h"

uint32_t screenWidth = 1280, screenHeight = 720;
sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "OpenGL", sf::Style::Default, sf::ContextSettings(24, 8, 4, 4, 4, sf::ContextSettings::Attribute::Debug));

int main()
{
    window.setVerticalSyncEnabled(true);
    // Load OpenGL functions using glad
    if (!gladLoadGL())
    {
        printf("Something went wrong!\n");
        exit(-1);
    }
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    (void)ImGui::SFML::Init(window);
    window.setActive(true);
    window.setMouseCursorVisible(false);

//    EnableOpenGLDebug();
    DisableOutputBuffer();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // Load m_ShaderID
    Shader lightShader("lighting.glsl");
    lightShader.setFloat3("fObjectColor", 1.0f, 0.5f, 0.31f);
    lightShader.setFloat3("fLightColor", 1.0f, 1.0f, 1.0f);

    Shader lightSurfaceShader("lightcube.glsl");

    // Load vertices
    VertexArray box;
    VertexBuffer *vb = new VertexBuffer;
    {
        vb->setData(vertices, sizeof(vertices));
        vb->setLayout({{ShaderDataType::Float3, "aPos"},
                      {ShaderDataType::Float3, "aNormal"}});
        box.AddVertexBuffer(vb);
    }
    VertexArray light;
    {
        vb->setData(vertices, sizeof(vertices));
        vb->setLayout({{ShaderDataType::Float3, "aPos"},
                       {ShaderDataType::Float3, "aNormal"}});
        light.AddVertexBuffer(vb);
    }

    FirstPersonView fpsView;

    sf::Clock stClk;
    sf::Clock deltaClock;
    float accClk = 0.f;
    bool running = true;
    while (running)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                running = false;
            else if (event.type == sf::Event::Resized)
            {
                glViewport(0, 0, event.size.width, event.size.height);
                screenWidth = event.size.width;
                screenHeight = event.size.height;
            }
            fpsView.processEvent(event);
        }
        float dt = stClk.restart().asSeconds();
        accClk += dt;

        // ImGui Update
        static float bgColor[4];
        static bool wire_mode = false;
        static float ambientStrength = 0.1;
        static glm::vec3 lightPos(3.0f, 0.f, -3.f);
        glm::mat4 model(1.f), view(1.f), projection(1.f);
        {
            ImGui::SFML::Update(window, deltaClock.restart());

            ImGui::Begin("Help", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            ImGui::Text("2.2.1.basic_lighting_diffuse");
            ImGui::Text("WASD - Move the camera");
            ImGui::Text("Space/Ctrl - Go up / down");
            ImGui::Text("Left Alt - Enable/Disable mouse capture");
            ImGui::Text("Esc - quit");
            ImGui::End();

            ImGui::Begin("Matrix");
            ImGui::DragFloat3("Position", glm::value_ptr(lightPos));
            ImGui::DragFloat("Ambient", &ambientStrength, 0.01f, 0.f, 1.f);

            ImGui::ColorEdit4("BG", bgColor, ImGuiColorEditFlags_Float);
            // Model matrix

            ImGui::Text("Model");
            //
            ImGui::Separator();
            // View matrix
            ImGui::Text("View");
            view = fpsView.getViewMatrix();
            ImGui::DragFloat3("Camera", glm::value_ptr(fpsView.m_camera.m_pos));
            ImGui::DragFloat("Speed", &fpsView.cameraSpeed);
            ImGui::DragFloat("Sensitivity", &fpsView.sensitivity);
            // Projection matrix
            ImGui::Text("Projection");
            ImGui::DragFloat("FOV", &fpsView.m_camera.m_fov, 1.0f, 0.f, 360.f);
            ImGui::DragFloat("Near", &fpsView.m_camera.m_near, 0.01);
            ImGui::DragFloat("Far", &fpsView.m_camera.m_far, 0.1);
            ImGui::Text("fpsView.m_camera.m_screenRatio = %f", fpsView.m_camera.m_screenRatio);
            projection = fpsView.getProjectionMatrix();
            ImGui::End();
        }

        // Update
        {
            fpsView.update(dt);
        }

        // Render
        {
            glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Upload uniforms
            lightShader.setMat4("vView", view);
            lightShader.setMat4("vProjection", projection);
            lightShader.setFloat("fAmbientStrength", ambientStrength);
            lightShader.setFloat3("fLightPos", lightPos);
            //
            lightSurfaceShader.setMat4("vView", view);
            lightSurfaceShader.setMat4("vProjection", projection);

            // Render box
            {
                box.Bind();
                model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
                model = glm::rotate(model, /*glm::radians(*/accClk/*)*/, glm::vec3(0.f, 1.f, 0.f));
                lightShader.setMat4("vModel", model);
                model = glm::mat4(1.f);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                box.Unbind();
            }

            // Render light
            {
                light.Bind();
                model = glm::translate(model, lightPos);
                model = glm::scale(model, glm::vec3(0.2f));
                lightSurfaceShader.setMat4("vModel", model);
                model = glm::mat4(1.f);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                light.Unbind();
            }
        }

        window.pushGLStates();
        ImGui::SFML::Render(window);
        window.popGLStates();
        window.display();
    }

    return 0;
}