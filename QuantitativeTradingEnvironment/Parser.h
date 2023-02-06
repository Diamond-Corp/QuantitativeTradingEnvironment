//
//  Parser.h
//  TradingSystemEnvironment
//
//  Created by Shashank Choudhary on 2/5/23.
//

#ifndef Parser_h
#define Parser_h

#include "Order.hpp"

class Parser{
public :
    virtual bool parse(std::string message_text, Message& msg) { return false;};
    
};




class SEBXParser: public Parser
{
public:
    virtual bool parse(std::string message_text, Message& message)
    {
        int ind1 = 0;
        while(message_text[ind1] != '|') ++ind1;
        std::string smessage_type = message_text.substr(0,ind1);
        message_type mt;
        if(smessage_type == "X")
            mt = message_type::incremental_snapshot;
        else if(smessage_type == "LOGON")
            mt = message_type::logon;
        else if(smessage_type == "LOGOUT")
            mt = message_type::logout;
        else if(smessage_type == "NEWORDER")
            mt = message_type::new_order;
        else if(smessage_type == "W")
            mt = message_type::full_snapshot;
        else if(smessage_type == "HEARTBEAT")
            mt = message_type::heartbeat;
        else if(smessage_type == "EXECUTION")
            mt = message_type::execution;
        else
            mt = message_type::none;
        
        message.setMessageType(mt);
        
        switch (mt) {
//            case message_type::incremental_snapshot:
//                return parse_incrementtal_refresh(message_text, message);
            case message_type::new_order:
                return parse_new_order(message_text, message);

                
              default:
                return true;
        }
        return false;
    }
    
    bool parse_new_order(std::string message_text, Message& message)
    {
        // Sample SEBX Order
        message.getOrder().setPrice(123);
        message.getOrder().setQuantity(150);
        message.getOrder().setType(ordertype::LIMIT);
        message.getOrder().setSymbol("MQ");
        message.getOrder().setVenue("SEBX");
        return true;
    }
    
};

class FIX42Parser : public Parser
{
public:
    virtual bool parse(std::string message_text, Message &message)
    {
        std::size_t found = message_text.find("=35");
        std::string smessage_type = message_text.substr(found+3,1);
        message_type mt;
        if(smessage_type == "X")
            mt = message_type::incremental_snapshot;
        else if(smessage_type == "A")
            mt = message_type::logon;
        else if(smessage_type == "5")
            mt = message_type::logout;
        else if(smessage_type == "D")
            mt = message_type::new_order;
        else if(smessage_type == "W")
            mt = message_type::full_snapshot;
        else if(smessage_type == "0")
            mt = message_type::heartbeat;
        else if(smessage_type == "8")
            mt = message_type::execution;
        else
            mt = message_type::none;
        
        message.setMessageType(mt);
        switch(mt)
        {
            case message_type::incremental_snapshot:
                return parse_incremental_refresh(message_text, message);
            case message_type::logon:
                return parse_logon(message_text, message);
            case message_type::new_order:
                return parse_new_order(message_text, message);
            default:
                return true;
        }
        
        return false;
        
    }
    
    virtual bool parse_incremental_refresh(std::string message_text, Message &message);
    virtual bool parse_logon(std::string message_text, Message &message){return true;}
    virtual bool parse_new_order(std::string message_text, Message &message);
    
    
    
};

bool parse_incremental_refresh(std::string message_text, Message &message)
{
    std::string subs;
    int ind1 = 0;
    int ind2 = 0;
    order_action oe = static_cast<order_action>(stoi(message_text.substr(message_text.find("279=") + 4,1)));
    message.getPriceUpdate().setAction(oe);
    subs = message_text.substr(message_text.find("269=")+4,1);
    bool buy = !bool(stoi(subs));
    message.getPriceUpdate().setSide(buy);
    
    ind1 = message_text.find("270=") + 4;
    ind2 = ind1;
    while(isdigit(message_text[ind2])) ++ind2;
    subs = message_text.substr(ind1, ind2 -ind1);
    unsigned int price_ = stoi(subs);
    message.getPriceUpdate().setPrice(price_);
    
    ind1 = message_text.find("271=") + 4;
    ind2 = ind1;
    while(isdigit(message_text[ind2])) ++ind2;
    subs = message_text.substr(ind1, ind2 -ind1);
    unsigned int quant = stoi(subs);
    message.getPriceUpdate().setQuantity(quant);
    
    message.getPriceUpdate().setType(ordertype::LIMIT);
    return true;
    
    
}

bool parse_new_order(std::string message_text, Message &message)
{
    std::string subs;
    int ind1 = 0;
    int ind2 = 0;
    
    ind1 = message_text.find("11=") + 3;
    ind2 = ind1;
    while(isdigit(message_text[ind2])) ++ind2;
    subs = message_text.substr(ind1, ind2 -ind1);
    auto _id = stoi(subs);
    message.getOrder().setOrderId(_id);
    
    ind1 = message_text.find("38=") + 3;
    ind2 = ind1;
    while(isdigit(message_text[ind2])) ++ind2;
    subs = message_text.substr(ind1, ind2 -ind1);
    auto _quant= stoi(subs);
    message.getOrder().setQuantity(_quant);
    
    ordertype ot;
    ind1 = message_text.find("40=") + 3;
    if(message_text.substr(ind1,1) == "2")
        ot = ordertype::LIMIT;
    else if(message_text.substr(ind1,1) == "1")
        ot = ordertype::MARKET;
    else{
        std::cout << "Process order type error, setting default limit order \n";
        ot = ordertype::LIMIT;
    }
    
    message.getOrder().setType(ot);
    
    ind1 = message_text.find("44=") + 3;
    ind2 = ind1;
    while(isdigit(message_text[ind2])) ++ind2;
    subs = message_text.substr(ind1, ind2 -ind1);
    auto _price= stoi(subs);
    message.getOrder().setPrice(_price);
    
    ind1 = message_text.find("54=") + 3;
    if(message_text.substr(ind1,1) == "1")
        message.getOrder().setSide(true);
    else if(message_text.substr(ind1,1) == "2")
        message.getOrder().setSide(false);
    else{
        std::cout << "Process orderside error , setting default order side Buy \n";
        message.getOrder().setSide(true);
    }
    
    
    ind1 = message_text.find("55=") + 3;
    ind2 = ind1;
    while(isdigit(message_text[ind2])) ++ind2;
    subs = message_text.substr(ind1, ind2 -ind1);
    message.getOrder().setSymbol(subs.c_str());
    
    return true;
    

    
}









#endif /* Parser_h */
