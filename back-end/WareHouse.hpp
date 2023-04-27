#ifndef _WAREHOUSE_HPP
#define _WAREHOUSE_HPP
#include <string>
#include <vector>
#define NUM_WH 5
#define NUM_PRODUCT 5
#define PRODUCT_INIT_NUM 5
static std::vector<std::string> p_name_lists = {"apple", "banana", "iPad", "iPhone", "Mattress"};
class Product{
public:
    int p_id;
    std::string p_name;
    int p_num;
};


// define Ware House
class WareHouse{
public:
    int wh_id;
    int loc_x;
    int loc_y;
    Product products;

    explicit WareHouse(int idx){
        products.p_num = PRODUCT_INIT_NUM;
        products.p_id = idx;
        products.p_name = p_name_lists[idx];
    };
};

#endif // _HANDLEPROTO_HPP