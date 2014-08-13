#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/GeomIo.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/gl/Context.h"
#include "cinder/Perlin.h"

#include "GBuffer.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Light {
public:
    
    Light( int _id = 0, const Vec3f& pos = Vec3f(0,1,0), const Color& diff = Color(1.,1.,1.), const Color& spec = Color(1.,1.,1.), const float& intense = 1. ) : id(_id), position(pos), specular(spec), diffuse(diff), intensity(intense) {}
    ~Light(){}
    
    Vec3f position;
    Color diffuse;
    Color specular;
    float intensity;
    int id;
    
    virtual void bind();
    
};

void Light::bind(){
    auto ctx = gl::context();
    auto glsl = ctx->getGlslProg();
    glsl->uniform("lights["+to_string(id)+"].position", position);
    glsl->uniform("lights["+to_string(id)+"].diffuse", diffuse);
    glsl->uniform("lights["+to_string(id)+"].specular", specular);
    glsl->uniform("lights["+to_string(id)+"].intensity", intensity);
    
}

class Sun : public Light {
public:
    Sun( const Vec3f& pos = Vec3f(0,1,0), const Color& diff = Color(1.,1.,1.), const Color& spec = Color(1.,1.,1.), const float& intense = 1. ) : Light(-1, pos, diff, spec, intense) {}
    ~Sun(){}
    
    void bind();
};

void Sun::bind(){
    auto ctx = gl::context();
    auto glsl = ctx->getGlslProg();
    glsl->uniform("Sun.position", position);
    glsl->uniform("Sun.diffuse", diffuse);
    glsl->uniform("Sun.specular", specular);
    glsl->uniform("Sun.intensity", intensity);
}

class Deffered_GBufferApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
    void keyDown( KeyEvent event );
	void update();
	void draw();
    void prepareSettings(Settings* settings);
    void initQuad();
    void initTeapot();
    void initLights();
    void drawDebug();
    
    GBufferRef mGBuffer;
    
    TriMeshRef      mTeapotMesh;
    gl::GlslProgRef mGPass, mLightPass, mQuadGlsl;
    gl::FboRef      mLightTarget;
    CameraPersp     mCam;
    
    gl::VaoRef      mVao;
    gl::VboRef      mPositions, mTexCoords, mNormals, mElements;
    
    gl::TextureRef  mDiffuseMap, mNormalMap, mSpecularMap;
    
    gl::VaoRef      mQuadVao;
    gl::VboRef      mQuadPos, mQuadTexCoord, mQuadElements;
    
    bool bShowDebug = false;
    Perlin perlin;
    vector<Light> lights;
    Color mGlobalAmbient;
    
    Sun mSun;
    
};

void Deffered_GBufferApp::keyDown(cinder::app::KeyEvent event){
    if(event.getChar() == ' '){
        bShowDebug = !bShowDebug;
    }
}

void Deffered_GBufferApp::prepareSettings(cinder::app::AppBasic::Settings *settings){
    //settings->setWindowSize(1024, 768);
    settings->setFullScreen();
    
}

void Deffered_GBufferApp::setup()
{
    initQuad();
    initTeapot();
    initLights();
    
    ///Create GBuffer and GPass GLSL
    
    GBuffer::Format format;
    format.attachment( GL_COLOR_ATTACHMENT0, GL_RGBA32F, "wcPositions");
    format.attachment( GL_COLOR_ATTACHMENT1, GL_RGBA32F, "wcNormals");
    format.attachment( GL_COLOR_ATTACHMENT2, GL_RGBA32F, "DiffuseSpecular");

    mGBuffer = GBuffer::create(getWindowWidth(), getWindowHeight(), format );
    
    gl::GlslProg::Format gPassFormat;
    gPassFormat.vertex(loadResource(GPASS_VERT))
    .fragment(loadResource(GPASS_FRAG))
    .uniform(gl::UniformSemantic::UNIFORM_MODEL_VIEW_PROJECTION, "ciModelViewProjectionMatrix")
    .uniform(gl::UniformSemantic::UNIFORM_MODEL_VIEW, "ciModelViewMatrix")
    .uniform(gl::UniformSemantic::UNIFORM_NORMAL_MATRIX, "ciNormalMatrix")
    .uniform(gl::UniformSemantic::UNIFORM_MODEL_MATRIX, "ciModelMatrix")
    .uniform(gl::UniformSemantic::UNIFORM_VIEW_MATRIX_INVERSE, "ciInverseViewMatrix")
    
    ///these attributes need to match the geometry you are drawing into the GBuffer
    
    .attribLocation("ciPosition", 0)
    .attribLocation("ciNormal", 1)
    .attribLocation("ciTexCoord", 2)
    .label("G-Buffer Pass");
    
    mGPass = gl::GlslProg::create(gPassFormat);
    
    ///setup camera
    
    mCam.setPerspective(50, getWindowAspectRatio(), .1, 10000);
    mCam.lookAt(Vec3f(40,10,100),Vec3f(40,10,-80),Vec3f::yAxis());
    
    gl::enableVerticalSync();

}

void Deffered_GBufferApp::mouseDown( MouseEvent event )
{
}

void Deffered_GBufferApp::update()
{
    ///GBuffer pass
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    mGBuffer->bind();
    
    gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); //clear depth buffer
    gl::depthMask(true); //enable the depth buffer
    gl::viewport(0, 0, mGBuffer->getWidth(), mGBuffer->getHeight() );
    gl::setMatrices(mCam);
    
    ///draw whatever you want to be deferred into the GBuffer
    
    gl::pushMatrices();
    {
        gl::ScopedVao vao( mVao );
        gl::ScopedBuffer elements( mElements );
        gl::ScopedGlslProg GPass( mGPass );
        gl::setDefaultShaderVars();
        
        gl::ScopedTextureBind tex1( mDiffuseMap, 0 );
        gl::ScopedTextureBind tex2( mNormalMap, 1 );
        gl::ScopedTextureBind tex3( mSpecularMap, 2 );

        mGPass->uniform("uSpecular", 128.f);
        mGPass->uniform("diffuseMap", 0);
        mGPass->uniform("normalMap", 1);
        mGPass->uniform("specularMap", 2);
            
        glDrawElementsInstanced(GL_TRIANGLES, mTeapotMesh->getNumIndices(), GL_UNSIGNED_INT, 0,1000);
        
        //draw teapots where the lights are
        for(Light l : lights){
            gl::pushMatrices();
            gl::translate(l.position.x, l.position.y+5, l.position.z);
            gl::setDefaultShaderVars();
            gl::drawElements(GL_TRIANGLES, mTeapotMesh->getNumIndices(), GL_UNSIGNED_INT, 0);
            gl::popMatrices();
        }
        
    }
    gl::popMatrices();
    
    gl::depthMask(false);
    
    
    mGBuffer->unbind();
    
    ///now we no longer are drawing any geometry other than a quad, the rest takes place in screen space
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    mLightTarget->bindFramebuffer();
    gl::clear();
    gl::setMatricesWindow(mLightTarget->getSize());
    gl::viewport(0, 0, mLightTarget->getWidth(), mLightTarget->getHeight());
    {
        gl::ScopedVao vao(mQuadVao);
        gl::ScopedBuffer vbo(mQuadElements);
        gl::ScopedGlslProg glsl( mLightPass );
        
        ///get our data textures from the GBuffer
        
        mGBuffer->bindTexture(0,"wcPositions");
        mGBuffer->bindTexture(1,"wcNormals");
        mGBuffer->bindTexture(2,"DiffuseSpecular");
        mGBuffer->bindDepthTexture(3);
        
        ///send them to the lighting GLSL to reconstruct and light the fragments in screen space
        
        mLightPass->uniform("wcPositions", 0);
        mLightPass->uniform("wcNormals", 1);
        mLightPass->uniform("DiffuseSpecular", 2);
        mLightPass->uniform("DepthTex", 3);
        mLightPass->uniform("GlobalAmbient", mGlobalAmbient);
        
        ///bind all the light uniforms
        
        mSun.bind();
        
        gl::setDefaultShaderVars();
        
        for(int i=0;i<lights.size();i++){
            lights[i].position = perlin.dnoise(i+getElapsedSeconds()/30., i+getElapsedSeconds()/30., i+getElapsedSeconds()/30.)*220 + Vec3f(300.,10.,-200.);
            lights[i].position.y = 15.f;
            lights[i].position.z *= 5.f;
            lights[i].position += .00000000001;
            lights[i].bind();
        }
        
        mLightPass->uniform("numLights", (float)lights.size() );
        
        ///draw a quad to render the scene to
        
        gl::drawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
    }
    mLightTarget->unbindFramebuffer();
    
    //rotate the camera around
    mCam.setEyePoint(Vec3f( 300+100*cos( getElapsedSeconds()*.2), sin( getElapsedSeconds()*.2 )*50.f+10.f, 600*sin( getElapsedSeconds()*.2) - 500) );
    mCam.setCenterOfInterestPoint(Vec3f(500.,10.,-1000.));
    
    WindowRef w = getWindow();
    w->setTitle(to_string(getAverageFps()));

}

void Deffered_GBufferApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    gl::setMatricesWindow(getWindowWidth(), getWindowHeight());
    gl::viewport(0, 0, getWindowWidth(), getWindowHeight());
    
    
    ///actually draw the lighting target texture or see what the GBuffer is up to
    
    gl::ScopedVao vao(mQuadVao);
    gl::ScopedBuffer vbo(mQuadElements);
    gl::ScopedGlslProg glsl( mQuadGlsl );
    mQuadGlsl->uniform("tex0", 0);
    
    if(!bShowDebug){
        
        gl::pushMatrices();
        gl::setDefaultShaderVars();
        mLightTarget->bindTexture(0);
        gl::drawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        gl::popMatrices();
        
    }else{
        drawDebug();
    }
    
}

void Deffered_GBufferApp::drawDebug(){
    
    //see world cordinate vertex positions
    gl::pushMatrices();
    gl::scale(.5, .5);
    gl::setDefaultShaderVars();
    mGBuffer->bindTexture(0,"wcPositions");
    gl::drawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    gl::popMatrices();
    
    //see normals
    gl::pushMatrices();
    gl::translate(Vec2f(getWindowWidth()/2, 0));
    gl::scale(.5, .5);
    gl::setDefaultShaderVars();
    mGBuffer->bindTexture(0,"wcNormals");
    gl::drawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    gl::popMatrices();
    
    //see diffuse texture color
    gl::pushMatrices();
    gl::translate(Vec2f(0,getWindowHeight()/2));
    gl::scale(.5, .5);
    gl::setDefaultShaderVars();
    mGBuffer->bindTexture(0,"DiffuseSpecular");
    gl::drawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    gl::popMatrices();
    
    //see depth buffer
    gl::pushMatrices();
    gl::translate(Vec2f(getWindowWidth()/2, getWindowHeight()/2));
    gl::scale(.5, .5);
    gl::setDefaultShaderVars();
    mGBuffer->bindDepthTexture(0);
    gl::drawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    gl::popMatrices();
}


void Deffered_GBufferApp::initQuad(){
    
    vector<Vec3f> positions;
    vector<uint32_t> indices;
    vector<Vec2f> texCoords;
    
    positions.push_back( Vec3f( 0, 0, 0) );
    positions.push_back( Vec3f( 0, getWindowHeight(), 0) );
    positions.push_back( Vec3f( getWindowWidth(), getWindowHeight(), 0) );
    positions.push_back( Vec3f( getWindowWidth(), 0, 0) );
    
    texCoords.push_back(Vec2f( 1,1 ));
    texCoords.push_back(Vec2f( 1,0 ));
    texCoords.push_back(Vec2f( 0,0 ));
    texCoords.push_back(Vec2f( 0,1 ));
    
    
    indices.push_back( 0 );  // everything is made of triangles, even rectangels, three indexes per triangle
    indices.push_back( 1 );  // 0 __   3
    indices.push_back( 3 );  //  |  //|
    indices.push_back( 1 );  //  | // |
    indices.push_back( 2 );  //  |//__|
    indices.push_back( 3 );  // 1      2
    
    
    mQuadPos = gl::Vbo::create(GL_ARRAY_BUFFER, positions.size()*sizeof(Vec3f), positions.data(), GL_STATIC_DRAW);
    mQuadTexCoord = gl::Vbo::create(GL_ARRAY_BUFFER, texCoords.size()*sizeof(Vec2f), texCoords.data(), GL_STATIC_DRAW);
    mQuadElements = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    mQuadVao = gl::Vao::create();
    
    {
        gl::ScopedVao vao(mQuadVao);
        {
            gl::ScopedBuffer vbo(mQuadPos);
            gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            gl::enableVertexAttribArray(0);
        }
        {
            gl::ScopedBuffer vbo(mQuadTexCoord);
            gl::vertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
            gl::enableVertexAttribArray(1);
        }
    }
    
    gl::GlslProg::Format debug;
    debug.vertex(loadResource(DEBUG_VERT))
    .fragment(loadResource(DEBUG_FRAG))
    .uniform(gl::UniformSemantic::UNIFORM_MODEL_VIEW_PROJECTION, "ciModelViewProjectionMatrix")
    .attribLocation("position", 0)
    .attribLocation("texcoord", 1);
    mQuadGlsl = gl::GlslProg::create(debug);

}

void Deffered_GBufferApp::initTeapot(){
    
    mTeapotMesh = TriMesh::create(geom::Teapot().enable(geom::Attrib::POSITION).enable(geom::Attrib::NORMAL).enable(geom::Attrib::TEX_COORD_0));
    
    mVao = gl::Vao::create();
    
    mPositions = gl::Vbo::create(GL_ARRAY_BUFFER, mTeapotMesh->getNumVertices() * sizeof(Vec3f), mTeapotMesh->getVertices<3>(), GL_STATIC_DRAW );
    mNormals = gl::Vbo::create(GL_ARRAY_BUFFER, mTeapotMesh->getNormals().size() * sizeof(Vec3f), mTeapotMesh->getNormals().data(),GL_STATIC_DRAW );
    mTexCoords = gl::Vbo::create(GL_ARRAY_BUFFER, mTeapotMesh->getTexCoords().size() * sizeof(float), mTeapotMesh->getTexCoords().data(),GL_STATIC_DRAW );
    mElements = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER, mTeapotMesh->getNumIndices() * sizeof(uint32_t), mTeapotMesh->getIndices().data(),GL_STATIC_DRAW );
    
    {
        gl::ScopedVao vao(mVao);
        
        {
            gl::ScopedBuffer vbo(mPositions);
            gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            gl::enableVertexAttribArray(0);
        }
        {
            gl::ScopedBuffer vbo(mNormals);
            gl::vertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            gl::enableVertexAttribArray(1);
        }
        {
            gl::ScopedBuffer vbo(mTexCoords);
            gl::vertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0);
            gl::enableVertexAttribArray(2);
        }
    }
    
    ///textures for the teapots
    
    mDiffuseMap = gl::Texture::create(loadImage(loadResource(ROCK)));
    mNormalMap = gl::Texture::create(loadImage(loadResource(FOIL_NRM)));
    mSpecularMap = gl::Texture::create(loadImage(loadResource(FOIL_SPEC)));

}

void Deffered_GBufferApp::initLights(){
    
    ///create a lighting target
    
    gl::Fbo::Format lightAccumulator;
    lightAccumulator.disableDepth();
    lightAccumulator.colorTexture();
    lightAccumulator.setColorBufferInternalFormat(GL_RGBA32F);
    lightAccumulator.setSamples(16);
    
    // lightAccumulator.stencilBuffer();
    
    mLightTarget = gl::Fbo::create(getWindowWidth(), getWindowHeight(), lightAccumulator);
    
    gl::GlslProg::Format lightPassFormat;
    lightPassFormat.vertex(loadResource(LIGHT_VERT))
    .fragment(loadResource(LIGHT_FRAG))
    .uniform(gl::UniformSemantic::UNIFORM_MODEL_VIEW_PROJECTION, "ciModelViewProjectionMatrix")
    .uniform(gl::UniformSemantic::UNIFORM_MODEL_VIEW, "ciModelViewMatrix")
    .uniform(gl::UniformSemantic::UNIFORM_NORMAL_MATRIX, "ciNormalMatrix")
    .uniform(gl::UniformSemantic::UNIFORM_VIEW_MATRIX, "ciViewMatrix")
    .attribLocation("ciPosition", 0)
    .attribLocation("ciTexCoord", 1)
    .label("Light Pass");
    mLightPass = gl::GlslProg::create(lightPassFormat);
    
    ///create Lights and scene settings
    
    int grid = 6;
    lights.resize(grid*grid);
    for( int x=0;x<grid;x++ ){
        for(int y =0;y<grid;y++){
            lights[x+y*grid].id = x+y*grid;
            lights[x+y*grid].position = Vec3f( x*1.f, 5.f, y*1.f);
            float a = lmap((float)(x+y*grid), 0.f, (float)lights.size(), 0.f, 1.f);
            lights[x+y*grid].diffuse = Color( a, 1.f - a, 1. );
            lights[x+y*grid].specular = Color( a, 1.f - a, 1. );
            lights[x+y*grid].intensity = 1.;
        }
    }
    
    mSun = Sun( Vec3f(1000,1000,-5000), Color(1.,1.,.9), Color(1.,1.,.9), 1. );
    
    mGlobalAmbient = Color(.05,.05,.05);

}

CINDER_APP_NATIVE( Deffered_GBufferApp, RendererGl )
