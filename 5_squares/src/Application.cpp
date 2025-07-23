#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>

#include <stb_image/stb_image.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <array>
#include <sstream>
#include <fstream>

struct Vec2 { float x, y; };
struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(glm::mat4 model, glm::vec3 pos)
    {
        glm::vec4 res = model * glm::vec4(pos, 1);
        x = res.x;
        y = res.y;
        z = res.z;
    }
};
struct Vec4 { float x, y, z, w; };
struct Vertex
{
    Vec3 Position;
    Vec4 Color;
    Vec2 TexCoords;
    float TexID;
};

class Shader
{
private:
    unsigned int m_ID;

    unsigned int CompileShader(unsigned int type, const std::string& source)
    {
        int success;
        char infoLog[512];

        unsigned int shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        return shader;
    }
    unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
    {
        unsigned int shaderProgram = glCreateProgram();
        unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

        glAttachShader(shaderProgram, vs);
        glAttachShader(shaderProgram, fs);
        glLinkProgram(shaderProgram);

        int success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        glDeleteShader(vs);
        glDeleteShader(fs);

        return shaderProgram;
    }
    int GetUniformLocation(const std::string& name) { return glGetUniformLocation(m_ID, name.c_str()); }
    std::string ParseShader(const std::string& filepath)
    {
        std::ifstream stream(filepath);
        std::string line;
        std::stringstream ss;

        while (getline(stream, line)) { ss << line << '\n'; }

        return ss.str();
    }
public:
    Shader(const std::string& vsPath, const std::string& fsPath) { m_ID = CreateShader(ParseShader(vsPath), ParseShader(fsPath)); }
    ~Shader() { glDeleteProgram(m_ID); }
    
    void Bind() const { glUseProgram(m_ID); }
    void Unbind() const { glUseProgram(0); }
    
    void SetUniform1i(const std::string& name, int value) { glUniform1i(GetUniformLocation(name), value); }
    void SetUniform1iv(const std::string& name, int count ,int *value) { glUniform1iv(GetUniformLocation(name), count, value); }
    void SetUniform1f(const std::string& name, float value) { glUniform1f(GetUniformLocation(name), value); }
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) { glUniform4f(GetUniformLocation(name), v0, v1, v2, v3); }
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix) { glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]); }
};

class VertexBuffer
{
private:
    unsigned int m_ID;
public:
    VertexBuffer(const void* data, unsigned int size, unsigned int type)
    {
        glGenBuffers(1, &m_ID);
        glBindBuffer(GL_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ARRAY_BUFFER, size, data, type);
    }
    ~VertexBuffer() { glDeleteBuffers(1, &m_ID); }

    void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_ID); }
    void Unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

    unsigned int GetID() const { return m_ID; }
};

class VertexArray
{
private:
    unsigned int m_ID;
public:
    VertexArray(){glGenVertexArrays(1, &m_ID);}
    ~VertexArray(){glDeleteVertexArrays(1, &m_ID);}

    void Bind() const { glBindVertexArray(m_ID); }
    void Unbind() const { glBindVertexArray(0); }

    void AddBuffer(VertexBuffer& VBO)
    {
        Bind();
        VBO.Bind();
        glEnableVertexArrayAttrib(VBO.GetID(), 0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

        glEnableVertexArrayAttrib(VBO.GetID(), 1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

        glEnableVertexArrayAttrib(VBO.GetID(), 2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoords));

        glEnableVertexArrayAttrib(VBO.GetID(), 3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexID));
    }
};

class IndexBuffer
{
private:
    unsigned int m_ID;
public:
    IndexBuffer(const void* data, unsigned int size, unsigned int type)
    {
        glGenBuffers(1, &m_ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, type);
    }
    ~IndexBuffer() { glDeleteBuffers(1, &m_ID); }

    void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID); }
    void Unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
};

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;
const char* textureSource1 = "src/textures/tex1.jpg";
const char* textureSource2 = "src/textures/tex2.jpg";

const float size = 50.0f;

static std::array<Vertex, 4> CreateSquare(float x, float y, float size, Vec4 color, float TexID, float angle);
static GLuint LoadTexture(const std::string& path);

int main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Program", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();

    glm::mat4 mvp = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, -1.0f, 1.0f);

    int samplers[2] = { 0, 1 };
    
    unsigned int texture1 = LoadTexture(textureSource1);
    unsigned int texture2 = LoadTexture(textureSource2);
    glBindTextureUnit(samplers[0], texture1);
    glBindTextureUnit(samplers[1], texture2);

    Shader shader("src/shaders/vertexShader.shader", "src/shaders/fragmentShader.shader");
    shader.Bind();
    shader.SetUniform1iv("u_Textures", 2, samplers);
    shader.SetUniformMat4f("u_MVP", mvp);

    unsigned int indicies[]
    {
         0,  1,  2,  2,  3,  0,
         4,  5,  6,  6,  7,  4,
         8,  9, 10, 10, 11,  8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18 ,19, 16
    };

    VertexBuffer VBO(nullptr, 5 * 4 * sizeof(Vertex), GL_DYNAMIC_DRAW);

    VertexArray VAO;
    VAO.AddBuffer(VBO);

    IndexBuffer IBO(indicies, sizeof(indicies), GL_STATIC_DRAW);

    Vertex vertecies[20];

    float lastTime = 0;
    
    int dir = 1;
    float velocity = 0;
    float resultPos = WIDTH / 2.;

    float angle = 0;
    float resultAngle = 0;
    
    ImVec4 color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplGlfwGL3_NewFrame();

        ImGui::Begin("Button");
        ImGui::DragFloat("valocity", &velocity, 1.0f, 0.0f, 10000.0f);
        ImGui::DragFloat("angle", &angle, 1.0f, 0.0f, 720.0f);
        ImGui::ColorEdit3("clear color", (float*)&color);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        if (resultPos > WIDTH * 2 / 3.) dir = -1;
        else if (resultPos < WIDTH / 3) dir = 1;

        float currentTime = ImGui::GetTime();
        resultPos += dir * velocity * (currentTime - lastTime);
        resultAngle += angle * (currentTime - lastTime);
        lastTime = currentTime;

        auto q0 = CreateSquare(WIDTH / 2., HEIGHT / 2., size, { color.x, color.y, color.z, color.w }, 0.0f, 0.0f);
        auto q1 = CreateSquare(WIDTH / 3., HEIGHT / 2., size, { 0.06f, 0.71f, 0.29f, 1.0f }, 1.0f, 0.0f);
        auto q2 = CreateSquare(WIDTH * 2 / 3., HEIGHT / 2., size, { 0.33f, 0.63f, 0.94f, 1.0f }, 2.0f, 0.0f);
        auto q3 = CreateSquare(WIDTH / 2., HEIGHT / 3., size, { 0.99f, 0.85f, 0.09f, 1.0f }, 0.0f, resultAngle);
        auto q4 = CreateSquare(resultPos, HEIGHT * 2 / 3., size, { 0.48f, 0.24f, 0.91f, 1.0f }, 0.0f, 0.0f);

        memcpy(vertecies, q0.data(), q0.size() * sizeof(Vertex));
        memcpy(vertecies + 4, q1.data(), q1.size() * sizeof(Vertex));
        memcpy(vertecies + 8, q2.data(), q2.size() * sizeof(Vertex));
        memcpy(vertecies + 12, q3.data(), q3.size() * sizeof(Vertex));
        memcpy(vertecies + 16, q4.data(), q4.size() * sizeof(Vertex));

        glBindBuffer(GL_ARRAY_BUFFER, VBO.GetID());
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertecies), vertecies);

        shader.Bind();
        VAO.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(indicies) / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}

static std::array<Vertex,4> CreateSquare(float x, float y, float size, Vec4 color, float TexID, float angle)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 0, 1));
    model = glm::translate(model, -glm::vec3(x, y, 0));

    Vertex v0;
    v0.Position = Vec3(model, glm::vec3( x - size / 2, y - size / 2, 0.0f));
    v0.Color = color;
    v0.TexCoords = { 0.0f, 0.0f };
    v0.TexID = TexID;
    
    Vertex v1;
    v1.Position = Vec3(model, glm::vec3( x + size / 2, y - size / 2, 0.0f));
    v1.Color = color;
    v1.TexCoords = { 1.0f, 0.0f };
    v1.TexID = TexID;
    
    Vertex v2;
    v2.Position = Vec3(model, glm::vec3( x + size / 2, y + size / 2, 0.0f));
    v2.Color = color;
    v2.TexCoords = { 1.0f, 1.0f };
    v2.TexID = TexID;
    
    Vertex v3;
    v3.Position = Vec3(model, glm::vec3( x - size / 2, y + size / 2, 0.0f));
    v3.Color = color;
    v3.TexCoords = { 0.0f, 1.0f };
    v3.TexID = TexID;

    return { v0, v1, v2, v3 };
}

static GLuint LoadTexture(const std::string& path)
{
    int width, height, bpp;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* localBuffer = stbi_load(path.c_str(), &width, &height, &bpp, 4);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    if (localBuffer)
        stbi_image_free(localBuffer);
    return texture;
}