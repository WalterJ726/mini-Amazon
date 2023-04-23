#ifndef _WAREHOUSE_HPP
#define _WAREHOUSE_HPP
#include <string>
#include <vector>
#define NUM_WH 5
#define NUM_PRODUCT 3
#define PRODUCT_INIT_NUM 5


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
    std::vector<Product> products;

    WareHouse(){
        for (int j = 0; j < NUM_PRODUCT; j ++ ){
            Product product;
            product.p_num = PRODUCT_INIT_NUM;
            product.p_id = j;
            product.p_name = std::to_string(j);
            products.push_back(product);
        }
    };
};

#endif // _HANDLEPROTO_HPP