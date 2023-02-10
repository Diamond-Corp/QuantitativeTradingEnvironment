//
//  Order.hpp
//  TradingSystemEnvironment
//
//  Created by Shashank Choudhary on 2/3/23.
//

#ifndef Order_hpp
#define Order_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <map>

using namespace std;

typedef unsigned int price_t;
typedef unsigned int quantity_t;
typedef unsigned int level_t;

enum ordertype {LIMIT, MARKET};
enum orderstate {OPEN, ACKNOWLEDGED, FILLED, CANCELLED, REJECTED};

class Order
{
protected:
    long timestamp; // epoch time : number of seconds elapsed since 00:00:00 coordinated universal time (UTC) , Thursday 1 January 1970
    bool is_buy ; // buy or sell
    unsigned int id; //order_id - unique identifier
    unsigned int price;
    unsigned int quantity;
    char venue[20]; // company name where is order is coming from
    char symbol[20];
    ordertype type;
    
public:
    Order(long _timestamp = 0, bool is_buy_ = true, unsigned int _id = 0, unsigned int _price = 0, unsigned int _quantity = 0, const char * _venue = " ", const char *_symbol = " ", ordertype _type = ordertype::MARKET);
    Order(const Order& o1);
    const char *getVenue() const;
    const char *getSymbol() const;
    unsigned int getId() const;
    ordertype getOrderType() const;
    unsigned int getQuantity() const;
    unsigned int getPrice() const;
    bool is_valid();
    void setVenue(const char * _venue);
    void setQuantity(unsigned int _quantity);
    virtual int getOutstandingQuantity(){ return quantity;} // to be modified by derived classes
    virtual ~Order(){};
    void setType(ordertype e);
    void setPrice(unsigned int _price);
    void setSide(bool);
    void setOrderId(unsigned int _id);
    virtual void setAction(unsigned int){};
    void setSymbol(const char* _symbol);
    unsigned int getTimeStamp() const;
    bool isBuy() const { return is_buy;}
    
    
};

// An open order is a Limit Order which sits on the market. This order has been acknowledged and can be potentially executed. The quantity which can be filled is called the outstanding volume. If an order is closed, it means the order doesn’t side on the market. As a result the outstanding volume will be 0.

class ClosedOrder: public Order
{
public:
    
    ClosedOrder(long _timestamp, bool is_buy_, unsigned int _id, unsigned int _price, unsigned int _quantity, const char * _venue, const char *_symbol , ordertype _type);
    virtual int getOustandingQuantity() { return 0 ;}
    virtual ~ClosedOrder() {}
    
};

class OpenOrder : public Order
{
public :
    OpenOrder(long _timestamp, bool is_buy_, unsigned int _id, unsigned int _price, unsigned int _quantity, const char * _venue, const char *_symbol , ordertype _type);
    virtual int getOustandingQuantity() { return quantity ;}
    virtual ~OpenOrder() {}
    
};

class VectorOrders
{
private:
    Order **orders; // order arrays
    unsigned int capacity; // limited size of orders
    unsigned int current_new_order_offset; // offset in the array orders which will be
    // written when adding an order
    
public:
    VectorOrders(unsigned int _capacity);
    VectorOrders(const VectorOrders&); // copy constructor
    Order** get_order_list() const;
    bool double_list_orders_size(); // when number of orders filled reach capacity we create a new arrray list and copy the old orders and then push new orders
    bool add_order(Order *o);
    unsigned int get_size() const;
    unsigned int get_capacity()const;
    void clear() ; // remove order list and set offset to zero
    bool delete_order(unsigned int id);
    int get_total_volume();
    int get_total_outstanding_volume();
    void dump_list_orders();
    
     
    
};

enum order_action{add, modify, suppress};
enum message_type{none, logon, logout, new_order, execution, full_snapshot, heartbeat, incremental_snapshot};

// FIX protocol tags
enum fix {
    BeginString = 8,
    BodyLength = 9,
    MsgType = 35,
    MsgSeqNum = 34,
    SenderCompID = 49,
    SendingTime = 52,
    TargetCompID = 56,
    CheckSum = 10,
};
enum fix_snapshot {
    Price_ = 270,
    OrderQty_ = 271,
};
enum fix_logon {
    EncryptMethod = 98,
    HeartBtInt = 108,
    ResetSeqNumFlag = 141,
};
enum fix_order {
    CIOrdID = 11,
    HandlInst = 21,
    OrderQty = 38,
    OrdType = 40,
    Price = 44,
    Side = 54,
    Symbol = 55,
    TransactTime = 60,
};


// PriceUpdate class stores the data into a PriceUdpate object after we parse the message from the exchange using the Parser class

class PriceUpdate: public Order
{
private:
    order_action action;
public:
    PriceUpdate(): action(order_action::modify){}
    order_action getAction() {return action; }
    void setAction(order_action oe) { action = oe ;}
    void setPriceforUpdate(unsigned int _price){ setPrice(_price);}

};

// The Message data structure will contain the type of message which has been parsed, and will also have three attributes: message_type (logon, logout, new_order,...) and Order and PriceUpdate

class Message
{
private:
    message_type type;
    Order order;
    PriceUpdate price_update;
    std::string stringrep;

public:
    Message() : type(message_type::none), order() , price_update(){}
    void setMessageType(message_type mt){type = mt;}
    void setMessageOrder(Order o){ order = o;}
    void setPriceUpdate(PriceUpdate pu){price_update = pu;}
    message_type getMessageType() {return type;}
    Order& getOrder() {return order;}
    PriceUpdate& getPriceUpdate() {return price_update;}
    void setStringRep(const char* st){stringrep = st;}
    std::string getStringRep(){ return stringrep;}


};

// The Execution Order contains the response from the market.
class ExecutionOrder : public Order
{
private:
    orderstate state;
    unsigned int execution_id;
    
public:
    ExecutionOrder(): Order(), state(orderstate::OPEN){}
    ExecutionOrder(const Order &o) : Order(o.getTimeStamp(), o.isBuy(), o.getId(), o.getPrice(), o.getQuantity(),o.getVenue(), o.getSymbol(), o.getOrderType()) {}
    orderstate getState() const { return state;}
    void setState(orderstate e ) {state = e;}
    void setExecutionID(unsigned int _id) { execution_id = _id;}
    unsigned int getExecutionID() const { return execution_id;}
};

// The signal will take the book updates as an entry and based on these books updates will generate an entry or exist signal.
class BookUpdate
{
private:
    level_t level;
    price_t price;
    quantity_t  quantity;
    char venue[20];
    char symbol[20];
    bool is_buy;
    unsigned int epoch_time;
    
public:
    BookUpdate(level_t _level , price_t _price, quantity_t _quantity , const char* _venue, const char* _symbol, bool _is_buy, unsigned int epoch_time_): level(_level), price(_price), quantity(_quantity), is_buy(_is_buy),epoch_time(epoch_time_)
    {
        strcpy(venue, _venue);
        strcpy(symbol, _symbol);
    }
    
    price_t get_price(){return price;}
    quantity_t get_quantity(){return quantity;}
    level_t get_level(){return level;}
    const char * get_venue()const {return venue;}
    bool get_is_buy(){return is_buy;}
    const char * get_symbol()const {return symbol;}
    unsigned int get_epoch_time() const { return epoch_time;}
    
    
    
    
    
};






#endif /* Order_hpp */
