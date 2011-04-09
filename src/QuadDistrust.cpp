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
#include "ofxSimplex.h"
#include "ofxSinCosLUT.h"
#include "cinder/Perlin.h"

struct IndexedQuad
{
	size_t index;
	ci::Vec3f velocity;
	ci::Vec3f acceleration;
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
	std::vector< IndexedQuad > _indexedQuads;

	ci::gl::Light*		_light;
	ci::gl::Material*	_material;

	ci::gl::GlslProg	mShader;
	float				mAngle;
	ofxSimplex*			_simplexNoise;

	float		mouseX;
	float		mouseY;
	bool		mMOUSEDOWN;
	float		mDirectional;

	ci::Vec3f	mMouseLoc;


	// Material
	ci::ColorA	_matNone;
	ci::ColorA	_matAmbient;
	ci::ColorA	_matDiffuse;
	ci::ColorA	_matSpecular;
	ci::ColorA	_matEmission;
	float		_matShininess;
	// Material toggle
	bool		_matUseDiffuse;
	bool		_matUseAmbient;
	bool		_matUseSpecular;
	bool		_matUseEmissive;
};

void QuadDistrustApp::prepareSettings( ci::app::AppBasic::Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
}


void QuadDistrustApp::setup()
{
	_light = new ci::gl::Light( ci::gl::Light::DIRECTIONAL, 0);
//	_light->setAttenuation( 1.0, 0.0014, 0.000007); // http://www.ogre3d.org/tikiwiki/-Point+Light+Attenuation
	mDirectional = 1.0f;
	setupMaterials();
	setupQuadSprites();

	_simplexNoise = new ofxSimplex();


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
	mMouseLoc 	= ci::Vec3f::zero();
	mMOUSEDOWN	= false;

	ci::gl::enableDepthWrite();
	ci::gl::enableDepthRead();
	ci::gl::enableAlphaBlending();

	_planeMesh = new ci::TriMesh();
	_planeMesh->clear();
	ZoaDebugFunctions::createPlane( *_planeMesh, ci::Vec3f(0, -15, 0), 4000.0f, 4000.0f, 8, 8, 20 );

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

	_indexedQuads.reserve( count );

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

		// Store in index quad
		IndexedQuad iq;
		iq.index = i;
		iq.velocity = ci::Vec3f::zero();
		iq.acceleration = ci::Vec3f::zero();

		_indexedQuads.push_back( iq );
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
	_matUseDiffuse		= true;
	_matUseAmbient		= true;
	_matUseSpecular		= true;
	_matUseEmissive		= false;

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
	mMOUSEDOWN = true;
	_mayaCam.mouseDown( event.getPos() );
}

void QuadDistrustApp::mouseDrag( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
	mouseX = event.getPos().x;
	mouseY = event.getPos().y;
}

void QuadDistrustApp::mouseMove( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
	mouseX = event.getPos().x;
	mouseY = event.getPos().y;
}

void QuadDistrustApp::mouseUp( ci::app::MouseEvent event )
{
	mMOUSEDOWN = false;
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
	static bool didTest = false;
	static ofxSinCosLUT sinCosLUT;
	static ci::Perlin	perlin;
	static float mCounter = 0.0f;

	if(!didTest) {
		perlin.setSeed( clock() );
		for(float i = 0; i < TWO_PI; i+=0.01f){
			std::cout << sinCosLUT._sin( i/TWO_PI ) << std::endl << ci::math<float>::sin( i/TWO_PI ) << std::endl << std::endl;
		}
		didTest = true;
	}
	if( ! mMOUSEDOWN )
		mDirectional -= ( mDirectional - 0.985f ) * 0.1f;
	else
		mDirectional -= ( mDirectional - 0.51f ) * 0.1f;

//	_light->update( _mayaCam.getCamera() )
	float vertexCount = _particleMesh->getNumVertices();
	if( vertexCount == 0 ) return;

	// store all the mesh information
	std::vector<ci::Vec3f> &vec = _particleMesh->getVertices();

	int i, j;
	i = _particleMesh->getNumVertices();
	j = 0;

	// something to add a little movement
	float 		limit = 2000;
	ci::Vec3f	moveSpeed = ci::Vec3f(0, 0.1, 0);
	std::vector<ci::Vec3f>& normals = _particleMesh->getNormals();

	int indexQuadIterator = 0;
	float maxSpeed = 2.5f;
	float grav = 2.0f;
	float maxVel = 15.0f;
	float nZ = getElapsedSeconds() * 0.005;
	mCounter += 0.01;
	while(j < i)
	{
		IndexedQuad* iq = &_indexedQuads[indexQuadIterator];
		ci::Vec3f noisePosition = vec[j];
		noisePosition *= 0.001f;
		noisePosition.z = noisePosition.z*0.8 + mCounter;

		ci::Vec3f noise = perlin.dfBm( noisePosition  );
		//noise.normalize();
		noise *= maxSpeed;

		// Apply noise
		iq->velocity += noise;

		float velLength = iq->velocity.lengthSquared() + 0.1f;
		if( velLength > maxVel*maxVel ){
			iq->velocity.normalize();
			iq->velocity *= maxVel;
		}

		// Apply gravity
		iq->velocity.y += grav;


		moveSpeed = iq->velocity;
		vec[j] += moveSpeed;
		vec[j+1] += moveSpeed;
		vec[j+2] += moveSpeed;
		vec[j+3] += moveSpeed;

		/*
		 * 	Vec3f noise = mPerlin.dfBm( Vec3f( particleIt->mLoc[0].x, particleIt->mLoc[0].y, particleIt->mLoc[0].z ) * 0.01f + Vec3f( 0, 0, counter / 100.0f ) );
		noise.normalize();
		noise *= mMagnitude;

		// Add some perlin noise to the velocity
		Vec3f deriv = mPerlin.dfBm( Vec3f( partIt->mPosition.x, partIt->mPosition.y, mAnimationCounter ) * 0.001f );
		partIt->mZ = deriv.z;
		Vec2f deriv2( deriv.x, deriv.y );
		deriv2.normalize();
		partIt->mVelocity += deriv2 * SPEED;
	}
		 */
		iq->velocity *= 0.98f;

		if(vec[j].lengthSquared() > 2000 * 2000 * 2)
		{
			float radius = 2000;

			iq->velocity = ci::Vec3f::zero();

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
		++indexQuadIterator;
	}

	mAngle += 0.05f;
}

void QuadDistrustApp::draw()
{
	static bool movedOnce = false;


	// clear out the window with black
	ci::gl::clear( ci::Color( 0, 0, 0 ), true );
	ci::gl::disableAlphaBlending();

	ci::gl::setMatrices( _mayaCam.getCamera() );
	_light->update( _mayaCam.getCamera() );

	_light->enable();

//	_light->update( _mayaCam.getCamera() );

	if(!movedOnce)
	{
		// Move light
			float r = 2000;
			float x = ci::math<float>::cos( mAngle ) * r;
			float y = ci::math<float>::sin( getElapsedSeconds() * 0.1 ) * 500;
			float z = ci::math<float>::sin( mAngle ) * r;
//			movedOnce = true;
			_light->lookAt( ci::Vec3f(x, y, z), ci::Vec3f::zero() );
			//
			float dir = ci::math<float>::abs( ci::math<float>::sin( getElapsedSeconds() * 0.5 ) ) * 0.989f + 0.1f;
//			std::cout << dir << std::endl;
			GLfloat light_position[] = { x, y, z, dir };
			glLightfv( GL_LIGHT0, GL_POSITION, light_position );
	}



	// BEGIN SHADER
	mShader.bind();
	mShader.uniform( "NumEnabledLights", 1 );
		ci::gl::draw( *_particleMesh );
		float cubeSize = 25;
		ci::gl::drawCube( ci::Vec3f::zero(), ci::Vec3f(cubeSize, cubeSize, cubeSize) );
	mShader.unbind();
	// END SHADER


	_light->disable();
	glColor3f( 1.0f, 1.0f, 0.1f );
//	ci::gl::drawFrustum( _light->getShadowCamera() );
	// Draw floor
	ci::gl::enableWireframe();
	ci::gl::draw( *_planeMesh );
	ci::gl::disableWireframe();
}

CINDER_APP_BASIC( QuadDistrustApp, ci::app::RendererGl(1) )
