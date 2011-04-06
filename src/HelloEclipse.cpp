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
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/app/Renderer.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/TriMesh.h"
#include "cinder/Rand.h"
#include "cinder/MayaCamUI.h"

class HelloWorldApp : public ci::app::AppBasic {
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

	// Draw a cube (
	ci::Matrix44f	_cubeRotation;
	ci::gl::Texture	_texture;
	ci::MayaCamUI	_mayaCam;
	ci::TriMesh*	_particleMesh;
};

void HelloWorldApp::prepareSettings( ci::app::AppBasic::Settings *settings )
{
	settings->setWindowSize( 800, 600 );
}

void HelloWorldApp::setup()
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
void HelloWorldApp::setupQuadSprites()
{
	_particleMesh = new ci::TriMesh();
	_particleMesh->clear();

	float quadSize = 30.0f;
	float count = 10000;

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

		float rate = (float)theta / (float)count;

//		std::cout << rate << std::endl;
		ci::ColorA aColor( ci::CM_HSV, fabs(y*0.4)+0.6, 1.0f, 0.5f, 1.0f );

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
}

void HelloWorldApp::setupCamera()
{
	// Camera perspective propertie
	float cameraFOV			= 65.0f;
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

void HelloWorldApp::addQuadToMesh( ci::TriMesh& mesh, const ci::Vec3f& P0, const ci::Vec3f& P1, const ci::Vec3f& P2, const ci::Vec3f& P3, const ci::ColorA& color )
{
	mesh.appendVertex( P0 );
	mesh.appendColorRGBA( color );
	mesh.appendVertex( P1 );
	mesh.appendColorRGBA( color );
	mesh.appendVertex( P2 );
	mesh.appendColorRGBA( color );
	mesh.appendVertex( P3 );
	mesh.appendColorRGBA( color );

	int vert0 = mesh.getNumVertices() - 4;
	int vert1 = mesh.getNumVertices() - 1;
	int vert2 = mesh.getNumVertices() - 2;
	int vert3 = mesh.getNumVertices() - 3;
	mesh.appendTriangle( vert0, vert1, vert3 );
	mesh.appendTriangle( vert3, vert1, vert2 );
}

void HelloWorldApp::mouseDown( ci::app::MouseEvent event )
{
	_mayaCam.mouseDown( event.getPos() );
}

void HelloWorldApp::mouseDrag( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
}

void HelloWorldApp::mouseMove( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
}

void HelloWorldApp::mouseUp( ci::app::MouseEvent event )
{
	_mayaCam.mouseDown( event.getPos() );
}

void HelloWorldApp::resize( ci::app::ResizeEvent event )
{
	ci::CameraPersp cam = _mayaCam.getCamera();
	cam.setPerspective( 60,  event.getAspectRatio(), 1,  1000*10 );
	_mayaCam.setCurrentCam( cam );
}

void HelloWorldApp::update()
{
	_cubeRotation.rotate( ci::Vec3f(1, 1, 1), 0.003f );
}

void HelloWorldApp::draw()
{
	// clear out the window with black
	ci::gl::clear( ci::Color( 0, 0, 0 ) );
	ci::gl::enableDepthRead();
	ci::gl::setMatrices( _mayaCam.getCamera() );

	glPushMatrix();
		ci::gl::multModelView( _cubeRotation );
		ci::gl::drawCube( ci::Vec3f::zero(), ci::Vec3f( 2.0f, 2.0f, 2.0f ) );
	glPopMatrix();


	ci::gl::enableAlphaBlending();
	ci::gl::draw( *_particleMesh );

//	ci::Vec3f mRight, mUp;
//	_mayaCam.getCamera().getBillboardVectors(&mRight, &mUp);
//	ci::gl::drawBillboard( ci::Vec3f::zero(), ci::Vec2f(100.0f, 100.0f), 0.0f, mRight, mUp);

	// Draw floor plane
	float floorSize = 2000.0f;
	_texture.enableAndBind();
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,1.0f); glVertex3f(-floorSize, 0.0f, floorSize);
	glTexCoord2f(1.0f,1.0f); glVertex3f( floorSize, 0.0f, floorSize);
	glTexCoord2f(1.0f,0.0f); glVertex3f( floorSize, 0.0f,-floorSize);
	glTexCoord2f(0.0f,0.0f); glVertex3f(-floorSize, 0.0f,-floorSize);
	glEnd();
	_texture.disable();
}

CINDER_APP_BASIC( HelloWorldApp, ci::app::RendererGl )
