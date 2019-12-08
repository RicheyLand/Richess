#ifndef MODEL_H
#define MODEL_H

#include "head.h"

struct Vertex                                           //  represents single vertex in the 3D space
{
    glm::vec3 Position;
    glm::vec3 Normal;                                   //  it is required to save normal vector too because of lighting calculation
};

class Model                                             //  creates and draws real polygonal model from vertices and indices file
{                                                       //  all faces of desired object need to be triangulated
private:
    unsigned int VBO;                                   //  apppropriate buffers
    unsigned int EBO;

    void init()                                         //  initialize requested model
    {
        glGenVertexArrays(1, &VAO);                     //  generate required buffers
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);                         //  execute buffers binding
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);                    // execute mapping of vertices into the buffers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);   
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        glBindVertexArray(0);                           //  bind vertex array after enabling
    }

public:
    vector<Vertex> vertices;                            //  holds all vertices if desired 3D object
    vector<unsigned int> indices;                       //  holds all indices if desired 3D object
    unsigned int VAO;

    Model(string verticesPath, string indicesPath)      //  class constructor which requires paths to vertices and indices files
    {
        ifstream fin;
        fin.open(verticesPath.c_str(), ios::in);        //  parsing of vertices file is going to be executed

        if (fin)
        {
            Vertex vertex;
            int count = 0;

            while (fin)
            {
                string line = "";
                fin >> line;                            //  read source file be single tokens separated by white character

                if (line.size())
                {
                    if (count == 6)                     //  handle offset between next vertex
                    {
                        count = 0;
                        this->vertices.push_back(vertex);   //  push new loaded vertex into the list
                    }
                                                        //  convert token value into the float number
                    if (count == 0)
                        vertex.Position.x = atof(line.c_str());
                    else if (count == 1)
                        vertex.Position.y = atof(line.c_str());
                    else if (count == 2)
                        vertex.Position.z = atof(line.c_str());
                    else if (count == 3)
                        vertex.Normal.x = atof(line.c_str());
                    else if (count == 4)
                        vertex.Normal.y = atof(line.c_str());
                    else if (count == 5)
                        vertex.Normal.z = atof(line.c_str());

                    count++;
                }
            }

            if (count == 6)                             //  handle offset between next index
                this->vertices.push_back(vertex);       //  push new loaded index into the list

            fin.close();
        }

        fin.open(indicesPath.c_str(), ios::in);         //  parsing of indices file is going to be executed

        if (fin)
        {
            while (fin)
            {
                string line = "";
                fin >> line;                            //  read source file be single tokens separated by white character

                if (line.size())
                    this->indices.push_back(atoi(line.c_str()));    //  convert token value into the integer number
            }

            fin.close();
        }

        init();                                         //  draw desired object
    }

    void render()                                       //  allows to draw the 3D object
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);   //  draw all polygons of desired object
        glBindVertexArray(0);
    }
};

class ModelUV
{
private:
    vector<glm::vec3> positions;
    vector<glm::vec3> normals;
    vector<unsigned int> indices;
    vector<glm::vec2> uv;

    unsigned int VAO = 0;
    unsigned int VBO;
    unsigned int EBO;

    bool initFlag = false;

    void init()
    {
        glGenVertexArrays(1, &VAO);

        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        vector<float> data;
        unsigned N = positions.size();

        for (unsigned i = 0; i < N; i++)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);

            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }

            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
        }

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        float stride = (3 + 2 + 3) * sizeof(float);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));

        initFlag = true;
    }

public:
    ModelUV(string pathPositions, string pathNormals, string pathIndices, string pathUV)
    {
        ifstream fin;
        fin.open(pathPositions, ios::in);

        if (fin)
        {
            while (true)
            {
                glm::vec3 value;
                fin >> value.x >> value.y >> value.z;

                if (fin.eof())
                    break;

                positions.push_back(value);
            }

            fin.close();
        }

        fin.open(pathNormals, ios::in);

        if (fin)
        {
            while (true)
            {
                glm::vec3 value;
                fin >> value.x >> value.y >> value.z;

                if (fin.eof())
                    break;

                normals.push_back(value);
            }

            fin.close();
        }

        fin.open(pathIndices, ios::in);

        if (fin)
        {
            while (true)
            {
                unsigned int value;
                fin >> value;

                if (fin.eof())
                    break;

                indices.push_back(value);
            }

            fin.close();
        }

        fin.open(pathUV, ios::in);

        if (fin)
        {
            while (true)
            {
                glm::vec2 value;
                fin >> value.x >> value.y;

                if (fin.eof())
                    break;

                uv.push_back(value);
            }

            fin.close();
        }

        if (positions.size() && normals.size() && uv.size() && indices.size())
            init();
    }

    void render()
    {
        if (initFlag)
        {
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }
};

#endif
