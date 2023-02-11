//
//  BookBuilder.h
//  QuantitativeTradingEnvironment
//
//  Created by Shashank Choudhary on 2/10/23.
//

#ifndef BookBuilder_h
#define BookBuilder_h

#include "AppBase.h"
#include <sstream>
#include <chrono>
#include <string>

//market data reader
class MDReader
{
    const string delimeter;
    const unsigned int number_of_lines;
    const bool has_header;
    
public:
    MDReader(string delm = "", unsigned int number_of_lines_ =  10, bool has_header_ = true):
    delimeter{delm}, number_of_lines{number_of_lines_}, has_header{has_header_}{}
    
    //Parses through a csv file and returns data as a vector of strings
    vector<BookUpdate> getData();
    
};

vector<BookUpdate> MDReader::getData()
{
    unsigned int current_number_of_lines = 0;
    vector<BookUpdate> dataList;
    string line = "";
    // Iterate through each line and split the content using delimiter
    bool header_handled = false;
    while(getline(cin, line))
    {
        if(has_header && !header_handled)
        {
            header_handled = true;
            continue;
        }
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        //\r is a carriage return character; it tells your terminal emulator to move the cursor at the start of the line.
        vector<string> vec;
        stringstream s_stream(line);
        while(s_stream.good())
        {
            string substr;
            getline(s_stream, substr, ',');
            vec.push_back(substr);
        }
        
        struct tm t;
        time_t t_of_day;
        t.tm_year = stoi(vec[3]. substr(0,4)) - 1900;
        t.tm_mon = stoi(vec[3]. substr(5,2)) - 1;  // where jan = 0
        t.tm_mday = stoi(vec[3]. substr(8,2));
        t.tm_hour  = stoi(vec[3].substr(11, 2));
        t.tm_min   = stoi(vec[3].substr(14, 2));
        t.tm_sec   = stoi(vec[3].substr(17, 2));
        t.tm_isdst = -1 ; // Is DSt On? 1= yes, 0= no, -1 = unknown (daylightsaving)
        t_of_day = mktime(&t);
        
//        BookUpdate(level_t _level , price_t _price, quantity_t _quantity , const char* _venue, const char* _symbol, bool _is_buy, unsigned int epoch_time_): level(_level), price(_price), quantity(_quantity), is_buy(_is_buy),epoch_time(epoch_time_);
        BookUpdate b1(0, stoi(vec[4]), 100000,"GAIN", vec[2].c_str(), true, t_of_day);
        BookUpdate b2(0, stoi(vec[5]), 100000,"GAIN", vec[2].c_str(), false, t_of_day);
        
        dataList.push_back(b1);
        dataList.push_back(b2);
        if(number_of_lines != 0 && current_number_of_lines++ > number_of_lines)
            break;
    }
        
        
    return dataList;
}

class BookBuilder: public AppBase
{
public:
    BookBuilder(std::queue<Order> &strategy_to_ordermanager_,
        std::queue<ExecutionOrder> &ordermanager_to_strategy_,
        std::queue<Order> &ordermanager_to_simulator_,
        std::queue<ExecutionOrder> &simulator_to_ordermanager_, std::queue<BookUpdate>&bookbuilder_to_strategy_):
    AppBase(strategy_to_ordermanager_,
        ordermanager_to_strategy_,
        ordermanager_to_simulator_,
            simulator_to_ordermanager_,bookbuilder_to_strategy_){}
    virtual void start(){
        MDReader reader(",", 0);
        //Get data from csv
        std::vector<BookUpdate> dataList = reader.getData();
        //send book update to trading strategy
        for(auto &bu : dataList)
            bookbuilder_to_strategy.push(bu);
    }
    virtual void stop(){};
};


#endif /* BookBuilder_h */
