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
#include <queue>
#include <set>
#include <unordered_map>



// When you create a large system with different components, you want to have common functionalities. Here AppBase will just be in charge of turning off and turning any components of the Trading System. That’s why you can see only the function start, stop changing the attribute is_working.

class AppBase
{
protected:
    bool is_working;
    std::queue<Order> &strategy_to_ordermanager;
    std::queue<ExecutionOrder> &ordermanager_to_strategy;
    std::queue<Order> &ordermanager_to_simulator;
    std::queue<ExecutionOrder> &simulator_to_ordermanager;
public:
   // Appbase(){ is_working = false;}
    AppBase(std::queue<Order> &strategy_to_ordermanager_,
        std::queue<ExecutionOrder> &ordermanager_to_strategy_,
        std::queue<Order> &ordermanager_to_simulator_,
        std::queue<ExecutionOrder> &simulator_to_ordermanager_):
                strategy_to_ordermanager(strategy_to_ordermanager_),
                ordermanager_to_strategy(ordermanager_to_strategy_),
                ordermanager_to_simulator(ordermanager_to_simulator_),
                simulator_to_ordermanager(simulator_to_ordermanager_)
        {
            is_working=false;
        }
    virtual void start() = 0;
    virtual void stop() = 0;
};

class Gateway : public AppBase
{
public :
    std::map<string, Parser*> list_parser;
    std::map<string, Composer*> list_composer;
    Message stored_message;
   // Gateway(): stored_message(){}
    
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

//The Market Simulator is here to simulate the response from the market. In our implementation we will reduce the functionality for now to accept a limit order and fills it. It will reject any orders whose quantity is lower 1000.

class MarketSimulator : public AppBase
{
public:
    unsigned execution_id;
    set<string> list_symbols;
    MarketSimulator(std::queue<Order> &strategy_to_ordermanager_,
        std::queue<ExecutionOrder> &ordermanager_to_strategy_,
        std::queue<Order> &ordermanager_to_simulator_,
        std::queue<ExecutionOrder> &simulator_to_ordermanager_):
    AppBase(strategy_to_ordermanager_,
        ordermanager_to_strategy_,
        ordermanager_to_simulator_,
            simulator_to_ordermanager_), execution_id{0}
    {
        list_symbols  = {"MSFT", "AAPL" ,"LUV" , "SKYW","UAL","HA","RYAAY","DAL"};
    }
    
    bool handle_order();
    virtual void start(){ is_working = true;}
    virtual void stop(){ is_working = false;}
    
    
};

bool MarketSimulator::handle_order()
{
    if(!is_working) return false;
    if(ordermanager_to_simulator.empty())
        return true;
    const Order&o = ordermanager_to_simulator.front();
    ordermanager_to_simulator.pop();
    const bool is_tradeable = list_symbols.find(o.getSymbol()) != list_symbols.end();
    ExecutionOrder new_execution(o);
    if(is_tradeable)
    {
        new_execution.setState(o.getQuantity() > 1000 ? orderstate::ACKNOWLEDGED : orderstate::REJECTED);
        new_execution.setExecutionID(++execution_id);
        simulator_to_ordermanager.push(new_execution);
        new_execution.setSide(orderstate::FILLED);
        std::cout << "simulator push a fill | " << new_execution.getPrice() << "|" <<
        new_execution.isBuy() << "|" << new_execution.getQuantity() <<"|" << endl;
        simulator_to_ordermanager.push(new_execution);
    }
    else
    {
        cout << "simulator push a reject \n";
        new_execution.setState(orderstate::REJECTED);
        new_execution.setExecutionID(++execution_id);
        simulator_to_ordermanager.push(new_execution);
    }
    
    return true;
    
}


#endif /* AppBase_h */
