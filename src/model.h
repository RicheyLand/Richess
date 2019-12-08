#ifndef MODEL_H
#define MODEL_H

#include "head.h"

class Model
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
    Model(string pathPositions, string pathNormals, string pathIndices, string pathUV)
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
