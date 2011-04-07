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
};

#endif /* ZOADEBUGFUNCTIONS_H_ */
