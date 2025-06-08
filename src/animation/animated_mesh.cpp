#include "animated_mesh.hpp"
#include <iostream>

AnimatedMesh::AnimatedMesh(std::vector<AnimatedVertex> vertices, std::vector<unsigned int> indices, 
                           std::vector<std::shared_ptr<Texture2D>> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
}

void AnimatedMesh::Draw(GLSLProgram& shader) {
    // Set material properties first
    shader.setUniformFloat("material.shininess", 64.0f);
    
    // Initialize texture binding state
    bool hasTextures = !textures.empty();
    
    if (hasTextures) {
        // Bind textures and set uniforms
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        
        for (unsigned int i = 0; i < textures.size(); i++) {
            textures[i]->bind(i);
            
            // Determine texture type based on position (this is a simplified approach)
            std::string name;
            std::string number;
            
            if (i == 0) {
                name = "texture_diffuse";
                number = std::to_string(diffuseNr++);
            } else if (i == 1) {
                name = "texture_specular";
                number = std::to_string(specularNr++);
            } else if (i == 2) {
                name = "texture_normal";
                number = std::to_string(normalNr++);
            } else if (i == 3) {
                name = "texture_height";
                number = std::to_string(heightNr++);
            } else {
                // Default to diffuse for additional textures
                name = "texture_diffuse";
                number = std::to_string(diffuseNr++);
            }
            
            // Set the sampler to the correct texture unit
            shader.setUniformInt(("material." + name + number).c_str(), i);
        }
    } else {
        // No textures loaded - bind a default white texture
        static GLuint whiteTexture = 0;
        if (whiteTexture == 0) {
            glGenTextures(1, &whiteTexture);
            glBindTexture(GL_TEXTURE_2D, whiteTexture);
            unsigned char whitePixel[] = {255, 255, 255, 255};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        shader.setUniformInt("material.texture_diffuse1", 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        shader.setUniformInt("material.texture_specular1", 1);
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Unbind textures
    if (hasTextures) {
        for (unsigned int i = 0; i < textures.size(); i++) {
            textures[i]->unbind();
        }
    }
    
    // Reset active texture
    glActiveTexture(GL_TEXTURE0);
}

void AnimatedMesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(AnimatedVertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, Bitangent));
    // bone ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, m_BoneIDs));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex), (void*)offsetof(AnimatedVertex, m_Weights));

    glBindVertexArray(0);
}
