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
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
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

GLfloat no_mat[]			= { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[]		= { 0.3, 0.1, 0.4, 1.0 };
GLfloat mat_diffuse[]		= { 0.3, 0.5, 0.8, 1.0 };
GLfloat mat_specular[]		= { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_emission[]		= { 0.4, 0.7, 1.0, 0.0 };
GLfloat no_shininess[]		= { 0.0 };
GLfloat mat_shininess[]		= { 128.0 };

class QuadDistrustApp : public ci::app::AppBasic {
public:
	void 	prepareSettings( ci::app::AppBasic::Settings *settings );
	void 	setup();
	void 	setupCamera();
	void 	setupQuadSprites();

	void	resize( ci::app::ResizeEvent event );
	void	mouseDown( ci::app::MouseEvent event );
	void	mouseMove( ci::app::MouseEvent event );
	void	mouseDrag( ci::app::MouseEvent event );
	void	mouseUp( ci::app::MouseEvent event );
	void 	keyDown( ci::app::KeyEvent event );

	void 	update();
	void 	draw();
	void 	addQuadToMesh( ci::TriMesh& mesh, const ci::Vec3f& P0, const ci::Vec3f& P1, const ci::Vec3f& P2, const ci::Vec3f& P3, const ci::ColorA& color );
	void	createPlane();
	void 	calculateTriMeshNormals( ci::TriMesh &mesh );

	//
	ci::Matrix44f		_cubeRotation;
	ci::gl::Texture		_texture;
	ci::MayaCamUI		_mayaCam;
	ci::TriMesh*		_particleMesh;
	ci::TriMesh*		_planeMesh;

	ci::gl::GlslProg	mShader;
	float				mAngle;

	float		mCounter;
	float		mouseX;
	float		mouseY;
	bool		mMOUSEDOWN;

	ci::Vec3f	mMouseLoc;

	bool		mDIFFUSE;
	bool		mAMBIENT;
	bool		mSPECULAR;
	bool		mEMISSIVE;
	bool		mSHADER;
	float		mDirectional;
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

		mDIFFUSE		= true;
		mAMBIENT		= true;
		mSPECULAR		= true;
		mSHADER			= true;
		mEMISSIVE		= false;

		mDirectional	= 1.0f;


	mMouseLoc = ci::Vec3f::zero();

	glDisable( GL_TEXTURE_2D );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	ci::gl::enableDepthWrite();
	ci::gl::enableDepthRead();
	ci::gl::enableAlphaBlending();

	createPlane();
//
//	mAngle = 0.0f;

/*
 * grid = new Array(_segmentsW+1);
            for (i = 0; i <= _segmentsW; ++i) {
                grid[i] = new Array(_segmentsH+1);
                for (j = 0; j <= _segmentsH; ++j) {
                	if (_yUp)
                    	grid[i][j] = createVertex((i / _segmentsW - 0.5) * _width, 0, (j / _segmentsH - 0.5) * _height);
                    else
                    	grid[i][j] = createVertex((i / _segmentsW - 0.5) * _width, (j / _segmentsH - 0.5) * _height, 0);
                }
            }

            for (i = 0; i < _segmentsW; ++i) {
                for (j = 0; j < _segmentsH; ++j) {
                    var a:Vertex = grid[i  ][j  ];
                    var b:Vertex = grid[i+1][j  ];
                    var c:Vertex = grid[i  ][j+1];
                    var d:Vertex = grid[i+1][j+1];

                    var uva:UV = createUV(i     / _segmentsW, j     / _segmentsH);
                    var uvb:UV = createUV((i+1) / _segmentsW, j     / _segmentsH);
                    var uvc:UV = createUV(i     / _segmentsW, (j+1) / _segmentsH);
                    var uvd:UV = createUV((i+1) / _segmentsW, (j+1) / _segmentsH);

                    addFace(createFace(a, b, c, null, uva, uvb, uvc));
                    addFace(createFace(d, c, b, null, uvd, uvc, uvb));
                }
            }
 */
}

void QuadDistrustApp::createPlane()
{
	_planeMesh = new ci::TriMesh();
	_planeMesh->clear();

	float segmentsW = 4;
	float segmentsH = 4;
	float width = 100;
	float height = width;


	std::vector< std::vector<ci::Vec3f> > grid;
	for(int i = 0; i <= segmentsW; ++i)
	{
		std::vector<ci::Vec3f> row;
		grid.push_back( row );
		for(int j = 0; j <= segmentsH; ++j)
		{
			ci::Vec3f pos = ci::Vec3f(((float)i / segmentsW - 0.5f) * width, 0, ((float)j / segmentsH - 0.5f) *  height);
			grid[i].push_back( pos );
		}
	}

	for(int i = 0; i < segmentsW; ++i) {
		for( int j = 0; j < segmentsH; ++j) {
			ci::Vec3f a = grid[i  ][j  ];
			ci::Vec3f b = grid[i+1][j  ];
			ci::Vec3f c = grid[i  ][j+1];
			ci::Vec3f d = grid[i+1][j+1];

			ci::ColorA color = ci::ColorA( ci::Rand::randFloat(), ci::Rand::randFloat(), ci::Rand::randFloat(), 0.8 );

			std::cout << a << b << c << d << std::endl;


			ci::Vec3f e0 = c - a;
			ci::Vec3f e1 = c - b;
			ci::Vec3f n = -e0.cross(e1).normalized();

			_planeMesh->appendVertex( a );
			_planeMesh->appendColorRGBA( color );
			_planeMesh->appendNormal( n );

			_planeMesh->appendVertex( b );
			_planeMesh->appendColorRGBA( color );
			_planeMesh->appendNormal( n );

			_planeMesh->appendVertex( c );
			_planeMesh->appendColorRGBA( color );
			_planeMesh->appendNormal( n );

			_planeMesh->appendVertex( d );
			_planeMesh->appendColorRGBA( color );
			_planeMesh->appendNormal( n );

			int vertA = _planeMesh->getNumVertices() - 4;
			int vertB = _planeMesh->getNumVertices() - 3;
			int vertC = _planeMesh->getNumVertices() - 2;
			int vertD = _planeMesh->getNumVertices() - 1;

			_planeMesh->appendTriangle( vertA, vertB, vertC );
			_planeMesh->appendTriangle( vertD, vertC, vertB );

		}
	}
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

	float count = 1000;

	float theta 	 = M_PI * (3.0 - sqrtf(5.0));
	float o			 = 2.0f / count;
	float radius	 = 100;
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

		ci::ColorA aColor( ci::CM_HSV, ci::Rand::randFloat() * 0.2 + 0.4, 0.7f, 0.9f, 0.9f );
		float angle = ci::Rand::randFloat( M_PI );

		ci::Vec3f v1; ci::Vec3f v2; ci::Vec3f v3; ci::Vec3f v4;
		ZoaDebugFunctions::createQuadAtPosition( pos, v1, v2, v3, v4, 50, 5, angle );
		addQuadToMesh( *_particleMesh, v1, v2, v3, v4, aColor );
	}

//	calculateTriMeshNormals( *_particleMesh );
}

void QuadDistrustApp::addQuadToMesh( ci::TriMesh& mesh, const ci::Vec3f& P0, const ci::Vec3f& P1, const ci::Vec3f& P2, const ci::Vec3f& P3, const ci::ColorA& color )
{
	ci::Vec3f e0 = P2 - P0;
	ci::Vec3f e1 = P2 - P1;
	ci::Vec3f n = -e0.cross(e1).normalized();

	mesh.appendVertex( P0 );
	mesh.appendColorRGBA( color );
	mesh.appendNormal( n );

	mesh.appendVertex( P1 );
	mesh.appendColorRGBA( color );
	mesh.appendNormal( n );

	mesh.appendVertex( P2 );
	mesh.appendColorRGBA( color );
	mesh.appendNormal( n );

	mesh.appendVertex( P3 );
	mesh.appendColorRGBA( color );
	mesh.appendNormal( n );

	int vert0 = mesh.getNumVertices() - 4;
	int vert1 = mesh.getNumVertices() - 1;
	int vert2 = mesh.getNumVertices() - 2;
	int vert3 = mesh.getNumVertices() - 3;

	mesh.appendTriangle( vert0, vert3, vert1 );
	mesh.appendTriangle( vert3, vert2, vert1 );
}

void QuadDistrustApp::calculateTriMeshNormals( ci::TriMesh &mesh )
{
		const std::vector<ci::Vec3f>& vertices = mesh.getVertices();
		const std::vector<size_t>& indices = mesh.getIndices();

		// remove all current normals
		std::vector<ci::Vec3f>& normals = mesh.getNormals();
		normals.reserve( mesh.getNumVertices() );
		normals.clear();

		// set the normal for each vertex to (0, 0, 0)
		for(size_t i=0; i < mesh.getNumVertices(); ++i)
			normals.push_back( ci::Vec3f::zero() );

		// Average out the normal for each vertex at an index
		for(size_t i=0; i< mesh.getNumTriangles(); ++i)
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
	if( event.getChar() == 'd' || event.getChar() == 'D' ){
		mDIFFUSE = ! mDIFFUSE;
	}
	else if( event.getChar() == 'a' || event.getChar() == 'A' ){
		mAMBIENT = ! mAMBIENT;
	}
	else if( event.getChar() == 's' || event.getChar() == 'S' ){
		mSPECULAR = ! mSPECULAR;
	}
	else if( event.getChar() == 'e' || event.getChar() == 'E' ){
		mEMISSIVE = ! mEMISSIVE;
	}
	else if( event.getChar() == 'g' || event.getChar() == 'G' ){
		mSHADER = ! mSHADER;
	}
	else if( event.getChar() == 'p' || event.getChar() == 'p' ){
//		setFullScreen( ! isFullScreen() );
		std::ostringstream stringBuffer;
		stringBuffer << clock();
		std::string aTimeString = stringBuffer.str();
		ci::writeImage( ci::getHomeDirectory() + "QuadDistrust/" + aTimeString + ".png", copyWindowSurface() );
	}
	else if( event.getChar() == '/' || event.getChar() == '?' ){
//		mInfoPanel.toggleState();
	}
	else if( event.getChar() == ',' || event.getChar() == '<' ){
		mat_shininess[0] *= 0.5f;
		if( mat_shininess[0] < 1.0f )
			mat_shininess[0] = 1.0f;
	}
	else if( event.getChar() == '.' || event.getChar() == '>' ){
		mat_shininess[0] *= 2.0f;
		if( mat_shininess[0] > 128.0f )
			mat_shininess[0] = 128.0f;
	}
}


void QuadDistrustApp::resize( ci::app::ResizeEvent event )
{
	ci::CameraPersp cam = _mayaCam.getCamera();
	cam.setPerspective( 60,  event.getAspectRatio(), 1, 6000);
	_mayaCam.setCurrentCam( cam );
}

void QuadDistrustApp::update()
{
	if( ! mMOUSEDOWN )
		mDirectional -= ( mDirectional - 0.985f ) * 0.1f;
	else
		mDirectional -= ( mDirectional - 0.51f ) * 0.1f;

	_cubeRotation.rotate( ci::Vec3f(1, 1, 1), 0.003f );

	float vertexCount = _particleMesh->getNumVertices();
	if( vertexCount == 0 ) return;

	// store all the mesh information
	std::vector<ci::Vec3f> &vec = _particleMesh->getVertices();

	int i, j;
	i = _particleMesh->getNumVertices();
	j = 0;

	// something to add a little movement
	float 		limit = 1000;
	ci::Vec3f	moveSpeed = ci::Vec3f(0, 4, 0);
	std::vector<ci::Vec3f>& normals = _particleMesh->getNormals();

	while(j < i)
	{
//		float angle = ci::Rand::randFloat(0.001f, 0.003);
//		vec[j].rotateY( angle );
//		vec[j+1].rotateY( angle );
//		vec[j+2].rotateY( angle );
//		vec[j+3].rotateY( angle );

		vec[j] += moveSpeed;
		vec[j+1] += moveSpeed;
		vec[j+2] += moveSpeed;
		vec[j+3] += moveSpeed;

		if(vec[j].y > limit )
		{
			float angle = ci::Rand::randFloat( (float)M_PI * 2.0f );
			float radius = 2000;
			float x = ci::math<float>::cos( angle ) * radius;
			float y = ci::Rand::randFloat() * 600;
			float z = ci::math<float>::sin( ci::Rand::randFloat( (float)M_PI * 2.0f ) ) * radius;


			// Normalize then position of each vector in the quad, and then set at new random location
			// Otherwise quad will be come zero width
			ci::Vec3f pos = ci::Vec3f( x, y, z );
			ZoaDebugFunctions::createQuadAtPosition( pos, vec[j], vec[j+1], vec[j+2], vec[j+3], 4, 0.5, ci::Rand::randFloat( (float)M_PI) );

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
	glEnable( GL_LIGHT0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	mMouseLoc -= ( mMouseLoc - ci::Vec3f( mouseX, mouseY, 500.0f ) ) * 0.2f;

	ci::Vec3f camPos = _mayaCam.getCamera().getEyePoint();
	GLfloat light_position[] = { camPos.x, camPos.y, camPos.z, mDirectional };
	glLightfv( GL_LIGHT0, GL_POSITION, light_position );

	//
	GLfloat light_Kd[] = { 1.0, 0.5, 1.0, 1.0 };
	glLightfv( GL_LIGHT0, GL_DIFFUSE, light_Kd);
	glLightf( GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5f );
	glLightf( GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f );
	glLightf( GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.00015f );

//	ci::Vec3f dirFromLight	= ci::Rand::randVec3f() * 1000;
//	float distFromLight	= dirFromLight.length();

	if( mDIFFUSE ){
		ci::ColorA color( 1.0f, 1.0f, 1.0f, 1.0f );
		glMaterialfv( GL_FRONT, GL_DIFFUSE,	color );
	} else {
		glMaterialfv( GL_FRONT, GL_DIFFUSE,	no_mat );
	}


	if( mAMBIENT ){
		glMaterialfv( GL_FRONT, GL_AMBIENT,	mat_ambient );
	} else {
		glMaterialfv( GL_FRONT, GL_AMBIENT,	no_mat );
	}


	if( mSPECULAR ){
		glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );
		glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess );
	} else {
		glMaterialfv( GL_FRONT, GL_SPECULAR, no_mat );
		glMaterialfv( GL_FRONT, GL_SHININESS, no_shininess );
	}


	if( mEMISSIVE ){
		glMaterialfv( GL_FRONT, GL_EMISSION, mat_emission );
	} else {
		glMaterialfv( GL_FRONT, GL_EMISSION, no_mat );
	}


	if( mSHADER ){
		mShader.bind();
		mShader.uniform( "NumEnabledLights", 1 );
	}

//	ci::gl::enableAlphaBlending();
	float cubeSize = 25;
	ci::gl::draw( *_particleMesh );
	ci::gl::drawCube( ci::Vec3f::zero(), ci::Vec3f(cubeSize, cubeSize, cubeSize) );
	if( mSHADER ) mShader.unbind();

//	glDisable( GL_LIGHTING );
	glColor3f( 1, 1, 1 );
	ci::gl::draw( *_planeMesh );
//	ZoaDebugFunctions::drawFloorPlane();
//	ZoaDebugFunctions::trimeshDrawNormals( *_particleMesh );
}

CINDER_APP_BASIC( QuadDistrustApp, ci::app::RendererGl(4) )
