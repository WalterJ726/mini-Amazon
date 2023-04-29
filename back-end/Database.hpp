#include <assert.h>
#include <iostream>
#include <pqxx/pqxx>
#include <set>
#include <string>
#include <utility>
#include <ctime>

// using pqxx::connection;
using namespace pqxx;
using std::pair;
using std::set;
using std::string;

#ifndef _QUERY_FUNCS_
#define _QUERY_FUNCS_

class Database {
 private:
  const string dbname;
  const string user;
  const string password;
  connection * c;
  Database();
  ~Database() {};
	Database(const Database&);
	Database& operator=(const Database&);
 public:

  // Singleton
	static Database& getInstance() {
    // adapted from Effective C++ using Singleton
		static Database instance; // magic static
		return instance;
	}
  connection * connect();

  //drop tables if already exist, and create tables
  void initialize();
  bool initialize_inventory(const int wh_id, const int p_id, const int quantity);
  // ~Database();

  // update warehouse and product
  bool insert_and_update_warehouse(const int wh_id, const int loc_x, const int loc_y);
  bool insert_and_update_product(const int p_id, const string& p_name, const string& p_description);

  // insert and update
  bool insert_order(const int order_num, const int product_id, const int user_id, const int quantity, const int package_id, const time_t & create_time);
  bool insert_and_update_inventory(const int wh_id, const int p_id, const int quantity);
  bool insert_package(const int package_id, const int owner_id, const int warehouse_id,const int dest_x, const int dest_y, const string & package_status);

  bool update_bind_status(const int user_id, const int ups_id, std::string bind_status);
  // update and check package status
  bool update_package_status(const int ship_id, const string status);
  bool check_package_status(const int ship_id, const string status);
  // query inventory and update if quantity matches 
  // NOTE USING SELECT FOR UPDATE, FUTURE MODIFICATION MAY NEEDED
  int match_inventory(const int product_id, const int quantity);
  
  // disconnect
  void disconnect();
};

#endif  //_QUERY_FUNCS_
