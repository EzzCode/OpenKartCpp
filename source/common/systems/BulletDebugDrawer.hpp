#ifndef BULLET_DEBUG_DRAWER_H
#define BULLET_DEBUG_DRAWER_H

#include <LinearMath/btIDebugDraw.h>
#include <vector>
#include <iostream>

class BulletDebugDrawer : public btIDebugDraw {
private:
    int m_debugMode;
    std::vector<float> m_lines;
    std::vector<float> m_colors; // Store colors for each line
    bool m_initialized;          // Track initialization state
        /*
    Debug Color Legend (varies by debug mode):
    
    WIREFRAME MODE (DBG_DrawWireframe):
    - GREEN: Static objects (mass = 0) like ground/walls  
    - WHITE/GRAY: Dynamic objects (mass > 0) like vehicles
    - BLUE: Wireframes and constraint visualization
    
    CONTACT POINTS MODE (DBG_DrawContactPoints):
    - RED: Contact points where collisions occur (vehicles touching ground)
    - YELLOW: Contact normals and directional vectors
    
    ADVANCED DEBUG (DBG_DrawAabb):
    - CYAN: AABB bounding boxes
    
    Note: When contact points are enabled, vehicles appear red due to 
    constant wheel-ground contact. Use wireframe-only mode to see 
    vehicle wireframes in white/gray.
    */

public:
    BulletDebugDrawer();
    ~BulletDebugDrawer(); // Destructor for cleanup

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

    virtual void reportErrorWarning(const char* warningString);

    virtual void setDebugMode(int debugMode);

    virtual int getDebugMode(void) const;    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override {
        // Enhanced contact point visualization with proper color
        btVector3 to = PointOnB + normalOnB * distance;
        drawLine(PointOnB, to, color);
        
        // Draw a small cross at the contact point for better visibility
        btVector3 cross1 = PointOnB + btVector3(0.1f, 0, 0);
        btVector3 cross2 = PointOnB - btVector3(0.1f, 0, 0);
        btVector3 cross3 = PointOnB + btVector3(0, 0.1f, 0);
        btVector3 cross4 = PointOnB - btVector3(0, 0.1f, 0);
        
        drawLine(cross1, cross2, btVector3(1.0f, 1.0f, 0.0f)); // Yellow cross
        drawLine(cross3, cross4, btVector3(1.0f, 1.0f, 0.0f));
    }

    virtual void draw3dText(const btVector3& location, const char* textString) {
        // Improved text rendering placeholder - could be enhanced with actual text rendering
        std::cout << "Debug Text at (" << location.x() << ", " << location.y() << ", " << location.z() 
                  << "): " << textString << std::endl;
    }    void glfw3_device_create(void);

    void glfw3_device_render(const float *matrix);

    void glfw3_device_destroy(void);
    
    // Additional utility methods
    bool isInitialized() const { return m_initialized; }
    void clearLines() { m_lines.clear(); m_colors.clear(); }
    size_t getLineCount() const { return m_lines.size() / 6; } // 2 vertices per line, 3 coords per vertex

};
#endif
