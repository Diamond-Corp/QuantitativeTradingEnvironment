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

class TradingStrategy: public AppBase
{
    
    
    
};



#endif /* TradingStrategy_h */
