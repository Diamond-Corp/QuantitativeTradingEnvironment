//
//  Order.cpp
//  TradingSystemEnvironment
//
//  Created by Shashank Choudhary on 2/3/23.
//

#include "Order.hpp"

Order::Order(long _timestamp, bool is_buy_, unsigned int _id, unsigned int _price, unsigned int _quantity, const char * _venue, const char *_symbol , ordertype _type): timestamp{_timestamp} , is_buy{is_buy_} , id{_id} , price{_price}, quantity{_quantity}, type{_type}
{
    strcpy(venue, _venue);
    strcpy(symbol, _symbol);
}

Order::Order(const Order& o1)
{
    timestamp = o1.timestamp;
    id = o1.id;
    price = o1.price;
    quantity = o1.quantity;
    strcpy(venue, o1.venue);
    strcpy(symbol, o1.symbol);
    type = o1.type;
    
}

const char * Order::getVenue() const { return venue;}
const char * Order::getSymbol() const { return symbol; }
unsigned int Order::getId() const { return id;}
ordertype Order::getOrderType() const { return type ;}
unsigned int Order::getQuantity() const{ return quantity;}
unsigned int Order::getPrice() const { return price;}
bool Order::is_valid() {
    return quantity >= 0 && price >= 0 && (venue != NULL && venue[0] != '\0');
}
void Order::setVenue(const char * _venue)
{
    strcpy(venue, _venue);
}
void Order::setQuantity(unsigned int _quantity)
{
    quantity = _quantity;
}

void Order::setType(ordertype e)
{
    type = e;
}

void Order::setPrice(unsigned int _price)
{
    price = _price;
}

void Order::setSide(bool is_buy_)
{
    is_buy = is_buy_;
}

void Order::setOrderId(unsigned int _id)
{
    id = _id;
}

void Order::setSymbol(const char *_symbol)
{
    strcpy(symbol, _symbol);
}

unsigned int Order::getTimeStamp() const {return timestamp;}

//--------ClosedOrder ---------


ClosedOrder::ClosedOrder(long _timestamp, bool is_buy_, unsigned int _id, unsigned int _price, unsigned int _quantity, const char * _venue, const char *_symbol , ordertype _type): Order(_timestamp, is_buy_, _id, _price, _quantity, _venue, _symbol, _type){}

OpenOrder::OpenOrder(long _timestamp, bool is_buy_, unsigned int _id, unsigned int _price, unsigned int _quantity, const char * _venue, const char *_symbol , ordertype _type): Order(_timestamp, is_buy_, _id, _price, _quantity, _venue, _symbol, _type){}


VectorOrders::VectorOrders(unsigned int _capacity): capacity(_capacity)
{
    orders = new Order*[_capacity];
    for(int i = 0 ; i < _capacity ; ++i)
        orders[i] = nullptr;
    current_new_order_offset = 0;
}

VectorOrders::VectorOrders(const VectorOrders& v)
{
    capacity = v.get_capacity();
    orders = new Order*[capacity];
    current_new_order_offset = v.get_size();
    for(int i = 0 ; i < current_new_order_offset ; ++i)
        orders[i] = new Order(*v.orders[i]);
    for(int i =  current_new_order_offset ; i < capacity ; ++i)
        orders[i] = nullptr;
    
    
}

Order** VectorOrders::get_order_list() const{ return orders; }

bool VectorOrders::double_list_orders_size()
{
    if(capacity > INT_MAX /2)
        return false;
    Order **tmp = orders;
    orders = new Order*[2*capacity];
    for(int i = 0; i < capacity ; ++i)
        orders[i] = tmp[i];
    for(int i = capacity ; i < 2*capacity ; ++i)
        orders[i] = nullptr;
    capacity = capacity*2;
    delete tmp;
    return true;
        
}

bool VectorOrders::add_order(Order *o)
{
    if(orders[current_new_order_offset] != nullptr)
        return false;
    for(int i = 0 ; i < current_new_order_offset ; ++i)
        if(orders[i]->getId() == o->getId())
            return false;
    orders[current_new_order_offset] = o;
    ++current_new_order_offset;
    if(current_new_order_offset == capacity)
        double_list_orders_size();
    return true;
}
unsigned int VectorOrders::get_size() const {return current_new_order_offset; }
unsigned int VectorOrders::get_capacity()const { return capacity;}
void VectorOrders::clear()
{
    for(int i = 0; i < current_new_order_offset ; ++i)
    {
        delete orders[i];
        orders[i] = nullptr;
    }
    current_new_order_offset  = 0;
}
bool VectorOrders::delete_order(unsigned int id)
{
    int flag = 0, loc = 0;
    for(int i = 0; i < current_new_order_offset ; ++i)
    {
        if(orders[i]->getId() == id)
        {
            flag = 1;
            loc = i;
            delete orders[i];
            break;
        }
    }
    if(flag == 0)
        return false;
    else
    {
        for(int i = loc ; i < current_new_order_offset - 1 ; ++i)
            orders[i] = orders[i+1];
    }
    orders[current_new_order_offset - 1] = nullptr ;
    --current_new_order_offset;
    return true;
    
}

int VectorOrders::get_total_volume()
{
    int res = 0;
    for(int i = 0 ; i < current_new_order_offset ; ++i)
        res += orders[i]->getQuantity();
    return res;
}
int VectorOrders::get_total_outstanding_volume()
{
    int res = 0;
    for(int i = 0 ; i < current_new_order_offset ; ++i)
        res += orders[i]->getOutstandingQuantity();
    return res;
}












