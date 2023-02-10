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
private:
    time_t current_time;
    double price;
    int count_5;
    int count_20;
    double moving_average_5;
    double moving_average_20;
    std::queue<pair<time_t, int>>past_5;
    std::queue<pair<time_t, int>>past_20;
    
public:
    Signal():current_time(0), price(0), count_5(0), count_20(0), moving_average_5{0}, moving_average_20{0}{}
    bool is_tradeable(BookUpdate &bu) { return bu.get_level() == 0 ? true : false;}
    void insert_book_update(BookUpdate bu);
    double get_5min_moving_average(){
        return 0.01 * static_cast<double>(static_cast<int>(moving_average_5 *100));
    }
    double get_20min_moving_average(){
        return 0.01 * static_cast<double>(static_cast<int>(moving_average_20 *100));
    }
    bool go_long() { return moving_average_5 > moving_average_20 ? true: false;}
    bool go_short(){return moving_average_5 < moving_average_20 ? true: false;}
    
    
};

void Signal::insert_book_update(BookUpdate bu)
{
    current_time = bu.get_epoch_time();
    price = bu.get_price();
    past_5.push({current_time, price});
    past_20.push({current_time, price});
    while(!past_5.empty() && past_5.front().first < current_time - 300)
    {
        if(count_5 == 1)
            moving_average_5 = 0;
        else
            moving_average_5 = (moving_average_5 *count_5 - past_5.front().second)/ (count_5 -1);
        --count_5;
        past_5.pop();
    }
    while(!past_20.empty() && past_20.front().first < current_time - 1200)
    {
        if(count_20 == 1)
            moving_average_20 = 0;
        else
            moving_average_20 = (moving_average_20 *count_20 - past_20.front().second)/ (count_20 -1);
        --count_20;
        past_20.pop();
    }
    
    moving_average_5 = (moving_average_5* count_5 + price) / (count_5 + 1);
    count_5++;
    moving_average_20 = (moving_average_20* count_20 + price) / (count_20 + 1);
    count_20++;
    
}

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
    double pnl;
    int pos ; // 0 : flat , 1: buy, -1 : sell
    
    
    
public:
    TradingStrategy(
                std::queue<Order> &strategy_to_ordermanager_,
                std::queue<ExecutionOrder> &ordermanager_to_strategy_,
                std::queue<Order> &ordermanager_to_simulator_,
                std::queue<ExecutionOrder> &simulator_to_ordermanager_,
                std::queue<BookUpdate> &bookbuilder_to_strategy_
        ):
        AppBase(strategy_to_ordermanager_,
                  ordermanager_to_strategy_,
                  ordermanager_to_simulator_,
                  simulator_to_ordermanager_,
                  bookbuilder_to_strategy_),
                  signal(),
                  execution(),
                  order_id(1),
                  number_of_rejections(0),
    number_of_fills(0), pnl{0.0}, pos{0}{}
        virtual void start() {is_working=true;}
        virtual void stop() {
            positions.clear();
            is_working=false;
        }
    
    bool process_book_update(BookUpdate &bu);
    bool process_book_update();
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
    int get_positions(string symbol)
    {
        if(positions.find(symbol)!= positions.end())
            return positions[symbol];
        else
            return 0;
    }
    double get_pnl(){return pnl;}
    
    
};

bool TradingStrategy::process_book_update(BookUpdate &bu)
{
    if(!is_working) return false;
    if(signal.is_tradeable(bu))
    {
//        execution.insert_order(0, bu.get_is_buy(), bu.get_price(), bu.get_quantity(), bu.get_venue(), bu.get_symbol(), ordertype::LIMIT, order_id++);
        signal.insert_book_update(bu);
        if(signal.go_long() && pos != 1) // long atmost one unit
        {
            execution.insert_order(0, true, bu.get_price(), bu.get_quantity(), bu.get_venue(), bu.get_symbol(), ordertype::LIMIT, order_id++);
        }
        else if(signal.go_short() && pos != -1 ) // short atmost 1 unit
        {
            execution.insert_order(0, false, bu.get_price(), bu.get_quantity(), bu.get_venue(), bu.get_symbol(), ordertype::LIMIT, order_id++);
        }
        else if(bookbuilder_to_strategy.empty() && pos ==1 ) //close long position
        {
            execution.insert_order(0, false, bu.get_price(), bu.get_quantity(), bu.get_venue(), bu.get_symbol(), ordertype::LIMIT, order_id++);
        }
        else if(bookbuilder_to_strategy.empty() && pos == -1 ) //close short position
        {
            execution.insert_order(0, true, bu.get_price(), bu.get_quantity(), bu.get_venue(), bu.get_symbol(), ordertype::LIMIT, order_id++);
        }
        else
            return true;
        execution.set_tradeable(true);
    }
    return process_execution();
}

bool TradingStrategy::process_book_update()
{
    if(!is_working) return false;
    if(bookbuilder_to_strategy.empty())
        return true;
    BookUpdate &bu = bookbuilder_to_strategy.front();
    bookbuilder_to_strategy.pop();
    return process_book_update(bu);
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
        
        int position = (order.isBuy() ?  1 :-1) *order.getPrice() *order.getQuantity();
        pnl -= position;
        
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
        pnl -= position;
    }
    return true;
}


    




#endif /* TradingStrategy_h */
