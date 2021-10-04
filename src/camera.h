#pragma once

#include "stdinclude.h"

#include "window.h"
#include "resource.h"

class Camera{
public:
    Camera(float width, float height){

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, ViewDistance);
        proj[1][1] *= -1;


        yaw = -90.0f;
        pitch = 0.0f;

    }
    ~Camera(){
        
    }

    void processMouse(float xpos, float ypos, GLboolean constrainPitch = true){
 
		if(firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
    
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; 
        lastX = xpos;
        lastY = ypos;
    
        float sensitivity = 0.05;
        xoffset *= sensitivity;
        yoffset *= sensitivity;
    
        yaw   += xoffset;
        pitch += yoffset;
    
        if(pitch > 89.0f)
            pitch = 89.0f;
        if(pitch < -89.0f)
            pitch = -89.0f;
    
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);
	}

    void Update(float deltaTime)
    {

        const float cameraSpeed = 0.2f; // настройте по вашему усмотрению
        float currSpeed;
        
        if(Resource::pressed[GLFW_KEY_LEFT_SHIFT])
            currSpeed = cameraSpeed * 4;
        else
            currSpeed = cameraSpeed;

        if(Resource::pressed[GLFW_KEY_W])
            cameraPos += currSpeed * cameraFront * deltaTime;
        if(Resource::pressed[GLFW_KEY_S])
            cameraPos -= currSpeed * cameraFront * deltaTime;
        if(Resource::pressed[GLFW_KEY_A])
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currSpeed * deltaTime;
        if(Resource::pressed[GLFW_KEY_D])
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currSpeed * deltaTime;

            
        if(!Resource::showCursor)
            processMouse(WindowManager::xpos, WindowManager::ypos);
        
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);


    }

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  10.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    inline static float ViewDistance = 600.f;

private:
    float yaw;
	float pitch;
	float mouseSensitivity;
    float lastX = 400, lastY = 300;
    float firstMouse = true;
    glm::vec2 moveDir;
};