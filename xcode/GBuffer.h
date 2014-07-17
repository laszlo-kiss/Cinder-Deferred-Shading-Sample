//
//  GBuffer.h
//  Deffered_GBuffer
//
//  Created by Mike Allison on 7/17/14.
//
//

#pragma once
#include "cinder/gl/Fbo.h"

typedef std::shared_ptr<class GBuffer> GBufferRef;

class GBuffer {
public:
    
    struct Attachment {
    public:
        
        Attachment(const GLint& loc = GL_COLOR_ATTACHMENT0, const GLint& type = GL_RGBA32F, const std::string& label = "" ) : mLocation(loc), mType(type), mLabel(label){}
        
        Attachment& label( const std::string& label ){ mLabel = label; return *this; }
        Attachment& type( const GLint& type ){ mType = type; return *this; }
        Attachment& location( const int& location ){ mLocation = location; return *this; }
        
        const GLint& getLocation() { return mLocation; }
        const GLint& getType() { return mType; }
        const std::string& getLabel() { return mLabel; }

    protected:
        GLint mLocation;
        GLint mType;
        std::string mLabel;
        
    };
    
    struct Format {
        
    public:
        
        Format(){
        
            mFboFormat.setDepthBufferInternalFormat(GL_DEPTH_COMPONENT32);
            mFboFormat.setSamples(16);
            mFboFormat.enableDepthTexture();
            mAttachments.clear();
        
        }
        
        Format& attachment( const GLint& loc, const GLint& type, const std::string& label ){ mAttachments.push_back( Attachment( loc, type, label ) ); return *this; }
        
        ci::gl::Fbo::Format& getFboFormat(){ return mFboFormat; }
        
        std::list<Attachment> mAttachments;
        ci::gl::Fbo::Format mFboFormat;
        
    };

    static GBufferRef create(  const float& width, const float& height, const Format& format ) { return GBufferRef( new GBuffer( width, height, format ) ); }
    
    void bind();
    void unbind();
    
    ci::gl::TextureRef getTexture( const std::string& label );
    ci::gl::TextureRef getDepthTexture();

    void bindTexture( const int& bind_location, const std::string& label );
    void bindDepthTexture( const int& bind_location );
    
    float getWidth();
    float getHeight();
    
    ~GBuffer();
    
protected:
    
    GBuffer( const float& width, const float& height, const Format& format );
    std::map<std::string, ci::gl::TextureRef> mAttachments;
    ci::gl::FboRef mFbo;
    
};