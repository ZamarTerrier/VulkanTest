#pragma once

#include "stdinclude.h"

#include "camera.h"

#include "tools.h"

#include "gameObject.h"
#include "resource.h"

class DirLight{
public:
    DirLight(Device* device, Camera* camera, ShadersPath paths)
    {
        this->camera = camera;

        PrimitiveObject pObject = Tools::GetPrimitives(PrimitiveType::PRIMITIVE_TYPE_QUAD);
        pObject.vertices[0].color = pObject.vertices[0].ColorGet(76, 60, 24);
        pObject.vertices[1].color = pObject.vertices[0].ColorGet(76, 60, 24);
        pObject.vertices[2].color = pObject.vertices[0].ColorGet(76, 60, 24);
        pObject.vertices[3].color = pObject.vertices[0].ColorGet(76, 60, 24);

        go = new GameObject (device, camera);

        go->SetShadersName(paths.vertShader, paths.fragShader);
        go->setVertex(pObject.vertices);
        go->setIndices(pObject.indices);
        go->applyLight = false;
        go->Init();

        distance = 250;

    }
    ~DirLight()
    {
        delete go;
    }
    void Init()
    {
        Resource::sunDir = glm::vec3(1,-1,1);
    }

    void Update(float deltaTime)
    {
        int m_del = 1000;
        float resultAngle = angle / m_del;
        sunPos = target;
        sunPos.x -= distance * cos(resultAngle);
        sunPos.z -= distance * sin(resultAngle);

        angle ++;

        if(angle >=360 * m_del)
            angle = 0;

        go->SetPosition(sunPos);
        go->SetRotation(glm::vec3(0,-resultAngle + 90,0) * 10.0f);      
        Resource::sunDir = glm::vec3(cos(resultAngle), -0.5f, sin(resultAngle));
        go->Update(deltaTime);
    }

    void SetTarget(glm::vec3 pos)
    {
        target = pos;
    }

    GameObject* go;

    Camera* camera;

    float distance;
    float angle;

    glm::vec3 sunPos;
    glm::vec3 target;

};