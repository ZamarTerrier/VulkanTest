#pragma once

#include "stdinclude.h"

#include "skyBox.h"
#include "gameObject.h"

#include "dirLight.h"

#include "device.h"
#include "graphics.h"

class Game{
public:
    Game(Device* device, Graphics* graphics)
    {
        this->device = device;
        this->graphics = graphics;
    }
    ~Game()
    {
        for(int x= 0; x < chunks.size(); x++)
        {
            for(int y= 0; y < chunks[x].size(); y++)
            {
                for(int z= 0; z < chunks[x][y].size(); z++)
                {
                    delete chunks[x][y][z];
                }
            }
        }

        delete gameObject;
        delete gameObject2;
        delete skyBox;
        delete dirLight;
        delete camera;
        
    }
    void Init(){
        
        camera = new Camera(Resource::swapChainExtent.width, Resource::swapChainExtent.height);
        
        PrimitiveObject pObject = Tools::GetPrimitives(PrimitiveType::PRIMITIVE_TYPE_CAPSULE);
        PrimitiveObject Terrain;

        gameObject = new GameObject(device, camera);
        gameObject->SetShadersName("shaders/vert.spv","shaders/frag.spv");    
        gameObject->SetSize(2);      
        gameObject->SetPosition({0,0,6});  
        gameObject->setVertex(pObject.vertices);
        gameObject->setIndices(pObject.indices);

        gameObject2 = new GameObject(device, camera);      
        gameObject2->SetSize(2);
        gameObject2->SetShadersName("shaders/vert.spv","shaders/frag2.spv");
        gameObject2->setVertex(pObject.vertices);
        gameObject2->setIndices(pObject.indices);

        skyBox = new SkyBox(device, camera, {"shaders/skyVert.spv","shaders/skyFrag.spv"});

        
        int size = 4;

        chunks.resize(size);

        for(int x= 0; x < size; x++)
        {
            chunks[x].resize(size);
            for(int y= 0; y < size; y++)
            {
                chunks[x][y].resize(size);
                for(int z= 0; z < size; z++)
                {
                    glm::vec3 pos;

                    pos.x = x; pos.y = y; pos.z = z;
                    
                    Terrain = Tools::GetMCubes(pos);

                    chunks[x][y][z] = new GameObject(device, camera);

                    chunks[x][y][z]->SetPosition(glm::vec3(x * 160, y * 160, z * 160));
                    chunks[x][y][z]->SetShadersName("shaders/vert.spv","shaders/frag.spv");
                    chunks[x][y][z]->setVertex(Terrain.vertices);
                    chunks[x][y][z]->setIndices(Terrain.indices);

                    graphics->SetGameObject(chunks[x][y][z]);

                }
            }
        }

        dirLight = new DirLight(device, camera, {"shaders/vert.spv","shaders/frag.spv"});
        dirLight->Init();     
        
        graphics->SetGameObject(gameObject);
        graphics->SetGameObject(gameObject2);
        graphics->SetGameObject(skyBox->go);
        graphics->SetGameObject(dirLight->go);
    }

    void Update(float time){

        camera->Update(time);
        gameObject->SetRotate({0,0,10});
        gameObject->Update(time);
        //gameObject2->Update(time);

        for(int x= 0; x < chunks.size(); x++)
        {
            for(int y= 0; y < chunks[x].size(); y++)
            {
                for(int z= 0; z < chunks[x][y].size(); z++)
                {
                    chunks[x][y][z]->Update(time);
                }
            }
        }

        dirLight->SetTarget(camera->GetPosition());  
        dirLight->Update(time);

        skyBox->Update(time);
    }

    void Draw(VkCommandBuffer cmd, int indx)
    {        
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gameObject->pipeline->graphicsPipeline);
        gameObject->Draw(cmd, indx);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gameObject2->pipeline->graphicsPipeline);
        gameObject2->Draw(cmd, indx);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, skyBox->go->pipeline->graphicsPipeline);
        skyBox->go->Draw(cmd, indx);        
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, dirLight->go->pipeline->graphicsPipeline);
        dirLight->go->Draw(cmd, indx);

        for(int x= 0; x < chunks.size(); x++)
        {
            for(int y= 0; y < chunks[x].size(); y++)
            {
                for(int z= 0; z < chunks[x][y].size(); z++)
                {
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, chunks[x][y][z]->pipeline->graphicsPipeline);
                    chunks[x][y][z]->Draw(cmd, indx);
                }
            }
        }
    }

    Graphics* graphics;

    SkyBox* skyBox;
    
    Camera* camera;

    GameObject* gameObject;
    GameObject* gameObject2;

    DirLight* dirLight;

    Device* device; 

    std::vector<std::vector<std::vector<GameObject*>>> chunks;

};