//
//  ofxSpacebrew.cpp
//  ofxSpacebrew
//
//  Created by Brett Renfer on 10/30/12.
//
//

#include "ciSpacebrew.h"

namespace Spacebrew {
    
#pragma mark Message
    
    //--------------------------------------------------------------
    Message::Message( string _name, string _type, string _val){
        name = _name;
        type = _type;
        _default = value = _val;
    }
    
    //--------------------------------------------------------------
    string Message::getJSON( string configName ){
        if ( type == "string" || type == "boolean" ){
            return "{\"message\":{\"clientName\":\"" + configName +"\",\"name\":\"" + name + "\",\"type\":\"" + type + "\",\"value\":\"" + value +"\"}}";
        } else {
            return "{\"message\":{\"clientName\":\"" + configName +"\",\"name\":\"" + name + "\",\"type\":\"" + type + "\",\"value\":" + value +"}}";
        }
    }
    
    //--------------------------------------------------------------
    bool Message::valueBoolean(){
        if ( type != "boolean" ) console() << "This Message is not a boolean type! You'll most likely get 'false'" << endl;;
        return value == "true";
    }
    
    //--------------------------------------------------------------
    int Message::valueRange(){
        if ( type != "range" ) console() << "This Message is not a range type! Results may be unpredictable" << endl;
        return ci::math<int>::clamp( fromString<int>(value), 0, 1023);
    }
    
    //--------------------------------------------------------------
    string Message::valueString(){
        if ( type != "string" ) console() << "This Message is not a string type! Returning raw value as string." << endl;
        return value;
    }
    
#pragma mark Config
    
    //--------------------------------------------------------------
    void Config::addSubscribe( string name, string type ){
        subscribe.push_back( Message(name, type) );
    }
    
    //--------------------------------------------------------------
    void Config::addSubscribe( Message m ){
        subscribe.push_back(m);
    }

    //--------------------------------------------------------------
    void Config::addPublish( string name, string type, string def){
        publish.push_back( Message(name, type, def) );
    }
    
    //--------------------------------------------------------------
    void Config::addPublish( Message m ){
        publish.push_back(m);
    }
    
    //--------------------------------------------------------------
    string Config::getJSON(){
        string message = "{\"config\": {\"name\": \"" + name +"\",\"description\":\"" + description +"\",\"publish\": {\"messages\": [";
        
        for (int i=0, len=publish.size(); i<len; i++){
            message += "{\"name\":\"" + publish[i].name + "\",";
            message += "\"type\":\"" + publish[i].type + "\",";
            message += "\"default\":\"" + publish[i].value + "\"";
            message += "}";
            if ( i+1 < len ){
                message += ",";
            }
        }
        
        message += "]},\"subscribe\": {\"messages\": [";
        
        for (int i=0, len=subscribe.size(); i<len; i++){
            message += "{\"name\":\"" + subscribe[i].name + "\",";
            message += "\"type\":\"" + subscribe[i].type + "\"";
            message += "}";
            if ( i+1 < len ){
                message += ",";
            }
        }
        
        message += "]}}}";
        
        return message;
    }
    
#pragma mark Connection
    
    //--------------------------------------------------------------
    Connection::Connection(){
        bConnected = false;
        
        console() << "Adding callbacks " << endl;
    #ifdef SPACEBREW_USE_OFX_LWS
        mClient.addConnectCallback( &Connection::onConnect, this );
        mClient.addDisconnectCallback( &Connection::onDisconnect, this );
        mClient.addErrorCallback( &Connection::onError, this );
        mClient.addInterruptCallback( &Connection::onInterrupt, this );
        mClient.addPingCallback( &Connection::onPing, this );
        mClient.addReadCallback( &Connection::onRead, this );
    #endif

//        ofAddListener( ofEvents().update, this, &Connection::update );
        reconnectInterval = 2000;
        bAutoReconnect    = false;
    }

    //--------------------------------------------------------------
    Connection::~Connection(){
        bConnected = false;
        bAutoReconnect = false;
//        ofRemoveListener( ofEvents().update, this, &Connection::update );
        
#ifdef SPACEBREW_USE_OFX_LWS
        mClient.disconnect();
#endif
    }
    
    //--------------------------------------------------------------
    void Connection::update(){
        mClient.poll();
        

        if ( bAutoReconnect ){
            if ( !bConnected && getElapsedSeconds() * 1000 - lastTimeTriedConnect > reconnectInterval ){
                lastTimeTriedConnect = getElapsedSeconds() * 1000;
                connect( host, config );
            }
        }
    }

    //--------------------------------------------------------------
    void Connection::connect( string _host, string name, string description){
        host = _host;
        config.name = name;
        config.description = description;

    #ifdef SPACEBREW_USE_OFX_LWS
        
        string addr = "ws://" + host + ":" + toString(SPACEBREW_PORT);
        
        console() << "Attempting connection :: " << addr << endl;
        mClient.connect( addr );
    #endif
    }
    
    //--------------------------------------------------------------
    void Connection::connect( string host, Config _config ){
        config = _config;
        string addr = "ws://" + host + ":" + toString(SPACEBREW_PORT);
        console() << "Attempting connection :: " << addr << endl;
    #ifdef SPACEBREW_USE_OFX_LWS
        mClient.connect( addr );
    #endif
    }
    
    //--------------------------------------------------------------
    void Connection::send( string name, string type, string value ){
        if ( bConnected ){
            Message m( name, type, value);
            send(m);
        } else {
            console() << "Send failed, not connected!" << endl;
        }
    }

    //--------------------------------------------------------------
    void Connection::sendString( string name, string value ){
        if ( bConnected ){
            Message m( name, "string", value);
            send(m);
        } else {
            console() <<  "Send failed, not connected!" << endl;
        }
    }

    //--------------------------------------------------------------
    void Connection::sendRange( string name, int value ){
        if ( bConnected ){
            Message m( name, "range", toString( value ) );
            send(m);
        } else {
            console() << "Send failed, not connected!" << endl;
        }
    }

    //--------------------------------------------------------------
    void Connection::sendBoolean( string name, bool value ){
        if ( bConnected ){
            string out = value ? "true" : "false";
            Message m( name, "boolean", out);
            send(m);
        } else {
            console() << "Send failed, not connected!" << endl;
        }
    }

    //--------------------------------------------------------------
    void Connection::send( Message m ){
        if ( bConnected ){
        #ifdef SPACEBREW_USE_OFX_LWS
            mClient.write( m.getJSON( config.name ) );
        #endif
        } else {
            console() << "Send failed, not connected!" << endl;
        }
    }

    //--------------------------------------------------------------
    void Connection::send( Message * m ){
        if ( bConnected ){
        #ifdef SPACEBREW_USE_OFX_LWS
            mClient.write( m->getJSON( config.name ) );
        #endif
        } else {
            console() << "Send failed, not connected!" << endl;
        }
    }
    
    //--------------------------------------------------------------
    void Connection::addSubscribe( string name, string type ){
        config.addSubscribe(name, type);
        if ( bConnected ){
            updatePubSub();
        }
    }
    
    //--------------------------------------------------------------
    void Connection::addSubscribe( Message m ){
        config.addSubscribe(m);
        if ( bConnected ){
            updatePubSub();
        }
    }
    
    //--------------------------------------------------------------
    void Connection::addPublish( string name, string type, string def){
        config.addPublish(name, type, def);
        if ( bConnected ){
            updatePubSub();
        }
    }
    
    //--------------------------------------------------------------
    void Connection::addPublish( Message m ){
        config.addPublish(m);
        if ( bConnected ){
            updatePubSub();
        }
    }

    //--------------------------------------------------------------
    Config * Connection::getConfig(){
        return &config;
    }
    
    //--------------------------------------------------------------
    bool Connection::isConnected(){
        return bConnected;
    }

    //--------------------------------------------------------------
    void Connection::setAutoReconnect( bool _bAutoReconnect ){
        bAutoReconnect = _bAutoReconnect;
    }

    //--------------------------------------------------------------
    void Connection::setReconnectRate( int reconnectMillis ){
        reconnectInterval = reconnectMillis;
    }

    //--------------------------------------------------------------
    bool Connection::doesAutoReconnect(){
        return bAutoReconnect;
    }

    //--------------------------------------------------------------
    void Connection::updatePubSub(){
#ifdef SPACEBREW_USE_OFX_LWS
        mClient.write( config.getJSON() );
#endif
    }
    
    //--------------------------------------------------------------
    string Connection::getHost(){
        return host;
    }

#ifdef SPACEBREW_USE_OFX_LWS
    
    //--------------------------------------------------------------
    void Connection::onConnect(){
        console() << "Connected!" << endl;
        bConnected = true;
        updatePubSub();
    }
    
    //--------------------------------------------------------------
    void Connection::onDisconnect(){
        bConnected = false;
        lastTimeTriedConnect = getElapsedSeconds() * 1000;
    }
    
    void Connection::onError( std::string msg ) {
        console() << "Error :: " << msg << endl;
    }
    
    void Connection::onPing() {
        
    }
    
    void Connection::onInterrupt() {
        
    }
    
    //--------------------------------------------------------------
//    void Connection::onIdle( ofxLibwebsockets::Event& args ){}
    
    //--------------------------------------------------------------
    void Connection::onRead( std::string msg ){
        console() << "I received a message!" << endl;
        /*
        if ( !args.json.isNull() ){
            Message m;
            m.name = args.json["message"]["name"].asString();
            m.type = args.json["message"]["type"].asString();
            
            string type = args.json["message"]["type"].asString();
            if ( type == "string" && args.json["message"]["value"].isString()){
                m.value = args.json["message"]["value"].asString();
            } else if ( type == "boolean" ){
                if ( args.json["message"]["value"].isInt() ){
                    m.value = ofToString( args.json["message"]["value"].asInt());
                } else if ( args.json["message"]["value"].isString() ){
                    m.value = args.json["message"]["value"].asString();
                }
            } else if ( type == "number" ){
                if ( args.json["message"]["value"].isInt() ){
                    m.value = ofToString( args.json["message"]["value"].asInt());
                } else if ( args.json["message"]["value"].isString() ){
                    m.value = args.json["message"]["value"].asString();
                }
            } else {
                stringstream s; s<<args.json["message"]["value"];
                m.value = s.str();
            }
            
            if ( bConnected ) ofNotifyEvent(onMessageEvent, m, this);
        }
         */
    }
    
    //--------------------------------------------------------------

#endif
}