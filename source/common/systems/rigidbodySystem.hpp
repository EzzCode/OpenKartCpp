#pragma once

#include "../ecs/world.hpp"
#include "../components/rigidbody.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <iostream>
using namespace std;
namespace our
{

    class RigidbodySystem
    {
    public:
        btDiscreteDynamicsWorld *dynWorld;
        void enter(btDiscreteDynamicsWorld *world)
        {
            dynWorld = world;
        }

        void update(World *world, float deltaTime)
        {
            for (auto entity : world->getEntities())
            {
                RigidbodyComponent *rigidbodyComponent = entity->getComponent<RigidbodyComponent>();
                if (rigidbodyComponent && !rigidbodyComponent->addedToWorld)
                {
                    
                    std::string path = "./assets/models/" + rigidbodyComponent->mesh + ".obj";
                    addObjectToWorld(path, dynWorld, btVector3(rigidbodyComponent->position.x, rigidbodyComponent->position.y, rigidbodyComponent->position.z), btVector3(rigidbodyComponent->rotation.x, rigidbodyComponent->rotation.y, rigidbodyComponent->rotation.z), rigidbodyComponent->mass);
                    rigidbodyComponent->addedToWorld = true;
                }
                if (rigidbodyComponent)
                {
                    cout << "Updating rigidbody" << endl;
                    cout << rigidbodyComponent->position.x << " " << rigidbodyComponent->position.y << " " << rigidbodyComponent->position.z << endl;
                    // Update the position of the entity based on the rigid body
                    entity->localTransform.position = rigidbodyComponent->position;
                }
            }
        }
        void addObjectToWorld(const std::string &path, btDiscreteDynamicsWorld *world, const btVector3 &position, const btVector3 &rotation, float mass)
        {
            // Step 1: Load OBJ file
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
            {
                throw std::runtime_error("Failed to load OBJ file: " + warn + err);
            }

            std::vector<VertexRigidbody> vertices_vec;
            std::vector<Face> faces;

            for (const auto &shape : shapes)
            {
                size_t index_offset = 0;
                for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
                {
                    size_t fv = shape.mesh.num_face_vertices[f];
                    if (fv != 3)
                    {
                        throw std::runtime_error("Only triangular faces are supported!");
                    }

                    Face face;
                    for (size_t v = 0; v < fv; v++)
                    {
                        tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                        tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                        tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                        tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

                        if (v == 0)
                            face.v1 = vertices_vec.size();
                        if (v == 1)
                            face.v2 = vertices_vec.size();
                        if (v == 2)
                            face.v3 = vertices_vec.size();

                        vertices_vec.push_back({vx, vy, vz});
                    }
                    index_offset += fv;
                    faces.push_back(face);
                }
            }
            // Step 2: Create a Bullet triangle mesh shape
            btTriangleMesh *triangleMesh = new btTriangleMesh();
            for (const auto &face : faces)
            {

                const VertexRigidbody &v1 = vertices_vec[face.v1];
                const VertexRigidbody &v2 = vertices_vec[face.v2];
                const VertexRigidbody &v3 = vertices_vec[face.v3];

                triangleMesh->addTriangle(
                    btVector3(v1.x, v1.y, v1.z),
                    btVector3(v2.x, v2.y, v2.z),
                    btVector3(v3.x, v3.y, v3.z));
            }

            btCollisionShape *shape = new btBvhTriangleMeshShape(triangleMesh, true);

            // Step 3: Create rigid body and add it to the world
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(position);
            transform.setRotation(btQuaternion(rotation.x(), rotation.y(), rotation.z()));

            btScalar btMass(mass);
            btVector3 localInertia(0, 0, 0);

            if (btMass != 0.0f)
            {
                shape->calculateLocalInertia(btMass, localInertia);
            }

            btDefaultMotionState *motionState = new btDefaultMotionState(transform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(btMass, motionState, shape, localInertia);
            btRigidBody *rigidbody = new btRigidBody(rbInfo);

            world->addRigidBody(rigidbody);
        }
    };

}
