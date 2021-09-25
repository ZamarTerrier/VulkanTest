#pragma once

#include  "stdinclude.h"

#include "entity.h"
#include "device.h"
#include "resource.h"
#include "camera.h"

class GameObject : public Entity{
public:
    GameObject(Device* device, Camera* camera) : Entity(device, camera), size(1)
    {
        
    }
    virtual ~GameObject(){
        
    }
    void Init(){
        Entity::Init();

    }

    void Update(float deltaTime)
    {
        updateUniformBuffer();
    }
    
    void setVertex(std::vector<Vertex> vertices)
    {
        this->vertices = vertices;
    }
    
    void setIndices(std::vector<uint16_t> indices)
    {
        this->indices = indices;
    }

    void SetPosition(glm::vec3 position)
    {
        position /= 10;
        this->position = position;
        model = glm::translate(glm::mat4(1.0f), position);
    }

    void SetSize(float size)
    {
        this->size = size;
    }

    void SetShadersName(std::string vertFile,std::string fragFile)
    {
        this->vertFile = vertFile;
        this->fragFile = fragFile;
    }

    void SetRotate(glm::vec3 rotation)
    {
        // needed for rotation
        glm::vec3 center = (position + rotation) / 0.5f; // vec3(1.5,2.0,0.0) mid point due to angle = 180 deg
        //glm::vec3 axis = cross((position - rotation), ); // any perpendicular vector to `p1-p0` if `p1-p0` is parallel to (0,0,1) then use `(0,1,0)` instead

        model = glm::rotate(model, 0.001f, rotation);
    }

    std::string vertFile, fragFile;

private :
    void updateUniformBuffer() {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        
        UniformBufferObject ubo{};
        ubo.model = model;
        ubo.size = size;
        ubo.view = camera->view;
        ubo.proj = camera->proj;

        void* data;
        vkMapMemory(device->device, uniformBuffersMemory[Resource::currentImage], 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device->device, uniformBuffersMemory[Resource::currentImage]);
    }

    glm::mat4 model;
    glm::vec3 position;
    float size;

};