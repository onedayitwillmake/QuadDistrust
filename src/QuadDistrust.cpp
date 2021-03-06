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
// System
#include <float.h>
#include <bitset>
#include <stdlib.h>

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
// App
#include "ZoaDebugFunctions.h"
#include "Resources.h"
#include "gl.h"
#include "ofxSimplex.h"
#include "ofxSinCosLUT.h"
#include "cinder/Perlin.h"
#include "SimpleGUI/include/SimpleGUI.h"

#define __USE_KINECT 1
#ifdef  __USE_KINECT
#include "CinderOpenNI.h"
#endif

#define distance2(a,b,c) {\
  float tmp;\
  c = a[0] - b[0];\
  c = c * c;\
  tmp = a[1] - b[1];\
  c += tmp * tmp;\
  tmp = a[2] - b[2];\
  c += tmp * tmp;\
}

#define quadRandomLifespan ci::Rand::randFloat( 400 * 0.75, 400)
#define distance(a,b,c) { float d; distance2(a,b,d); c = (float)sqrt((double)d); }
#define maxForceCount 4

struct IndexedQuad
{
	size_t		index;
	float 		skyLimit;
	float 		mass;
	float		inverseMass;
	ci::Vec3f	position;
	ci::Vec3f	velocity;
	ci::Vec3f	acceleration;

	float		age;
	float		lifespan;
	float 		agePer;
};

struct Force
{
	bool		isActive;
	float		textureScale;
	float		charge;
	ci::Vec3f	targetPosition;
	ci::Vec3f	position;
};

class QuadDistrustApp : public ci::app::AppBasic {
public:
	void 		prepareSettings( ci::app::AppBasic::Settings *settings );
	void 		setup();
	void 		setupCamera();
	void 		setupQuadSprites();
	void		setupMaterials();
	void	 	setupShader();
	void		setupKinect();
	void		setupGui();

	// Application updates
	void		resize( ci::app::ResizeEvent event );
	void		mouseDown( ci::app::MouseEvent event );
	void		mouseMove( ci::app::MouseEvent event );
	void		mouseDrag( ci::app::MouseEvent event );
	void		mouseUp( ci::app::MouseEvent event );
	void 		keyDown( ci::app::KeyEvent event );

	void 		update();
	void 		draw();

	// Camera
	bool		shouldAutoRotate;
	float		autoRotationSpeed;
	ci::Vec2f	autoRotationMouse;
	void 		startAutoRotation();
	void 		stopAutoRotation();
	void		handleAutoRotation();

	// Kinect
	void 		drawKinectDepth();
	ci::Rectf	getKinectDepthArea( int width, int height );

	// Scene
	ci::MayaCamUI					_mayaCam;
	ci::TriMesh*					_particleMesh;
	ci::TriMesh*					_planeMesh;

	// Particle properties
	std::vector< Force >			_forces;
	float 							_forceStrength;
	float 							_forceMinDist;
	float 							_forceMaxDist;
	float 							maxSpeed;
	float 							grav;
	float							maxVel;

	// Scene properties
	std::vector< IndexedQuad >		 _indexedQuads;
	float 							_quadMaxSize;
	float							_quadMaxDistortion;
	ci::gl::Light*					_light;
	float							mDirectional;

	ci::gl::Material*				_material;
	ci::gl::Texture						_textureParticle;

	ci::gl::GlslProg				_shader;
	ofxSimplex*						_simplexNoise;

	// Mouse
	bool							_mouseIsDown;
	ci::Vec2f						_mousePosition;

	// Tick
	float							mAngle;

	bool							shouldApplyForces;
	bool							shouldApplyNoise;
	bool							shouldDrawSkeleton;
	bool							shouldDrawSimpleGui;

	// GUI
	mowa::sgui::SimpleGUI	*GUI;

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

uint_fast32_t approx_distance( int_fast32_t dx, int_fast32_t dy )
{
	uint_fast32_t min, max, approx;

   if ( dx < 0 ) dx = -dx;
   if ( dy < 0 ) dy = -dy;

   if ( dx < dy )
   {
      min = dx;
      max = dy;
   } else {
      min = dy;
      max = dx;
   }

   approx = ( max * 1007 ) + ( min * 441 );
   if ( max < ( min << 4 ))
      approx -= ( max * 40 );

   // add 512 for proper rounding
   return (( approx + 512 ) >> 10 );
}

void QuadDistrustApp::prepareSettings( ci::app::AppBasic::Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
}

#define defaultTextureScale 3100.0f

void QuadDistrustApp::setup()
{
//	std::cout << getcwd(temp, maxPathLenth) << std::endl;
	_light = new ci::gl::Light( ci::gl::Light::DIRECTIONAL, 0);
//	_light->setAttenuation( 1.0, 0.0014, 0.000007); // http://www.ogre3d.org/tikiwiki/-Point+Light+Attenuation

////// SETUP FORCE PROPERTIES
#ifdef __USE_KINECT
	_forces.resize( CINDERSKELETON->maxUsers * 2 );
	for(int i = 0; i < _forces.size(); i++ ) {
		_forces[i].position = ci::Vec3f::zero();
		_forces[i].targetPosition = ci::Vec3f::zero();
	}
#else
	_forces.resize( maxForceCount );
	for(int i = 0; i < maxForceCount; i++) {
		ci::Vec3f pos = ci::Rand::randVec3f() * 2000;
		pos.y = fabs(pos.y);
		_forces[i].isActive = true;
		_forces[i].position = pos;
		_forces[i].textureScale = defaultTextureScale;
		_forces[i].targetPosition = pos;
//		_forces[i].
	}
#endif


	_forceStrength = 0.15f;
	_forceMinDist = 200.0f;
	_forceMaxDist = 3500.0f;

	mDirectional = 1.0f;
	setupMaterials();
	setupQuadSprites();
	setupKinect();

	ci::Rand::randSeed( clock() );
	_simplexNoise = new ofxSimplex();

	setupShader();

	// Mouse
	_mousePosition 	= ci::Vec2f::zero();
	_mouseIsDown	= false;


	// Create floor plane
	_planeMesh = new ci::TriMesh();
	_planeMesh->clear();
	ZoaDebugFunctions::createPlane( *_planeMesh, ci::Vec3f(0, -15, 0), 4000.0f, 4000.0f, 8, 8, 40 );

	// Setup OpenGL
	ci::gl::enableDepthWrite();
	ci::gl::enableDepthRead();
	ci::gl::enableAlphaBlending();

	_textureParticle	= ci::gl::Texture( loadImage( loadResource( RES_PARTICLE ) ) );

	shouldApplyForces = true;
	shouldDrawSkeleton = true;

	setupGui();
//	getFocus();
}

void QuadDistrustApp::setupShader()
{
	try {
		_shader = ci::gl::GlslProg( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_BLUR_FRAG ) );
	}
	catch( ci::gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	} catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}
}

void QuadDistrustApp::setupGui()
{
	shouldDrawSimpleGui = true;

	// create
	GUI = new mowa::sgui::SimpleGUI(this);

	// Intialiaze
	GUI->textColor = ColorA(1,1,1,1);
	GUI->lightColor = ColorA(1, 0, 1, 1);
	GUI->darkColor = ColorA(0.05,0.05,0.05, 1);
	GUI->bgColor = ColorA(0.15, 0.15, 0.15, 1.0);
	GUI->addColumn();
//
//	GUI->addSeparator();
#ifdef  __USE_KINECT
	float tRange = 2000;		// Range of translate calls
	GUI->addLabel("OpenNI");
	GUI->addParam("translateX", &CINDERSKELETON->worldOffset.x, -tRange, tRange, CINDERSKELETON->worldOffset.x);
	GUI->addParam("translateY", &CINDERSKELETON->worldOffset.y, -tRange, tRange, CINDERSKELETON->worldOffset.y);
	GUI->addParam("translateZ", &CINDERSKELETON->worldOffset.z, -3000, 1000, CINDERSKELETON->worldOffset.z);
#endif

	GUI->addColumn();		// Place in own column
	GUI->addLabel("Particle");
	GUI->addParam("Size", &_quadMaxSize, 1, _quadMaxSize*2, _quadMaxSize);
	GUI->addParam("distortion", &_quadMaxDistortion, 0, _quadMaxDistortion*2, _quadMaxDistortion);
	GUI->addParam("maxSpeed", &maxSpeed, 0.1, 10.0f, maxSpeed);
	GUI->addParam("maxVelocity", &maxVel, 2, maxVel*2, maxVel);

	GUI->addLabel("Force");
	GUI->addParam("Strength", &_forceStrength, 0.1f, _forceStrength*3, _forceStrength);
	GUI->addParam("MinDistance", &_forceMinDist, 1, _forceMinDist*3, _forceMinDist);
	GUI->addParam("MaxDistance", &_forceMaxDist, 100, _forceMaxDist, _forceMaxDist);
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
	_quadMaxSize = 4.0f;
	_quadMaxDistortion = 1.0f;

	maxSpeed = 1.0f;
	grav = 0.04;
	maxVel = 15.0f;

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
		//pos.y = 10000;

		ci::ColorA aColor( ci::CM_HSV, ci::Rand::randFloat() * 0.2 + 0.4, 0.7f, 0.9f, 0.9f );
		float angle = ci::Rand::randFloat( M_PI );

		ci::Vec3f v1; ci::Vec3f v2; ci::Vec3f v3; ci::Vec3f v4;
		ZoaDebugFunctions::createQuadAtPosition( pos, v1, v2, v3, v4, _quadMaxSize, _quadMaxDistortion, angle );
		ZoaDebugFunctions::addQuadToMesh( *_particleMesh, v1, v2, v3, v4, aColor );

		// Store in index quad
		IndexedQuad iq;
		iq.index = i;
		iq.velocity = ci::Vec3f::zero();
		iq.acceleration = ci::Vec3f::zero();
		iq.skyLimit = 2000 + ci::Rand::randFloat(1000);
		iq.mass = ci::Rand::randFloat(0.5, 2.0);
		iq.inverseMass = 1.0f / iq.mass;
		iq.age = 1.0f;
		iq.lifespan = quadRandomLifespan;
		_indexedQuads.push_back( iq );
	}

//	calculateTriMeshNormals( *_particleMesh );
}

void QuadDistrustApp::setupKinect()
{
#ifdef  __USE_KINECT
	// For now we have to manually change to the application path. Bug?
	chdir( getAppPath().c_str() );

    int     maxPathLenth = 255;
    char    temp[maxPathLenth];
    std::string cwd = ( getcwd(temp, maxPathLenth) ? std::string( temp ) : std::string("") );

    std::cout << "CurrentWorkingDirectory is:" << cwd << std::endl;
    std::cout << "AppPath: " << this->getAppPath() << std::endl;
	bool useRecording = false;

	XnStatus nRetVal = XN_STATUS_OK;
	CinderOpenNISkeleton *skeleton = CINDERSKELETON;

	// shared setup
	skeleton->setup( );


	if(useRecording) {
		nRetVal = skeleton->mContext.OpenFileRecording("Contents/Resources/SkeletonRec.oni");
		// File opened
		CHECK_RC(nRetVal, "B-Open File Recording", true);

		// Get recording 'player'
		nRetVal = skeleton->mContext.FindExistingNode(XN_NODE_TYPE_PLAYER, skeleton->mPlayer);
		CHECK_RC(nRetVal, "Find player generator", true);
	} else {
		skeleton->setupFromXML( "Contents/Resources/configIR.xml" );;
	}

	// Output device production nodes (user, depth, etc)
	skeleton->debugOutputNodeTypes();

	// Find depth generator
	nRetVal = skeleton->mContext.FindExistingNode(XN_NODE_TYPE_DEPTH, skeleton->mDepthGenerator);
	CHECK_RC(nRetVal, "Find depth generator", true);


	// Find skeleton / user generator
	nRetVal = skeleton->mContext.FindExistingNode(XN_NODE_TYPE_USER, skeleton->mUserGenerator);
	if (nRetVal != XN_STATUS_OK) {
		// Create one
		nRetVal = skeleton->mUserGenerator.Create(skeleton->mContext);
		CHECK_RC(nRetVal, "Find user generator", false);
	}

	// Check if user generator can detect skeleton
	if (!skeleton->mUserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
		app::console() << "Supplied user generator doesn't support skeleton\n" << endl;
	}

	// Register callbacks
	nRetVal = skeleton->setupCallbacks();

	// Start generating
	nRetVal = skeleton->mContext.StartGeneratingAll();
	CHECK_RC(nRetVal, "StartGenerating", true);

	skeleton->shouldStartUpdating();
#endif

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

	autoRotationSpeed = 5.0f;

	// Set mayacamera
	_mayaCam.setCurrentCam( cam );
}

void QuadDistrustApp::setupMaterials()
{
	_matUseDiffuse		= true;
	_matUseAmbient		= true;
	_matUseSpecular		= true;
	_matUseEmissive		= true;



	_matNone			= ci::ColorA( 0.0f, 0.0f, 0.0f, 1.0f );
	_matAmbient			= ci::ColorA( 0.1f, 0.3f, 0.3f, 1.0f );
	_matDiffuse			= ci::ColorA( 0.3f, 0.1f, 0.4f, 1.0f );
	_matSpecular		= ci::ColorA( 1.0f, 1.0f, 1.0f, 1.0f );
	_matEmission		= ci::ColorA( 0.4f/4, 0.7f/4, 1.0f/4, 1.0f );
	_matShininess		= 64.0f;

	_material = new ci::gl::Material( _matAmbient, _matDiffuse, _matSpecular, _matShininess, _matEmission, GL_FRONT_AND_BACK);
	_material->apply();
}


void QuadDistrustApp::mouseDown( ci::app::MouseEvent event )
{
	_mouseIsDown = true;
	_mayaCam.mouseDown( event.getPos() );
}

void QuadDistrustApp::mouseDrag( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
	_mousePosition = event.getPos();
}

void QuadDistrustApp::mouseMove( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
	_mousePosition = event.getPos();
}

void QuadDistrustApp::mouseUp( ci::app::MouseEvent event )
{
	_mouseIsDown = false;
	_mayaCam.mouseDown( event.getPos() );
}

void QuadDistrustApp::drawKinectDepth()
{
#ifdef  __USE_KINECT
	CinderOpenNISkeleton *skeleton = CINDERSKELETON;

	// Get surface
	Surface8u depthSurface = skeleton->getDepthSurface();

	// Not ready yet
	if( depthSurface == NULL ) {
		return;
	}

	// Convert to texture
	ci::Rectf depthArea = getKinectDepthArea( 320/2, 240/2 );
	gl::draw( gl::Texture( depthSurface ), depthArea );

	// Debug draw
	skeleton->debugDrawLabels( Font( "Helvetica", 10 ), depthArea );
#endif

}

void QuadDistrustApp::keyDown( ci::app::KeyEvent event )
{
	/**
	 * Toggle material properties
	 */
	bool shouldUpdateMaterial = false;
	if( event.getChar() == 'd' || event.getChar() == 'D' ){							// Material Diffuse on/off
		_matUseDiffuse = ! _matUseDiffuse;
		_material->setDiffuse( _matUseDiffuse ? _matDiffuse : _matNone );
		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == 'a' || event.getChar() == 'A' ){					// Material Ambient on/off
		_matUseAmbient = ! _matUseAmbient;
		_material->setAmbient( _matUseAmbient ? _matAmbient : _matNone );
		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == 's' || event.getChar() == 'S' ){					// Material Specular on/off
		_matUseSpecular = ! _matUseSpecular;
		_material->setSpecular( _matUseSpecular ? _matSpecular : _matNone );
		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == 'e' || event.getChar() == 'E' ){					// Material Emissive on/off
		_matUseEmissive = ! _matUseEmissive;
		_material->setEmission( _matUseEmissive ? _matEmission : _matNone );
		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == ',' || event.getChar() == '<' ){					// Material Shininess +
		_matShininess *= 0.5f;
		if( _matShininess < 1.0f ) // Cap
			_matShininess = 1.0f;
		_material->setShininess( _matShininess );

		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == '.' || event.getChar() == '>' ){					// Material Shininess -
		_matShininess *= 2.0f;
		if( _matShininess > 128.0f ) // Cap
			_matShininess = 128.0f;
		_material->setShininess( _matShininess );

		shouldUpdateMaterial = true;
	}
	else if( event.getChar() == 'r' || event.getChar() == 'R' ){					// Autorotation
		if(shouldAutoRotate) stopAutoRotation();
		else startAutoRotation();
	}
	else if( event.getChar() == 'g' || event.getChar() == 'G' ){					// Autorotation
		shouldApplyForces = !shouldApplyForces;
	}
	else if( event.getChar() == 'N' ){					// Autorotation
		shouldApplyNoise =  !shouldApplyNoise;
	}
	else if( event.getChar() == 'b' ){					// Autorotation
		for(int i = 0; i < _forces.size(); ++i ) {
			if(ci::Rand::randFloat() < 0.5) {
				_forces[i].textureScale = ci::Rand::randFloat(defaultTextureScale*0.5, defaultTextureScale*3 );
			}
		}

	} else if( event.getChar() == 'F' ){											// Fullscreen
		setFullScreen( ! isFullScreen() );
	} else if ( event.getChar() == '0' ) {
		shouldDrawSimpleGui = !shouldDrawSimpleGui;
	}

	if ( event.getChar() == '1' ) _forces.at( 0 ).isActive = !_forces.at( 0 ).isActive;
	else if ( event.getChar() == '2' ) _forces.at( 1 ).isActive = !_forces.at( 1 ).isActive;
	else if ( event.getChar() == '3' ) _forces.at( 2 ).isActive = !_forces.at( 2 ).isActive;
	else if ( event.getChar() == '4' ) _forces.at( 3 ).isActive = !_forces.at( 3 ).isActive;


	float moveRange = 2000.0f;
#define randomTarget(__X__) { ci::Vec3f pos = ci::Rand::randVec3f() * moveRange; pos.y = fabs(pos.y); _forces.at(__X__).targetPosition = pos; }
	if ( event.getChar() == '5' ) randomTarget(0);
	if ( event.getChar() == '6' ) randomTarget(1);
	if ( event.getChar() == '7' ) randomTarget(2);
	if ( event.getChar() == '8' ) randomTarget(3);

//	else if ( event.getChar() == '6' ) _forces.at( 1 ).isActive = !_forces.at( 1 ).isActive;
//	else if ( event.getChar() == '7' ) _forces.at( 2 ).isActive = !_forces.at( 2 ).isActive;
//	else if ( event.getChar() == '8' ) _forces.at( 3 ).isActive = !_forces.at( 3 ).isActive;

// Toggle forces
	// Screenshot
	if( event.getChar() == 'P' ) {
		std::ostringstream stringBuffer;
		stringBuffer << clock();
		std::string aTimeString = stringBuffer.str();
		ci::writeImage( ci::getHomeDirectory() + "QuadDistrust/" + aTimeString + ".png", copyWindowSurface() );
	}

	// Update material if it was changed
	if(shouldUpdateMaterial)
		_material->apply();
}


void QuadDistrustApp::startAutoRotation() {
	shouldAutoRotate = true;
	autoRotationMouse = ci::Vec2f(0, 0);
	_mayaCam.mouseDown( autoRotationMouse );
}

void QuadDistrustApp::stopAutoRotation() {
	shouldAutoRotate = false;
}

void QuadDistrustApp::handleAutoRotation() {
	if(!shouldAutoRotate)
		return;

	autoRotationMouse.x -= 0.5f;
	_mayaCam.mouseDrag( autoRotationMouse, true, false, false );
}
void QuadDistrustApp::draw()
{
	static bool moveLight = false;

	// clear out the window with black
	ci::gl::clear( ci::Color( 0, 0, 0 ), true );

	// Draw 2D
	ci::gl::setMatricesWindow( getWindowSize() );
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
	//drawKinectDepth();
	ci::gl::color( ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f) );

	ci::gl::enableAlphaBlending();

	// Draw 3D
	ci::gl::setMatrices( _mayaCam.getCamera() );
	ci::gl::enableDepthRead();
	ci::gl::enableDepthWrite();
	ci::gl::disableAlphaBlending();




	_light->update( _mayaCam.getCamera() );
	_light->enable();
	if(!moveLight)
	{
		// Move light
		float r = 500;
		float x = ci::math<float>::cos( mAngle ) * r;
		float y = ci::math<float>::sin( getElapsedSeconds() * 0.2 ) * 500;
		float z = ci::math<float>::sin( mAngle ) * r;
		ci::Vec3f camEye = _mayaCam.getCamera().getEyePoint();

		_light->lookAt( ci::Vec3f(x, y, z), ci::Vec3f(0, y, 0) );
		float dir = ci::math<float>::abs( ci::math<float>::sin( getElapsedSeconds() * 0.015 ) ) * 0.889f + 0.1f;
		GLfloat light_position[] = { x, y, z, dir };
		glLightfv( GL_LIGHT0, GL_POSITION, light_position );
	}



	// BEGIN draw
	_shader.bind();
	_shader.uniform( "NumEnabledLights", 1 );
		ci::gl::draw( *_particleMesh );
//		float cubeSize = 25;
		ci::gl::drawCube( ci::Vec3f::zero(), ci::Vec3f(25.0f, 25.0f, 25.0f ));
	_shader.unbind();
	// END SHADER
	_light->disable();


	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
	ci::gl::enableAdditiveBlending();


	// Draw force billboards
	ci::Vec3f mRight, mUp;
	_mayaCam.getCamera().getBillboardVectors(&mRight, &mUp);
	_textureParticle.bind();
	glEnable( GL_TEXTURE_2D );
	size_t len = _forces.size();
	//billboardSize;
	for(int i = 0; i < len; ++i ) {
		if( !_forces[i].isActive ) continue;
		_forces[i].textureScale -= (_forces[i].textureScale - defaultTextureScale) * 0.25;
		ci::gl::drawBillboard( _forces[i].position, ci::Vec2f(_forces[i].textureScale, _forces[i].textureScale), 0.0f, mRight, mUp);
	}
//	ci::gl::drawBillboard( ci::Vec3f::zero(), ci::Vec2f(textureScale, textureScale), 0.0f, mRight, mUp); // Debug one at origin
	glDisable( GL_TEXTURE_2D );
//ci::ColorA( 0.0f, 0.0f, 0.0f, 1.0f );

#ifdef  __USE_KINECT
	if(shouldDrawSkeleton) {
		CinderOpenNISkeleton *skeleton = CINDERSKELETON;
		skeleton->debugDrawSkeleton();
	}
#endif
//	ci::gl::draw(_textureParticle, getWindowBounds() );
	ci::Vec3f forceCubeSize = ci::Vec3f(25.0f, 25.0f, 25.0f );

//	ci::gl::drawFrustum( _light->getShadowCamera() );
	// Draw floor
	ci::gl::enableWireframe();
	ci::gl::draw( *_planeMesh );
	ci::gl::disableWireframe();

	if( shouldDrawSimpleGui )
		GUI->draw();
}


void QuadDistrustApp::resize( ci::app::ResizeEvent event )
{
	ci::CameraPersp cam = _mayaCam.getCamera();
	cam.setPerspective( 70,  event.getAspectRatio(), 1, 7000);
	_mayaCam.setCurrentCam( cam );
}

void QuadDistrustApp::update()
{
	static bool didTest = false;
	static ofxSinCosLUT sinCosLUT;
	static ci::Perlin	perlin;
	static float mCounter = ci::Rand::randFloat() * 1000;

	if(!didTest) {
		perlin.setSeed( clock() );
//		for(float i = 0; i < TWO_PI*10; i+=0.01f){
//			std::cout << sinCosLUT._sin( i/TWO_PI ) << std::endl << ci::math<float>::sin( i/TWO_PI ) << std::endl << std::endl;
//		}
		didTest = true;
	}
	if( ! _mouseIsDown )
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
	float nZ = 0.9f;//getElapsedSeconds() * 0.005;
	mCounter += 0.01;




#ifdef __USE_KINECT
	CinderOpenNISkeleton *skeleton = CINDERSKELETON;

	XnSkeletonJoint jointsOfInterest[3] = {XN_SKEL_LEFT_HAND, XN_SKEL_RIGHT_HAND, XN_SKEL_HEAD};
	float forceCharge[3] = {1.0f, -1.0f, 0.5};
	// Add force per hand
	int forceIterator = 0;
	for (int i = 0; i < skeleton->_allUsers.size(); ++i)
	{
		// Not tracking - set forces for this user to false
		XnUserID userId = skeleton->currentUsers[i];
		if( !skeleton->mUserGenerator.GetSkeletonCap().IsTracking( userId ) )
		{
			_forces[forceIterator].isActive = false;
			_forces[forceIterator+1].isActive = false;
			forceIterator += 2;
			continue;
		}

		// Create i, i+1, i+n force objects per user
		int forcesPerUser = 2;
		for(int j = 0; j < forcesPerUser; ++j)
		{
			_forces[forceIterator+j].isActive = true;
			_forces[forceIterator+j].charge = forceCharge[i+j];
//			_forces[forceIterator+j].targetPosition = _forces[i+j].position;
			_forces[forceIterator+j].position = skeleton->_allUsers[i].projectedPositions[ jointsOfInterest[j] ];
		}


//
		forceIterator += 2;

	}

	_forces[i].position = (_forces[i].targetPosition - skeleton->_allUsers[i].projectedPositions[XN_SKEL_LEFT_HAND]) * ease;

#else
	size_t len = _forces.size();
	float ease = 0.09f;
	for(int i = 0; i < len; ++i ) {
		_forces[i].position -= (_forces[i].position - _forces[i].targetPosition) * ease;
	}
#endif


	float maxDistSQ = _forceMaxDist*_forceMaxDist;
	size_t forcesLength = _forces.size();
	while(j < i)
	{
		IndexedQuad* iq = &_indexedQuads[indexQuadIterator];
		ci::Vec3f noisePosition = vec[j];

		iq->age++;
		iq->agePer = iq->age / iq->lifespan;

		if(shouldApplyForces)
		{
			// Update forces
			for(int ig = 0; ig < forcesLength; ig++ )
			{
				if(!_forces[ig].isActive)
					continue;

				ci::Vec3f delta = _forces[ig].position - iq->position;
				float s = delta.lengthSquared();
				if( s > _forceMinDist*_forceMinDist  && s < maxDistSQ ) // is within range
				{
					// normalize
					float dist = ci::math<float>::sqrt( s );
					float invS = 1.0f / dist;
					delta *= invS;

					// Apply inverse force
					float inverseForce = 1.0f - dist/_forceMaxDist * _forceStrength;//(1.0-(dist/maxDist)) * force / iq->mass;
					delta *= inverseForce;

					if( ci::Rand::randFloat() < 0.3f) delta = -delta;

					iq->velocity += delta;
	//				nZ += 0.005f;
				}
			}
		}

			// Apply simplex noise
			iq->position = vec[j];
			noisePosition *= 0.0005f;
			float nNoise = _simplexNoise->noise( noisePosition.x, noisePosition.y, noisePosition.z, mCounter);
			nNoise *= TWO_PI*2;

			iq->velocity.x += cosf(nNoise) * maxSpeed * nZ;
			iq->velocity.y += sinCosLUT._sin( nNoise ) * 0.5 * nZ; // Apply gravity
			iq->velocity.z += sinf(nNoise) * maxSpeed * nZ;
		// Cap
		iq->velocity.limit( maxVel );

		// Apply velocity to positions
		if( shouldApplyNoise ) {
			moveSpeed = iq->velocity;
		}

		moveSpeed.y += grav;
		vec[j] += moveSpeed;
		vec[j+1] += moveSpeed;
		vec[j+2] += moveSpeed;
		vec[j+3] += moveSpeed;



		iq->velocity *= 0.98f;


		if(iq->age > iq->lifespan /* || vec[j].y > iq->skyLimit */)
		{
			float radius = 2000;

			iq->velocity = ci::Vec3f::zero();
			iq->age = 1.0f;
			iq->lifespan = quadRandomLifespan;

			// (if X and Z use the same angle we will create a cylinder
			float xAngle = ci::Rand::randFloat( (float)M_PI * 2.0f );
			float zAngle = ci::Rand::randFloat( (float)M_PI * 2.0f );
			float rotAngle = ci::Rand::randFloat( (float)M_PI * 2 );

			float x = ci::math<float>::cos( xAngle ) * radius;
			float y = ci::Rand::randFloat() * 200;
			float z = ci::math<float>::sin( zAngle ) * radius;
			ci::Vec3f pos = ci::Vec3f( x, y, z );

			// Modify quad vertices to place at position
			ZoaDebugFunctions::createQuadAtPosition( pos, vec[j], vec[j+1], vec[j+2], vec[j+3], ci::Rand::randFloat(_quadMaxSize/2, _quadMaxSize*4), _quadMaxDistortion, rotAngle );

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

	mAngle += 0.2f;
	handleAutoRotation();
}



#ifdef  __USE_KINECT
// Returns the area where the kinect depth map is drawn
// This is used when drawing labels, to draw the labels at the relative location by scaling, then translating the values returned by the kinect
ci::Rectf QuadDistrustApp::getKinectDepthArea( int width, int height )
{
    int padding = 10;
    int y1 = getWindowSize().y - height;
    int y2 = y1 + height;

	return ci::Rectf( padding, y1 - padding, width + padding, y2 - padding );
}
#endif


CINDER_APP_BASIC( QuadDistrustApp, ci::app::RendererGl(1) )
