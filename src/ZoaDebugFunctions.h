/*
 * ZoaDebugFunctions.h
 * The place where commented out functions go to live
 *
 * @author Mario Gonzalez
 */

#ifndef ZOADEBUGFUNCTIONS_H_
#define ZOADEBUGFUNCTIONS_H_

#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"

class ZoaDebugFunctions
{
public:
	static void trimeshDrawNormals( ci::TriMesh &mesh );
	static void cameraDrawBillboard( const ci::CameraPersp &camera, ci::Vec2f position );
	static void drawFloorPlane( float floorSize );

	// Modifies the 4 passed vertices to create a quad of 'size' dimensions
	static void createQuadAtPosition( ci::Vec3f position,
			ci::Vec3f& v1, ci::Vec3f& v2, ci::Vec3f& v3, ci::Vec3f& v4,
			float size, float noise, float rotationY );
};

#endif /* ZOADEBUGFUNCTIONS_H_ */
