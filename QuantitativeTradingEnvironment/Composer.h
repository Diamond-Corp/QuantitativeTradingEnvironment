//
//  Composer.h
//  TradingSystemEnvironment
//
//  Created by Shashank Choudhary on 2/5/23.
//

#ifndef Composer_h
#define Composer_h

#include "Order.hpp"



class Composer
{
public:
    virtual std::string compose(Message& message)
    {
        std::string return_string  = "";
        std::string price_string = std::to_string(message.getOrder().getPrice());
        std::string quant_string = std::to_string(message.getOrder().getQuantity());
        std::string symb = message.getOrder().getSymbol();
        return_string = "NEWORDER|" + price_string + "|" + quant_string + "|SEBX" + symb + "|";
        return return_string;
    };
    
};


class SEBXComposer : public Composer{};

class FIX42Composer : public Composer
{
    virtual std::string compose(Message & message)
    {
        std::string fixtype = "D";
        message_type mt = message.getMessageType();
        
        if(mt == message_type::incremental_snapshot){fixtype ="X";}
        else if(mt == message_type::logon){fixtype ="A";}
        else if(mt == message_type::logout){fixtype ="5";}
        else if(mt == message_type::new_order){fixtype ="D";}
        else if(mt == message_type::full_snapshot){fixtype ="W";}
        else if(mt == message_type::heartbeat){fixtype ="0";}
        else if(mt == message_type::execution){fixtype ="8";}
        
        string quant_string = to_string(message.getOrder().getQuantity());
        string price_string = to_string(message.getOrder().getPrice());
        string return_string = "";
        return_string = "8=FIX.4.2|9=118|35="+fixtype+ "|34=2|49=DONALD|52=20160613-22:52:37.227|56=VENUE|11=1|21=3|38= " +quant_string + " |40=2|44="+price_string+
        "|54=1|55=MQ|60=20160613-22:52:37.227|10=058|";
        
        return return_string;
        
    }
    
};

#endif /* Composer_h */
