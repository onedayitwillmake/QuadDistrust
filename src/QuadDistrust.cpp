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
#include "cinder/gl/Material.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Light.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include <float.h>
#include <bitset>
#include "ZoaDebugFunctions.h"
#include "Resources.h"
#include "gl.h"

struct MeshQuad
{
	size_t index;
	ci::Vec3f velocity;
};

class QuadDistrustApp : public ci::app::AppBasic {
public:
	void 	prepareSettings( ci::app::AppBasic::Settings *settings );
	void 	setup();
	void 	setupCamera();
	void 	setupQuadSprites();
	void	setupMaterials();

	void	resize( ci::app::ResizeEvent event );
	void	mouseDown( ci::app::MouseEvent event );
	void	mouseMove( ci::app::MouseEvent event );
	void	mouseDrag( ci::app::MouseEvent event );
	void	mouseUp( ci::app::MouseEvent event );
	void 	keyDown( ci::app::KeyEvent event );

	void 	update();
	void 	draw();

	//
	ci::gl::Texture		_texture;
	ci::MayaCamUI		_mayaCam;
	ci::TriMesh*		_particleMesh;
	ci::TriMesh*		_planeMesh;

	ci::gl::Light*		_light;
	ci::gl::Material*	_material;

	ci::gl::GlslProg	mShader;
	float				mAngle;

	float		mCounter;
	float		mouseX;
	float		mouseY;
	bool		mMOUSEDOWN;

	ci::Vec3f	mMouseLoc;

	bool		_matUseDiffuse;
	bool		_matUseAmbient;
	bool		_matUseSpecular;
	bool		_matUseEmissive;
	bool		mSHADER;
	float		mDirectional;

	ci::ColorA	_matNone;
	ci::ColorA	_matAmbient;
	ci::ColorA	_matDiffuse;
	ci::ColorA	_matSpecular;
	ci::ColorA	_matEmission;
	float		_matShininess;
};

void QuadDistrustApp::prepareSettings( ci::app::AppBasic::Settings *settings )
{
	settings->setWindowSize( 800, 600 );
}


void QuadDistrustApp::setup()
{
	_light = new ci::gl::Light( ci::gl::Light::DIRECTIONAL, 0);
	_light->setAttenuation( 1.0, 0.0014, 0.000007); // http://www.ogre3d.org/tikiwiki/-Point+Light+Attenuation

	setupMaterials();
	setupQuadSprites();


	try {
		mShader = ci::gl::GlslProg( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_BLUR_FRAG ) );
	}
	catch( ci::gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	} catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}

//	// GL Stuff
	mCounter		= 0.0f;
	mMouseLoc = ci::Vec3f::zero();


	mMOUSEDOWN		= false;

	_matUseDiffuse		= true;
	_matUseAmbient		= true;
	_matUseSpecular		= true;
	mSHADER			= true;
	_matUseEmissive		= false;

	mDirectional	= 1.0f;


	mMouseLoc = ci::Vec3f::zero();


	ci::gl::enableDepthWrite();
	ci::gl::enableDepthRead();
	ci::gl::enableAlphaBlending();

	_planeMesh = new ci::TriMesh();
	_planeMesh->clear();
	ZoaDebugFunctions::createPlane( *_planeMesh, ci::Vec3f(0, -15, 0), 4000.0f, 4000.0f, 8, 8 );

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

	float count = 10000;

	float theta 	 = M_PI * (3.0 - sqrtf(5.0));
	float o			 = 2.0f / count;
	float radius	 = 1000;
	for(int i = 0; i < count; ++i)
	{
		// Random position within radius
		ci::Vec3f pos = ci::Rand::randVec3f();

		// Distribute quad uniformly on a sphere
		float y = i * o - 1 + (o / 2);
		float r = sqrtf(1 - y*y);
		float phi = i * theta;
		pos = ci::Vec3f( cosf(phi)*r, y, sinf(phi)*r) * radius;
		pos += ci::Rand::randVec3f() * 50;
		pos.y = 10000;

		ci::ColorA aColor( ci::CM_HSV, ci::Rand::randFloat() * 0.2 + 0.4, 0.7f, 0.9f, 0.9f );
		float angle = ci::Rand::randFloat( M_PI );

		ci::Vec3f v1; ci::Vec3f v2; ci::Vec3f v3; ci::Vec3f v4;
		ZoaDebugFunctions::createQuadAtPosition( pos, v1, v2, v3, v4, 50, 5, angle );
		ZoaDebugFunctions::addQuadToMesh( *_particleMesh, v1, v2, v3, v4, aColor );
	}

//	calculateTriMeshNormals( *_particleMesh );
}

void QuadDistrustApp::setupCamera()
{
	// Camera perspective properties
	float cameraFOV			= 60.0f;
	float cameraNear		= 1.0f;
	float cameraFar			= FLT_MAX;

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

void QuadDistrustApp::setupMaterials()
{
	_matNone			= ci::ColorA( 0.0f, 0.0f, 0.0f, 1.0f );
	_matAmbient			= ci::ColorA( 0.1f, 0.1f, 0.1f, 1.0f );
	_matDiffuse			= ci::ColorA( 0.5f, 0.5f, 0.5f, 1.0f );
	_matSpecular		= ci::ColorA( 1.0f, 1.0f, 1.0f, 1.0f );
	_matEmission		= ci::ColorA( 0.1f,0.01f, 0.1f, 1.0f );
	_matShininess		= 64.0f;

	_material = new ci::gl::Material( _matAmbient, _matDiffuse, _matSpecular, _matShininess, _matEmission, GL_FRONT );
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

void QuadDistrustApp::keyDown( ci::app::KeyEvent event )
{
	/**
	 * Toggle material properties
	 */
	bool shouldUpdateMaterial = false;
	if( event.getChar() == 'd' || event.getChar() == 'D' ){
		_matUseDiffuse = ! _matUseDiffuse;
		_material->setDiffuse( _matUseDiffuse ? _matDiffuse : _matNone );
		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == 'a' || event.getChar() == 'A' ){
		_matUseAmbient = ! _matUseAmbient;
		_material->setAmbient( _matUseAmbient ? _matAmbient : _matNone );
		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == 's' || event.getChar() == 'S' ){
		_matUseSpecular = ! _matUseSpecular;
		_material->setSpecular( _matUseSpecular ? _matSpecular : _matNone );
		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == 'e' || event.getChar() == 'E' ){
		_matUseEmissive = ! _matUseEmissive;
		_material->setEmission( _matUseEmissive ? _matEmission : _matNone );
		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == ',' || event.getChar() == '<' ){
		_matShininess *= 0.5f;
		if( _matShininess < 1.0f ) // Cap
			_matShininess = 1.0f;
		_material->setShininess( _matShininess );

		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == '.' || event.getChar() == '>' ){
		_matShininess *= 2.0f;
		if( _matShininess > 128.0f ) // Cap
			_matShininess = 128.0f;
		_material->setShininess( _matShininess );

		shouldUpdateMaterial = true;
	}


	// Screenshot
	if( event.getChar() == 'p' || event.getChar() == 'p' ) {
		std::ostringstream stringBuffer;
		stringBuffer << clock();
		std::string aTimeString = stringBuffer.str();
		ci::writeImage( ci::getHomeDirectory() + "QuadDistrust/" + aTimeString + ".png", copyWindowSurface() );
	}

	// Update material if it was changed
	if(shouldUpdateMaterial)
		_material->apply();


}


void QuadDistrustApp::resize( ci::app::ResizeEvent event )
{
	ci::CameraPersp cam = _mayaCam.getCamera();
	cam.setPerspective( 60,  event.getAspectRatio(), 1, 6000);
	_mayaCam.setCurrentCam( cam );
}

void QuadDistrustApp::update()
{
//	if( ! mMOUSEDOWN )
//		mDirectional -= ( mDirectional - 0.985f ) * 0.1f;
//	else
//		mDirectional -= ( mDirectional - 0.51f ) * 0.1f;

//	_light->update( _mayaCam.getCamera() )
	float vertexCount = _particleMesh->getNumVertices();
	if( vertexCount == 0 ) return;

	// store all the mesh information
	std::vector<ci::Vec3f> &vec = _particleMesh->getVertices();

	int i, j;
	i = _particleMesh->getNumVertices();
	j = 0;

	// something to add a little movement
	float 		limit = 1000;
	ci::Vec3f	moveSpeed = ci::Vec3f(0, 0, 0);
	std::vector<ci::Vec3f>& normals = _particleMesh->getNormals();

	while(j < i)
	{
		vec[j] += moveSpeed;
		vec[j+1] += moveSpeed;
		vec[j+2] += moveSpeed;
		vec[j+3] += moveSpeed;

		if(vec[j].y > limit )
		{
			float radius = 2000;

			// (if X and Z use the same angle we will create a cylinder
			float xAngle = ci::Rand::randFloat( (float)M_PI * 2.0f );
			float zAngle = ci::Rand::randFloat( (float)M_PI * 2.0f );
			float rotAngle = ci::Rand::randFloat( (float)M_PI );

			float x = ci::math<float>::cos( xAngle ) * radius;
			float y = ci::Rand::randFloat() * 600;
			float z = ci::math<float>::sin( zAngle ) * radius;
			ci::Vec3f pos = ci::Vec3f( x, y, z );

			// Modify quad vertices to place at position
			ZoaDebugFunctions::createQuadAtPosition( pos, vec[j], vec[j+1], vec[j+2], vec[j+3], 4, 0.5, rotAngle );

			// Fix normal for new quad position
			ci::Vec3f e0 = vec[j+2] - vec[j];
			ci::Vec3f e1 = vec[j+2] - vec[j+1];
			ci::Vec3f n = e0.cross(e1).normalized();
			normals[j] = n;
			normals[j+1] = n;
			normals[j+2] = n;
			normals[j+3] = n;
		}

		// go to the next triangle pair
		j+=4;
	}

	mAngle += 0.05f;;
}

void QuadDistrustApp::draw()
{
	// clear out the window with black
	ci::gl::clear( ci::Color( 0, 0, 0 ) );
	ci::gl::setMatrices( _mayaCam.getCamera() );

//	glEnable( GL_LIGHTING );
//	glEnable( GL_LIGHT0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	mMouseLoc -= ( mMouseLoc - ci::Vec3f( mouseX, mouseY, 500.0f ) ) * 0.2f;

//	ci::Vec3f camPos = _mayaCam.getCamera().getEyePoint();
//	ci::Vec3f lightOffset = ci::Vec3f(200, 0, 0);
//	lightOffset
//
//	float moveX = ci::math<float>::sin( getElapsedSeconds() * 0.5 );
//	GLfloat light_position[] = { camPos.x + moveX*3000, camPos.y, camPos.z, mDirectional };
//	glLightfv( GL_LIGHT0, GL_POSITION, light_position );

	//
//	GLfloat light_Kd[] = { 0.5f, 0.5f, 0.5f, 1.0 };
//	glLightfv( GL_LIGHT0, GL_DIFFUSE, light_Kd);
//	glLightf( GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5f );
//	glLightf( GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f );
//	glLightf( GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.00015f );

//	ci::Vec3f dirFromLight	= ci::Rand::randVec3f() * 1000;
//	float distFromLight	= dirFromLight.length();

//	if( mDIFFUSE ){
//		ci::ColorA color( 1.0f, 1.0f, 1.0f, 1.0f );
//		glMaterialfv( GL_FRONT, GL_DIFFUSE,	color );
//	} else {
//		glMaterialfv( GL_FRONT, GL_DIFFUSE,	no_mat );
//	}


//	if( mAMBIENT ){
//		glMaterialfv( GL_FRONT, GL_AMBIENT,	mat_ambient );
//	} else {
//		glMaterialfv( GL_FRONT, GL_AMBIENT,	no_mat );
//	}
//
//
//	if( mSPECULAR ){
//		glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );
//		glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess );
//	} else {
//		glMaterialfv( GL_FRONT, GL_SPECULAR, no_mat );
//		glMaterialfv( GL_FRONT, GL_SHININESS, no_shininess );
//	}
//
//
//	if( mEMISSIVE ){
//		glMaterialfv( GL_FRONT, GL_EMISSION, mat_emission );
//	} else {
//		glMaterialfv( GL_FRONT, GL_EMISSION, no_mat );
//	}

	GLfloat light_Kd[] = { 0.5f, 0.5f, 0.5f, 1.0 };
	glLightfv( GL_LIGHT0, GL_DIFFUSE, light_Kd);
	_light->enable();

	// BEGIN SHADER
	if( mSHADER ){
		mShader.bind();
		mShader.uniform( "NumEnabledLights", 1 );
	}

//	ci::gl::enableAlphaBlending();
	ci::gl::draw( *_particleMesh );

	float cubeSize = 25;
	ci::gl::drawCube( ci::Vec3f::zero(), ci::Vec3f(cubeSize, cubeSize, cubeSize) );

	// END SHADER
	if(mSHADER)
		mShader.unbind();

	// Draw floor
	ci::gl::enableWireframe();
	ci::gl::draw( *_planeMesh );
	ci::gl::disableWireframe();
}

CINDER_APP_BASIC( QuadDistrustApp, ci::app::RendererGl(1) )
