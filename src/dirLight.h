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

    }
    ~DirLight()
    {
        delete go;
    }
    void Init()
    {
        Resource::sunPos = glm::vec3(1,-1,1);
    }
    void Update(float deltaTime)
    {
        go->SetPosition(Resource::sunPos * 10.0f);
        go->Update(deltaTime);
    }

    GameObject* go;

    Camera* camera;

};