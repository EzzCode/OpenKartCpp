#ifndef BULLET_DEBUG_DRAWER_H
#define BULLET_DEBUG_DRAWER_H

#include <LinearMath/btIDebugDraw.h>
#include <vector>

class BulletDebugDrawer : public btIDebugDraw {
private:
    int m_debugMode;

    std::vector<float> m_lines;

public:
    BulletDebugDrawer();

    virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color);

    virtual void reportErrorWarning(const char* warningString);

    virtual void setDebugMode(int debugMode);

    virtual int getDebugMode(void) const;

    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override {
        // Implementation for drawing contact points
        // For simplicity, we will just draw a line from the contact point in the direction of the normal
        btVector3 to = PointOnB + normalOnB * distance;
        drawLine(PointOnB, to, color);
    }

    virtual void draw3dText(const btVector3& location, const char* textString) {

    }

    void glfw3_device_create(void);

    void glfw3_device_render(const float *matrix);

    void glfw3_device_destroy(void);

};
#endif
