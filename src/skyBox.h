#pragma once

#include "stdinclude.h"

#include "device.h"

#include "camera.h"
#include "gameObject.h"


class SkyBox{
public:
    SkyBox(Device* device, Camera* camera, ShadersPath paths)
    {
        this->camera = camera;

        colorBottom = {0.0f, 0.0f, 0.1f};
        colorTop = {0.1f, 0.1f, 1.0f};

        vertices = {
            //Back
            {{-1.0f, -1.0f, 1.0f}, {0.0f,0.0f,0.0f}, colorBottom},
            {{1.0f, -1.0f, 1.0f}, {0.0f,0.0f,0.0f},colorBottom},
            {{1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f},colorTop},        
            {{-1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f},colorTop},
            //Front
            {{-1.0f, -1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorBottom},
            {{1.0f, -1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorBottom},
            {{1.0f, 1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorTop},        
            {{-1.0f, 1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorTop},
            //Left
            {{-1.0f, -1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorBottom},
            {{-1.0f, 1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorTop},
            {{-1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f}, colorTop},        
            {{-1.0f, -1.0f, 1.0f}, {0.0f,0.0f,0.0f}, colorBottom},
            //Right
            {{1.0f, -1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorBottom},
            {{1.0f, 1.0f, -1.0f},{0.0f,0.0f,0.0f}, colorTop},
            {{1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f}, colorTop},        
            {{1.0f, -1.0f, 1.0f},{0.0f,0.0f,0.0f}, colorBottom},
            //Top
            {{-1.0f, 1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorTop},
            {{-1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f}, colorTop},       
            {{1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f}, colorTop},
            {{1.0f, 1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorTop}, 
            //Bottom
            {{-1.0f, -1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorBottom},
            {{-1.0f, -1.0f, 1.0f}, {0.0f,0.0f,0.0f}, colorBottom},       
            {{1.0f, -1.0f, 1.0f}, {0.0f,0.0f,0.0f}, colorBottom},
            {{1.0f, -1.0f, -1.0f}, {0.0f,0.0f,0.0f}, colorBottom},
        };

        indices = {
            0, 1, 2, 2, 3, 0,
            6, 5, 4, 4, 7, 6,
            10, 9, 8, 8, 11, 10,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            22, 21, 20, 20, 23, 22
        };

        go = new GameObject(device, camera);
        go->SetShadersName(paths.vertShader, paths.fragShader);
        float size = Camera::ViewDistance / 2;
        go->SetSize(glm::vec3(size,size,size));
        go->setVertex(vertices);
        go->setIndices(indices);
    }
    ~SkyBox()
    {
        delete go;
    }

    void Update(float deltaTime)
    {
        glm::vec3 pos = camera->GetPosition();
        go->SetPosition(pos);
        go->Update(deltaTime);
    }

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    glm::vec3 colorTop;
    glm::vec3 colorBottom;

    GameObject* go;
    Camera* camera;
};