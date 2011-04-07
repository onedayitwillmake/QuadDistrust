/**
 * ZoaDebugFunctions.h
 * The place where commented out functions go to live
 *
 * @author Mario Gonzalez
 */
#include "ZoaDebugFunctions.h"

void ZoaDebugFunctions::trimeshDrawNormals( ci::TriMesh &mesh )
{
	std::vector<ci::Vec3f> meshVertices = mesh.getVertices();
	std::vector<ci::Vec3f> meshNormals = mesh.getNormals();
	for( size_t i = 3; i < meshNormals.size(); i+=4 )
	{
		float t = 0.5f; // Because T is 0.5 it's not really necessary but the we could project along the plane this way
		ci::Vec3f midPoint = ci::Vec3f( (1.0f - t) * ( meshVertices[i-2] ) + t * ( meshVertices[i] ) );
		ci::Vec3f normal = meshNormals[i]*10;
		ci::gl::drawVector( midPoint, midPoint+normal, 10, 2.5);
	}
}

//
void ZoaDebugFunctions::cameraDrawBillboard( const ci::CameraPersp &camera, ci::Vec2f position )
{
	ci::Vec3f mRight, mUp;
	camera.getBillboardVectors(&mRight, &mUp);
	ci::gl::drawBillboard( ci::Vec3f::zero(), position, 0.0f, mRight, mUp);
}

void ZoaDebugFunctions::drawFloorPlane( float floorSize )
{
	// Draw floor plane
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex3f(-floorSize, 0.0f, floorSize);
		glTexCoord2f(1.0f,1.0f); glVertex3f( floorSize, 0.0f, floorSize);
		glTexCoord2f(1.0f,0.0f); glVertex3f( floorSize, 0.0f,-floorSize);
		glTexCoord2f(0.0f,0.0f); glVertex3f(-floorSize, 0.0f,-floorSize);
	glEnd();
}
