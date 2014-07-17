//
//  GBuffer.cpp
//  Deffered_GBuffer
//
//  Created by Mike Allison on 7/17/14.
//
//

#include "GBuffer.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;
using namespace std;


GBuffer::GBuffer( const float& width, const float& height, const Format& format ) {
    
    gl::Texture::Format tex;
    tex.setMagFilter(GL_NEAREST);
    tex.setMinFilter(GL_NEAREST);
    
    gl::Fbo::Format fboFormat = format.mFboFormat;
    
    for( Attachment a : format.mAttachments ){
        tex.setInternalFormat( a.getType() );
        gl::TextureRef texture = gl::Texture::create( width, height, tex );
        mAttachments.emplace( a.getLabel(), texture );
        fboFormat.attachment( a.getLocation(), mAttachments.find(a.getLabel())->second );
    }
    
    mFbo = gl::Fbo::create( width, height, fboFormat );
    
}

GBuffer::~GBuffer(){
    mFbo = nullptr;
    mAttachments.clear();
}

void GBuffer::bind(){
    mFbo->bindFramebuffer();
}
void GBuffer::unbind(){
    mFbo->unbindFramebuffer();
}

//should turn this off
ci::gl::TextureRef GBuffer::getTexture( const std::string& label ){
    if(mAttachments.count(label) == 0){
        cout<<"[ERROR] GBuffer::getTexture::There is no attachment with the label: "<<label<<endl;
        return nullptr;
    }else{
        return mAttachments.find(label)->second;
    }
}

void GBuffer::bindTexture( const int& bind_location, const std::string& label ){
    if(mAttachments.count(label) == 0){
        cout<<"[ERROR] GBuffer::bindTexture::There is no attachment with the label: "<<label<<endl;
    }else{
        mAttachments.find(label)->second->bind(bind_location);
    }
}

void GBuffer::bindDepthTexture( const int& bind_location ){
    mFbo->getDepthTexture()->bind(bind_location);
}

ci::gl::TextureRef GBuffer::getDepthTexture(){
    return mFbo->getDepthTexture();
}

float GBuffer::getWidth(){
    return mFbo->getWidth();
}
float GBuffer::getHeight(){
    return mFbo->getHeight();
}
