/**
 * EclipseLovesCinder example application
 *
 * * On first run, run Project -> Clean...
 * * If you change your project name go into debug configurations (arrow next to bug icon), and modify where the debug application will run from
 *
 * This project is released under public domain, do whatever with it.
 *
 *
 * Mario Gonzalez
 * http://onedayitwillmake
 */
#include "cinder/Camera.h"
#include "cinder/TriMesh.h"
#include "cinder/Rand.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Surface.h"
#include "cinder/app/AppBasic.h"
#include "cinder/app/Renderer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"

class QuadDistrustApp : public ci::app::AppBasic {
public:
	void prepareSettings( ci::app::AppBasic::Settings *settings );
	void setup();
	void setupCamera();
	void setupQuadSprites();

	void	resize( ci::app::ResizeEvent event );
	void	mouseDown( ci::app::MouseEvent event );
	void	mouseMove( ci::app::MouseEvent event );
	void	mouseDrag( ci::app::MouseEvent event );
	void	mouseUp( ci::app::MouseEvent event );

	void update();
	void draw();
	void addQuadToMesh( ci::TriMesh& mesh, const ci::Vec3f& P0, const ci::Vec3f& P1, const ci::Vec3f& P2, const ci::Vec3f& P3, const ci::ColorA& color );
	void calculateTriMeshNormals( ci::TriMesh &mesh );

	// Draw a cube (
	ci::Matrix44f		_cubeRotation;
	ci::gl::Texture		_texture;
	ci::MayaCamUI		_mayaCam;
	ci::TriMesh*		_particleMesh;
};

void QuadDistrustApp::prepareSettings( ci::app::AppBasic::Settings *settings )
{
	settings->setWindowSize( 800, 600 );
}

void QuadDistrustApp::setup()
{
	_cubeRotation.setToIdentity();

	int colorRange = 128;
	// Creates a blue-green gradient to use as an OpenGL texture
	ci::Surface8u surface(256, 256, false);
	ci::Surface::Iter iter = surface.getIter();
	while( iter.line() ) {
		while( iter.pixel() ) {
			iter.r() = iter.x()/2;
			iter.g() = iter.x()/2;
			iter.b() = iter.x()/2;
		}
	}

	_texture = ci::gl::Texture( surface );

	setupQuadSprites();
	getFocus();
}


// Creates a bunch of quads, trying to copy this structure
/*
glBegin(GL_QUADS);						// Start Drawing Quads
glVertex3f(-1.0f, 1.0f, 0.0f);			// Left And Up 1 Unit (Top Left)
glVertex3f( 1.0f, 1.0f, 0.0f);			// Right And Up 1 Unit (Top Right)
glVertex3f( 1.0f,-1.0f, 0.0f);			// Right And Down One Unit (Bottom Right)
glVertex3f(-1.0f,-1.0f, 0.0f);			// Left And Down One Unit (Bottom Left)
glEnd();
 */
void QuadDistrustApp::setupQuadSprites()
{
	_particleMesh = new ci::TriMesh();
	_particleMesh->clear();

	float quadSize = 100.0f;
	float count = 100;

	float quadNoiseAmount		 = 0;
#define quadNoise() (quadSize + ci::Rand::randFloat(-quadNoiseAmount, quadNoiseAmount))

	float theta 	 = M_PI * (3.0 - sqrtf(5.0));
	float o			 = 2.0f / count;
	float radius	 = 1000;
	for(int i = 0; i < count; ++i)
	{
		// Random position within radius
		ci::Vec3f pos = ci::Rand::randVec3f() * (ci::Rand::randFloat() * 500.0 + 1000);

		// Distribute quad uniformly on a sphere
		float y = i * o - 1 + (o / 2);
		float r = sqrtf(1 - y*y);
		float phi = i * theta;
		pos = ci::Vec3f( cosf(phi)*r, y, sinf(phi)*r) * radius;
		pos += ci::Rand::randVec3f() * 50;

		float rate = (float)theta / (float)count;

		ci::ColorA aColor( ci::CM_HSV, ci::Rand::randFloat() * 0.2 + 0.4, 0.7f, 0.9f, 0.9f );

		// Define v1,v2,v3,v4 by taking that position and moving outward 1 "quadSize"
		ci::Vec3f v1 = pos;
		v1.x -= quadNoise(), v1.y += quadNoise();

		ci::Vec3f v2 = pos;
		v2.x += quadNoise(), v2.y += quadNoise();

		ci::Vec3f v3 = pos;
		v3.x += quadNoise(), v3.y -= quadNoise();

		ci::Vec3f v4 = pos;
		v4.x -= quadNoise(), v4.y -= quadNoise();

		addQuadToMesh( *_particleMesh, v1, v2, v3, v4, aColor );
	}

	calculateTriMeshNormals( *_particleMesh );
}

void QuadDistrustApp::calculateTriMeshNormals( ci::TriMesh &mesh )
{
		// remove all current normals
		std::vector<ci::Vec3f>& normals = mesh.getNormals();
		const std::vector<ci::Vec3f>& vertices = mesh.getVertices();
		const std::vector<size_t>& indices = mesh.getIndices();
		normals.clear();

		// set the normal for each vertex to (0, 0, 0)
		for(size_t i=0;i< mesh.getNumVertices();++i)
			normals.push_back( ci::Vec3f::zero() );

		for(size_t i=0;i< mesh.getNumTriangles();++i)
		{
			ci::Vec3f v0 = vertices[ indices[i * 3] ];
			ci::Vec3f v1 = vertices[ indices[i * 3 + 1] ];
			ci::Vec3f v2 = vertices[ indices[i * 3 + 2] ];

			// calculate normal and normalize it, so each of the normals equally contributes to the final result
			ci::Vec3f e0 = v2 - v0;
			ci::Vec3f e1 = v2 - v1;
			ci::Vec3f n = e0.cross(e1).normalized();

			// add the normal to the final result, so we get an average of the normals of each triangle
			normals[ indices[i * 3] ] += n;
			normals[ indices[i * 3 + 1] ] += n;
			normals[ indices[i * 3 + 2] ] += n;
		}

		// the normals are probably not normalized by now, so make sure their lengths will be 1.0 as expected
		for(size_t i=0;i< normals.size();++i) {
			normals[i].normalize();
		}
}



void QuadDistrustApp::setupCamera()
{
	// Camera perspective propertie
	float cameraFOV			= 60.0f;
	float cameraNear		= 1.0f;
	float cameraFar			= 50000000.0f;


	ci::Vec3f p = ci::Vec3f::one() * 2000.0f;// Start off this far away from the center
	ci::CameraPersp cam = ci::CameraPersp( getWindowWidth(), getWindowHeight(), cameraFOV );

	cam.setWorldUp( ci::Vec3f(0, 1, 0) );
	cam.setEyePoint( ci::Vec3f(0, 0, 0 ) );
	cam.setCenterOfInterestPoint( ci::Vec3f::zero() );
	cam.setPerspective( cameraFOV, getWindowAspectRatio(), cameraNear, cameraFar );
	cam.setViewDirection( ci::Vec3f(0, 0, 1 ) );

	// Set mayacamera
	_mayaCam.setCurrentCam( cam );
}

void QuadDistrustApp::addQuadToMesh( ci::TriMesh& mesh, const ci::Vec3f& P0, const ci::Vec3f& P1, const ci::Vec3f& P2, const ci::Vec3f& P3, const ci::ColorA& color )
{
//	ci::Vec3f u = P3 - P0;
//	ci::Vec3f v = P2 - P0;
//	ci::Vec3f surfaceNormal = u.cross(v);
//	surfaceNormal.safeNormalize();
//
//	std::cout << surfaceNormal << std::endl;

	mesh.appendVertex( P0 );
	mesh.appendColorRGBA( color );
//	mesh.appendNormal( surfaceNormal );

	mesh.appendVertex( P1 );
	mesh.appendColorRGBA( color );
//	mesh.appendNormal( surfaceNormal );

	mesh.appendVertex( P2 );
	mesh.appendColorRGBA( color );
//	mesh.appendNormal( surfaceNormal );

	mesh.appendVertex( P3 );
	mesh.appendColorRGBA( color );
//	mesh.appendNormal( surfaceNormal );

	int vert0 = mesh.getNumVertices() - 4;
	int vert1 = mesh.getNumVertices() - 1;
	int vert2 = mesh.getNumVertices() - 2;
	int vert3 = mesh.getNumVertices() - 3;
	mesh.appendTriangle( vert0, vert3, vert1 );
	mesh.appendTriangle( vert3, vert2, vert1 );
}

void QuadDistrustApp::mouseDown( ci::app::MouseEvent event )
{
	_mayaCam.mouseDown( event.getPos() );
}

void QuadDistrustApp::mouseDrag( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
}

void QuadDistrustApp::mouseMove( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
}

void QuadDistrustApp::mouseUp( ci::app::MouseEvent event )
{
	_mayaCam.mouseDown( event.getPos() );
}

void QuadDistrustApp::resize( ci::app::ResizeEvent event )
{
	ci::CameraPersp cam = _mayaCam.getCamera();
	cam.setPerspective( 60,  event.getAspectRatio(), 1,  1000*10 );
	_mayaCam.setCurrentCam( cam );
}

void QuadDistrustApp::update()
{
	_cubeRotation.rotate( ci::Vec3f(1, 1, 1), 0.003f );
}

void QuadDistrustApp::draw()
{
	// clear out the window with black
	ci::gl::clear( ci::Color( 0, 0, 0 ) );
	ci::gl::enableDepthRead();
	ci::gl::setMatrices( _mayaCam.getCamera() );

//	glPushMatrix();
//		ci::gl::multModelView( _cubeRotation );
//		ci::gl::drawCube( ci::Vec3f::zero(), ci::Vec3f( 2.0f, 2.0f, 2.0f ) );
//	glPopMatrix();


	ci::gl::enableAlphaBlending();
	ci::gl::draw( *_particleMesh );


	std::vector<ci::Vec3f> meshVertices = _particleMesh->getVertices();
	std::vector<ci::Vec3f> meshNormals = _particleMesh->getNormals();
	for( size_t i = 3; i < meshNormals.size(); i+=4 )
	{
		float t = 0.5f;
		ci::Vec3f midPoint = ci::Vec3f( (1.0f-t) * (meshVertices[i-2]) + t*(meshVertices[i]) );
		ci::Vec3f normal = meshNormals[i]*10;
		ci::gl::drawVector( midPoint, midPoint+normal );
	}

//	ci::Vec3f mRight, mUp;
//	_mayaCam.getCamera().getBillboardVectors(&mRight, &mUp);
//	ci::gl::drawBillboard( ci::Vec3f::zero(), ci::Vec2f(100.0f, 100.0f), 0.0f, mRight, mUp);

//	// Draw floor plane
//	float floorSize = 2000.0f;
//	_texture.enableAndBind();
//	glBegin(GL_QUADS);
//	glTexCoord2f(0.0f,1.0f); glVertex3f(-floorSize, 0.0f, floorSize);
//	glTexCoord2f(1.0f,1.0f); glVertex3f( floorSize, 0.0f, floorSize);
//	glTexCoord2f(1.0f,0.0f); glVertex3f( floorSize, 0.0f,-floorSize);
//	glTexCoord2f(0.0f,0.0f); glVertex3f(-floorSize, 0.0f,-floorSize);
//	glEnd();
//	_texture.disable();
}

CINDER_APP_BASIC( QuadDistrustApp, ci::app::RendererGl )
