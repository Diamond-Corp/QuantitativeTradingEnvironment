//
//  AppBase.h
//  TradingSystemEnvironment
//
//  Created by Shashank Choudhary on 2/5/23.
//

#ifndef AppBase_h
#define AppBase_h

#include "Parser.h"
#include "Composer.h"
#include <map>
#include <string>



// When you create a large system with different components, you want to have common functionalities. Here AppBase will just be in charge of turning off and turning any components of the Trading System. That’s why you can see only the function start, stop changing the attribute is_working.

class Appbase
{
protected:
    bool is_working;
public:
    Appbase(){ is_working = false;}
    virtual void start() = 0;
    virtual void stop() = 0;
};

class Gateway : public Appbase
{
public :
    std::map<string, Parser*> list_parser;
    std::map<string, Composer*> list_composer;
    Message stored_message;
    Gateway(): stored_message(){}
    
    bool add_parser(string venue, Parser *parser)
    {
        if(is_working){
            if(list_parser.find(venue) == list_parser.end()){
                list_parser[venue] = parser;
                return true;
            }
        }
        return false;
    
    }
    
    bool add_composer(string venue, Composer *composer)
    {
        if(is_working){
            if(list_composer.find(venue) == list_composer.end()){
                list_composer[venue] = composer;
                return true;
            }
        }
        return false;
    
    }
    
    bool process_message_from_exchange_for_price_update(string venue, string message_to_parse)
    {
        if(is_working)
            return list_parser[venue]->parse(message_to_parse, stored_message);
        return false;
    }
    
    bool process_message_from_exchange_for_order(string venue, string message_to_parse)
    {
        if(is_working)
        {
            stored_message.getOrder().setVenue(venue.c_str());
            return list_parser[venue]->parse(message_to_parse, stored_message);
        }
        return false;
    }
    
    string send_message_for_order(string venue, Message& me)
    {
        string ret_string = list_composer[venue]->compose(me);
        return ret_string;
    }
    Message return_stored_message() { return stored_message;}
    
    
};

#endif /* AppBase_h */
