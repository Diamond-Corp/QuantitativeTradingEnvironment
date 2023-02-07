//
//  TradingStrategy.h
//  QuantitativeTradingEnvironment
//
//  Created by Shashank Choudhary on 2/6/23.
//

#ifndef TradingStrategy_h
#define TradingStrategy_h

#include "AppBase.h"

class Signal
{
public:
    bool is_tradeable(BookUpdate &bu) { return bu.get_level() == 0 ? true : false;}
    
};

class Execution
{
private:
    Order e;
    bool tradeable;
public:
    Execution() : tradeable(false){}
    bool insert_order(long time_stamp_, bool is_buy_, unsigned int price_, unsigned int quantity_, const char *venue_, const char* symbol_, ordertype type_, unsigned int id_)
    {
        e.setSide(is_buy_);
        e.setType(type_);
        e.setOrderId(id_);
        e.setVenue(venue_);
        e.setSymbol(symbol_);
        e.setPrice(price_);
        e.setQuantity(quantity_);
        return true;
    }
    
    bool is_tradeable() { return tradeable;}
    void set_tradeable(bool is_tradeable){ tradeable = is_tradeable;}
    Order& get_order(){ return e;}
    
};

class TradingStrategy: public AppBase
{
private:
    Signal signal;
    Execution execution;
    int order_id;
    unordered_map<string, int> positions;
    unsigned int number_of_rejections;
    unsigned int number_of_fills;
    
    
    
    
public:
    TradingStrategy(
                std::queue<Order> &strategy_to_ordermanager_,
                std::queue<ExecutionOrder> &ordermanager_to_strategy_,
                std::queue<Order> &ordermanager_to_simulator_,
                std::queue<ExecutionOrder> &simulator_to_ordermanager_
        ):
        AppBase(strategy_to_ordermanager_,
                  ordermanager_to_strategy_,
                  ordermanager_to_simulator_,
                  simulator_to_ordermanager_),
                  signal(),
                  execution(),
                  order_id(1),
                  number_of_rejections(0),
                  number_of_fills(0){}
        virtual void start() {is_working=true;}
        virtual void stop() {
            positions.clear();
            is_working=false;
        }
    
    bool process_book_update(BookUpdate &bu);
    bool process_execution();
    bool process_market_response();
    
    int get_position(std::string symbol)
    {
        if (positions.find(symbol) != positions.end())
            return positions[symbol];
        else
            return 0;
    }
    unsigned int get_number_of_rejections() { return number_of_rejections; }
    unsigned int get_number_of_fills() { return number_of_fills; }
    void reset_position() { positions.clear(); }
    
    
};

bool TradingStrategy::process_book_update(BookUpdate &bu)
{
    if(!is_working) return false;
    if(signal.is_tradeable(bu))
    {
        execution.insert_order(0, bu.get_is_buy(), bu.get_price(), bu.get_quantity(), bu.get_venue(), bu.get_symbol(), ordertype::LIMIT, order_id++);
        execution.set_tradeable(true);
    }
    return process_execution();
}

bool TradingStrategy::process_execution()
{
    if(!is_working) return false;
    Order order;
    if(execution.is_tradeable())
    {
        order.setType(execution.get_order().getOrderType());
        order.setVenue(execution.get_order().getVenue());
        order.setQuantity(execution.get_order().getQuantity());
        order.setPrice(execution.get_order().getPrice());
        order.setOrderId(execution.get_order().getId());
        order.setSymbol(execution.get_order().getSymbol());
        order.setSide(execution.get_order().isBuy());
        execution.set_tradeable(false);
        strategy_to_ordermanager.push(order);
        
    }
    return true;
}

bool TradingStrategy::process_market_response()
{
    if(ordermanager_to_strategy.empty()) return true;
    ExecutionOrder &order = ordermanager_to_strategy.front();
    ordermanager_to_strategy.pop();
    
    if(order.getState() == orderstate::REJECTED)
        ++number_of_rejections;
    else if(order.getState() == orderstate::FILLED){
            ++number_of_fills;
            int position = (order.isBuy() ? 1 : -1) * order.getPrice() *
            order.getQuantity();
            if (positions.find(order.getSymbol()) != positions.end())
                positions[order.getSymbol()] += position;
            else
                positions[order.getSymbol()] = position;
    }
    return true;
}


    




#endif /* TradingStrategy_h */
