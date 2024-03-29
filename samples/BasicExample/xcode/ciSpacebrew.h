//
//  SpaceBrew.h
//  ofxSpacebrew
//
//  Created by Brett Renfer on 8/17/12.
//  Cinder port Charlie Whitney
//

#pragma once

#include "WebSocketClient.h"

#include "cinder/Utilities.h"
#include "cinder/Json.h"
#include "cinder/CinderMath.h"

#include <boost/signals2.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

namespace Spacebrew {
    
    // Some useful constants
    static const int            SPACEBREW_PORT  = 9000;
    static const std::string    SPACEBREW_CLOUD = "sandbox.spacebrew.cc";
    static const std::string    TYPE_STRING     = "string";
    static const std::string    TYPE_RANGE      = "range";
    static const std::string    TYPE_BOOLEAN    = "boolean";
    
    /**
     * @brief Spacebrew message
     * @class Spacebrew::Message
     */
    class Message {
      public:
    
        /** @constructor */
        Message( string _name="", string _type="", string _val="");
        virtual string getJSON( string configName );
        
        /**
         * @brief Name of Message
         * @type {std::string}
         */
        string name;

        /**
         * @brief Message type ("string", "boolean", "range", or custom type)
         * @type {std::string}
         */
        string type;

        /**
         * @brief Default value
         * @type {std::string}
         */
        string _default;

        /**
         * @brief Current value (cast to string)
         * @type {std::string}
         */
        string value;
    
        /**
         * @brief Get your incoming value as a boolean
         */
        bool    valueBoolean();
    
        /**
         * @brief Get your incoming value as a range (0-1023)
         */
        int     valueRange();
    
        /**
         * @brief Get your incoming value as a string
         */
        string    valueString();
    
        friend ostream& operator<<(ostream& os, const Message& vec);
    };
    
    inline ostream& operator<<(ostream& os, const Message& m) {
        os << m.name << ", " << m.type << ", " << m.value;
        return os;
    }
    
    /**
     * @brief Wrapper for Spacebrew config message. Gets created automatically by
     * Spacebrew::Connection, but can sometimes be nice to use yourself.
     * @class Spacebrew::Config
     */
    class Config {
      public:
            
        // see documentation below
        // docs left out here to avoid confusion. Most people will use these methods
        // on Spacebrew::Connection directly
        void addSubscribe( string name, string type );
        void addSubscribe( Message m );
        void addPublish( string name, string type, string def);
        void addPublish( Message m );
        
        string getJSON();
        string name, description;
        
      private:
        
        vector<Message> publish;
        vector<Message> subscribe;
    };
    
    /**
     * @brief Main Spacebrew class, connected to Spacebrew server. Sets up socket, builds configs
     * and publishes ofEvents on incoming messages.
     * @class Spacebrew::Connection
     */
    class Connection {
      public:
        Connection();
        ~Connection();
    
        void setup();
        bool bSetup;
    
        /**
         * @brief Connect to Spacebrew. Pass empty values to connect to default host as "openFrameworks" app 
         * (use only for testing!)
         * @param {std::string} host        Host to connect to (e.g. "localhost", SPACEBREW_CLOUD ). Can be IP address OR hostname
         * @param {std::string} name        Name of your app (shows up in Spacebrew admin)
         * @param {std::string} description What does your app do?
         */
        void connect( string host = SPACEBREW_CLOUD, string name = "cinder app", string description = "");
        void connect( string host, Config _config );
        
        /**
         * @brief Send a message
         * @param {std::string} name    Name of message
         * @param {std::string} type    Message type ("string", "boolean", "range", or custom type)
         * @param {std::string} value   Value (cast to string)
         */
        void send( string name, string type, string value );

        /**
         * @brief Send a string message
         * @param {std::string} name    Name of message
         * @param {std::string} value   Value
         */
        void sendString( string name, string value );

        /**
         * @brief Send a range message
         * @param {std::string} name    Name of message
         * @param {int}         value   Value
         */
        void sendRange( string name, int value );

        /**
         * @brief Send a boolean message
         * @param {std::string} name    Name of message
         * @param {bool}        value   Value
         */
        void sendBoolean( string name, bool value );

        /**
         * Send a Spacebrew Message object
         * @param {Spacebrew::Message} m
         */
        void send( Message m );

        /**
         * @brief Send a Spacebrew Message object. Use this method if you've overridden Spacebrew::Message
         * (especially) if you've created a custom getJson() method!)
         * @param {Spacebrew::Message} m
         */
        void send( Message * m );
    
        /**
         * @brief Add a message that you want to subscribe to
         * @param {std::string} name    Name of message
         * @param {std::string} type    Message type ("string", "boolean", "range", or custom type)
         */
        void addSubscribe( string name, string type );

        /**
         * @brief Add a message that you want to subscribe to
         * @param {Spacebrew::Message} m
         */
        void addSubscribe( Message m );
        
        /**
         * @brief Add message of specific name + type to publish
         * @param {std::string} name Name of message
         * @param {std::string} typ  Message type ("string", "boolean", "range", or custom type)
         * @param {std::string} def  Default value
         */
        void addPublish( string name, string type, string def="");

        /**
         * @brief Add message to publish
         * @param {Spacebrew::Message} m
         */
        void addPublish( Message m );

        /**
         * @return Current Spacebrew::Config (list of publish/subscribe, etc)
         */
        Config * getConfig();
    
        /**
         * @return Are we connected?
         */
        bool isConnected();

        /**
         * @brief Turn on/off auto reconnect (try to connect when/if Spacebrew server closes)
         * @param {boolean} bAutoReconnect (true by default)
         */
        void setAutoReconnect( bool bAutoReconnect=true );

        /**
         * @brief How often should we try to reconnect if auto-reconnect is on (defaults to 1 second [1000 millis])
         * @param {int} reconnectMillis How often to reconnect, in milliseconds
         */
        void setReconnectRate( int reconnectMillis );

        /**
         * @return Are we trying to auto-reconnect?
         */
        bool doesAutoReconnect();

        /**
         * @return Current hostname
         */
        string getHost();
    
        void				connect();
        void				disconnect();
    
        void				onConnect();
        void				onDisconnect();
        void				onError( std::string msg );
        void				onInterrupt();
        void				onPing();
        void				onRead( std::string msg );
        void				write();
    
        void                update();
    
        boost::signals2::signal<void(Message)>  signalOnMessage;
        boost::signals2::signal<void(void)>     signalOnConnect;
        boost::signals2::signal<void(void)>     signalOnDisconnect;
        boost::signals2::signal<void(string)>   signalOnError;
        boost::signals2::signal<void(void)>     signalOnInterrupt;
        boost::signals2::signal<void(void)>     signalOnPing;
    
        template<typename T, typename Y>
        inline void addListener(T callback, Y *callbackObject) {
            signalOnMessage.connect(std::bind(callback, callbackObject, std::placeholders::_1));
        }
    
      protected:
        string host;
        bool bConnected;
        void updatePubSub();
    
        Config config;
        
        // reconnect
        bool bAutoReconnect;
        int  lastTimeTriedConnect;
        int  reconnectInterval;
    
        WebSocketClient		mClient;
    };
    
    /**
     * @brief Helper function to automatically add a listener to a connections onMessageEvent
     * @example 
     * Spacebrew::connection;
     * void onMessage( Message & e ){};
     *
     * void setup(){
     *      Spacebrew::addListener( this, connection);
     * }
     */
    template<class T, class SB>
    void addListener(T * app, SB & connection){
        ofAddListener( connection.onMessageEvent, app, &T::onMessage);
    }
        
    /**
     * @brief Helper function to remove onMessage listener
     */
    template<class T, class SB>
    void removeListener(T * app, SB & connection){
        ofRemoveListener( connection.onMessageEvent, app, &T::onMessage);
    }
}
