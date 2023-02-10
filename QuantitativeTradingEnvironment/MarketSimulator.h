//
//  MarketSimulator.h
//  QuantitativeTradingEnvironment
//
//  Created by Shashank Choudhary on 2/6/23.
//

#ifndef MarketSimulator_h
#define MarketSimulator_h

#include "AppBase.h"

//The Market Simulator is here to simulate the response from the market. In our implementation we will reduce the functionality for now to accept a limit order and fills it. It will reject any orders whose quantity is lower 1000.

class MarketSimulator : public AppBase
{
public:
    unsigned execution_id;
    set<string> list_symbols;
    double pnl;
    MarketSimulator(std::queue<Order> &strategy_to_ordermanager_,
        std::queue<ExecutionOrder> &ordermanager_to_strategy_,
        std::queue<Order> &ordermanager_to_simulator_,
        std::queue<ExecutionOrder> &simulator_to_ordermanager_, std::queue<BookUpdate>&bookbuilder_to_strategy_):
    AppBase(strategy_to_ordermanager_,
        ordermanager_to_strategy_,
        ordermanager_to_simulator_,
            simulator_to_ordermanager_,bookbuilder_to_strategy_), execution_id{0}, pnl{0}
    {
        //list_symbols  = {"MSFT", "AAPL" ,"LUV" , "SKYW","UAL","HA","RYAAY","DAL"};
        list_symbols = {"EUR/USD"};
    }
    
    bool handle_order();
    virtual void start(){ is_working = true;}
    virtual void stop(){ is_working = false;}
    double get_pnl()const { return pnl;}
    
    
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
        pnl -= (o.isBuy() ? 1: -1) * o.getPrice() * o.getQuantity();
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


#endif /* MarketSimulator_h */
