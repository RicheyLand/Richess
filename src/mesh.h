#ifndef MESH_H
#define MESH_H

#include "head.h"

class Mesh                                                              //  mesh loader class with rendering support
{
private:
    vector<glm::vec3> positions;                                        //  array of all required mesh position coordinates
    vector<glm::vec3> normals;                                          //  array of all required mesh normal coordinates
    vector<unsigned int> indices;                                       //  indices array
    vector<glm::vec2> uv;                                               //  array of all required UV mapping coordinates

    unsigned int VAO = 0;
    unsigned int VBO;
    unsigned int EBO;

    bool initFlag = false;                                              //  holds if mesh has been successfully initialized

    void init()                                                         //  initialize mesh using loaded data
    {
        glGenVertexArrays(1, &VAO);

        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        vector<float> data;                                             //  loaded mesh needs to be represented as a linear array
        unsigned N = positions.size();

        for (unsigned i = 0; i < N; i++)                                //  every position of mesh needs to be handled
        {
            data.push_back(positions[i].x);                             //  push all three position coordinates into the data array
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);

            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);                                //  push all three UV mapping coordinates into the data array
                data.push_back(uv[i].y);
            }

            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);                           //  push all three normal coordinates into the data array
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
        }

        glBindVertexArray(VAO);                                         //  setup and bind all required buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        float stride = (3 + 2 + 3) * sizeof(float);                     //  setup appropriate stride for data array

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);      //  use stride value for position values in array
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));    //  use stride value for UV values in array
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));    //  use stride value for normal values in array

        initFlag = true;                                                //  mesh has been successfully initilized
    }

public:
    Mesh(string pathPositions, string pathNormals, string pathIndices, string pathUV)   //  constructor requires paths to all coordinate files as parameters
    {
        ifstream fin;
        fin.open(pathPositions, ios::in);                               //  try to open files with position coordinates

        if (fin)                                                        //  check if file has been successfully opened
        {
            while (true)
            {
                glm::vec3 value;
                fin >> value.x >> value.y >> value.z;                   //  load sequence of three coordinates from the file

                if (fin.eof())                                          //  end of the file has been reached
                    break;

                positions.push_back(value);
            }

            fin.close();
        }

        fin.open(pathNormals, ios::in);                                 //  try to open files with normal coordinates

        if (fin)                                                        //  check if file has been successfully opened
        {
            while (true)
            {
                glm::vec3 value;
                fin >> value.x >> value.y >> value.z;                   //  load sequence of three coordinates from the file

                if (fin.eof())                                          //  end of the file has been reached
                    break;

                normals.push_back(value);
            }

            fin.close();
        }

        fin.open(pathIndices, ios::in);                                 //  try to open files with indices

        if (fin)                                                        //  check if file has been successfully opened
        {
            while (true)
            {
                unsigned int value;
                fin >> value;                                           //  load single index value from the file

                if (fin.eof())                                          //  end of the file has been reached
                    break;

                indices.push_back(value);
            }

            fin.close();
        }

        fin.open(pathUV, ios::in);                                      //  try to open files with UV coordinates

        if (fin)                                                        //  check if file has been successfully opened
        {
            while (true)
            {
                glm::vec2 value;
                fin >> value.x >> value.y;                              //  load sequence of two coordinates from the file

                if (fin.eof())                                          //  end of the file has been reached
                    break;

                uv.push_back(value);
            }

            fin.close();
        }

        if (positions.size() && normals.size() && uv.size() && indices.size())
            init();                                                     //  initialize mesh only if data from files has been loaded
    }

    void render()
    {
        if (initFlag)                                                   //  only initialized mesh could be rendered
        {
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);   //  draw loaded polygonal mesh
            glBindVertexArray(0);
        }
    }
};

#endif
