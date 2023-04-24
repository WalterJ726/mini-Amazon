#include <assert.h>
#include <iostream>
#include <pqxx/pqxx>
#include <set>
#include <string>
#include <utility>

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
  bool insert_and_update_order();
  bool insert_and_update_inventory(const int wh_id, const int p_id, const int quantity);
  
  // update package status
  bool update_package_status(const int ship_id, const string status);
  
  // disconnect
  void disconnect();
};

#endif  //_QUERY_FUNCS_
