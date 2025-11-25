#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// 输入处理相关函数声明
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
