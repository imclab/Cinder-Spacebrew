#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"

#include "ciSpacebrew.h"
#include <boost/signals2.hpp>
#include <boost/lambda/lambda.hpp>

using namespace ci;
using namespace std;

class BasicExampleApp : public AppNative {
  public:
    void prepareSettings( cinder::app::App::Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    void onMessage( Spacebrew::Message msg );
    
    bool                        bDrawIcon;
    float                       mBgColor;
    Spacebrew::Connection       mSpacebrew;
    gl::TextureRef              mTex;
};

void BasicExampleApp::prepareSettings(cinder::app::App::Settings *settings) {
    settings->setFrameRate( 60.0 );
}

void BasicExampleApp::setup() {
    setWindowSize( 525, 525 );
    
    bDrawIcon   = true;
    mBgColor    = 0.0f;
    mTex        = gl::Texture::create( loadImage( loadAsset("cinderIcon.png") ) );
    
    string host = "ws://localhost:" + to_string( Spacebrew::SPACEBREW_PORT );
    string name = "cinder-button-example";
    string description = "Cinder <3 spacebrew";
    
    mSpacebrew.addPublish( "greeting", Spacebrew::TYPE_STRING );
    mSpacebrew.addSubscribe( "backgroundColor", Spacebrew::TYPE_RANGE );
    mSpacebrew.addSubscribe( "drawIcon", Spacebrew::TYPE_BOOLEAN );
    
    // Connect to websocket host
    mSpacebrew.connect( host, name, description );
    
    // Connect directly to signals or use the tempalted helper methods for events
    // Signals include signalOnMessage, signalOnConnect, signalOnDisconnect, signalOnError, signalOnInterrupt, signalOnPing

    mSpacebrew.addListener( &BasicExampleApp::onMessage, this );
    
    gl::enableAlphaBlending();
}

void BasicExampleApp::onMessage( Spacebrew::Message msg ) {
    if( msg.type == Spacebrew::TYPE_BOOLEAN ){
        bDrawIcon = msg.valueBoolean();
    }else if( msg.type == Spacebrew::TYPE_RANGE ){
        mBgColor = msg.valueRange() / 1023.0f;
    }
}

void BasicExampleApp::mouseDown( MouseEvent event ) {
    mSpacebrew.sendString("greeting", "hello there!");
}

void BasicExampleApp::update() {
}

void BasicExampleApp::draw() {
	gl::clear( Color( mBgColor, mBgColor, mBgColor ) );
    
    if( bDrawIcon ){
        gl::draw( mTex );
    }
}

CINDER_APP_NATIVE( BasicExampleApp, RendererGl )
