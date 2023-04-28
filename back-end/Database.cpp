#include "Database.hpp"

#include <ctime>
#include <sstream>
#include <vector>
using std::stringstream;
using std::vector;

Database::Database() :
    dbname("miniamazon"), user("postgres"), password("passw0rd"), c(NULL) {
}

connection * Database::connect() {
  connection * c = NULL;
  stringstream ss_connect;
  ss_connect << "host=vcm-30819.vm.duke.edu port=5432 dbname=" << this->dbname << " user=" << this->user
             << " password=" << this->password;
  try {
    c = new connection(ss_connect.str());
    if (c->is_open()) {
      std::cout << "Opened database successfully: " << c->dbname() << std::endl;
      this->c = c;
      return c;
    }
    else {
      std::cout << "Can't open database" << std::endl;
      return NULL;
    }
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << std::endl;
    return NULL;
  }
}

result executeSQL(connection * c, const string & sql) {
  try {
    work w(*c);
    // w.exec("set transaction isolation level serializable");
    result r(w.exec(sql));
    w.commit();
    return r;
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    std::cout << "fail to execute SQL" << std::endl;
  }
  return result();  // if an error occurs, return an empty result
}

// select part for the use of "select for update"
result select_for_update(const string & sql, work & w){
    try{
      result r(w.exec(sql));
      if (r.empty()){
        std::cout << "No rows selected." << std::endl;
      }
      std::cout << "selected for updated has founded results" << std::endl;
      return r;
    }
    catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    std::cout << "fail to execute SQL select_for_update" << std::endl;
  }
  return result();  // if an error occurs, return an empty result
}

// after updating the selectd rows, use this function to commit the changes
void update_commit(work & w){
  try{
    w.commit();
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    std::cout << "Failed to execute update_commit. Rolling back the transaction." << std::endl;
    w.abort();
  }
}

int getCurrTime() {
  // get the current time in seconds since the epoch
  std::time_t now = std::time(nullptr);

  // save the time as an integer
  int time_as_int = static_cast<int>(now);

  return time_as_int;
}

void dropTables(connection * c, const vector<string> & tables) {
  string sql = "DROP TABLE IF EXISTS ";
  if (tables.size() > 0) {
    sql.append("\"" + tables[0] + "\"");
    for (size_t i = 1; i < tables.size(); i++) {
      sql.append(", \"" + tables[i] + "\"");
    }
  }
  executeSQL(c, sql);
}

void Database::initialize() {
  // initialize the products item and amounts
  std::vector<std::string> table_name = {"\"amazonSite_inventory\"", "\"amazonSite_order\"", "\"amazonSite_package\"", "\"amazonSite_product\"", "\"amazonSite_warehouse\""};
  for (size_t i = 0; i < table_name.size(); i ++ ){
    std::string sql = "DELETE FROM " + table_name[i] + ";";
    std::cout << sql << std::endl;
    executeSQL(c, sql);
  }
}

bool Database::initialize_inventory(const int wh_id, const int p_id, const int quantity){
  string sql;
  try
  {
    sql = "INSERT INTO \"amazonSite_inventory\" (quantity, product_id, warehouse_id) ";
    sql += string("VALUES (");
    sql += std::to_string(quantity) + ",";
    sql += std::to_string(p_id) + ",";
    sql += std::to_string(wh_id);
    sql += string(") ON CONFLICT (product_id, warehouse_id) DO UPDATE SET quantity = EXCLUDED.quantity; ");
    std::cout << "start to execute sql" << std::endl;
    executeSQL(c, sql);
    std::cout << "finished executing sql" << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return false;
  }
  return true;
}

bool Database::insert_and_update_warehouse(const int wh_id, const int loc_x, const int loc_y){
  string sql;
  try
  {
    sql = "INSERT INTO \"amazonSite_warehouse\" (warehouse_id, address_x, address_y) ";
    sql += string("VALUES (");
    sql += std::to_string(wh_id) + ",";
    sql += std::to_string(loc_x) + ",";
    sql += std::to_string(loc_y);
    sql += string(") ON CONFLICT (warehouse_id) DO NOTHING; ");
    std::cout << "start to execute sql" << std::endl;
    executeSQL(c, sql);
    std::cout << "finished executing sql" << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return false;
  }
  return true;
}

bool Database::insert_and_update_product(const int p_id, const string& p_name, const string& p_description){
  string sql;
  try
  {
    sql = "INSERT INTO \"amazonSite_product\" (product_id, name, description) ";
    sql += string("VALUES (");
    sql += std::to_string(p_id) + ",";
    sql += c->quote(p_name) + ",";
    sql += c->quote(p_description);
    sql += string(") ON CONFLICT (product_id) DO UPDATE SET name = EXCLUDED.name, description = EXCLUDED.description; ");
    std::cout << "start to execute sql" << std::endl;
    executeSQL(c, sql);
    std::cout << "finished executing sql" << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return false;
  }
  return true;
}

bool Database::insert_order(const int order_num, const int product_id, const int user_id, const int quantity, const string & order_status, const int package_id, const time_t & create_time){
  string sql;
  try
  {
    sql = "INSERT INTO \"amazonSite_order\" (order_num, product_id, user_id, quantity, order_status, package_id, create_time) ";
    sql += string("VALUES (");
    sql += std::to_string(order_num) + ",";
    sql += std::to_string(product_id) + ",";
    sql += std::to_string(user_id) + ",";
    sql += std::to_string(quantity) + ",";
    sql += c->quote(order_status) + ",";
    sql += std::to_string(package_id) + ",";
    sql += "CURRENT_TIME(2)";  //defalut current_time
    sql += string(");");
    std::cout << "start to execute sql" << std::endl;
    std::cout << sql << std::endl;
    executeSQL(c, sql);
    std::cout << "finished executing sql" << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return false;
  }
  return true;
}

bool Database::insert_package(const int package_id, const int owner_id, const int warehouse_id, const int dest_x, const int dest_y){
  string sql;
  try
  {
    sql = "INSERT INTO \"amazonSite_package\" (package_id, owner_id, warehouse_id, dest_x, dest_y, pack_time, ups_id, truck_id) ";
    sql += string("VALUES (");
    sql += std::to_string(package_id) + ",";
    sql += std::to_string(owner_id) + ",";
    sql += std::to_string(warehouse_id) + ",";
    sql += std::to_string(dest_x) + ",";
    sql += std::to_string(dest_y) + ",";
    sql += "NULL,";   // set pack_time to NULL
    sql += "NULL,";   // set ups_id to NULL
    sql += "NULL";    // set track_num to NULL
    sql += string(");");
    std::cout << "start to execute sql" << std::endl;
    std::cout << sql << std::endl;
    executeSQL(c, sql);
    std::cout << "finished executing sql" << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return false;
  }
  return true;
}

bool Database::insert_and_update_inventory(const int wh_id, const int p_id, const int quantity){
  string sql;
  try
  {
    sql = "INSERT INTO \"amazonSite_inventory\" (quantity, warehouse_id, product_id) ";
    sql += string("VALUES (");
    sql += std::to_string(quantity) + ",";
    sql += std::to_string(wh_id) + ",";
    sql += std::to_string(p_id);
    sql += string(") ON CONFLICT (warehouse_id,product_id) DO UPDATE SET quantity = \"amazonSite_inventory\".quantity + EXCLUDED.quantity; ");
    std::cout << "start to execute sql" << std::endl;
    executeSQL(c, sql);
    std::cout << "finished executing sql" << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return false;
  }
  return true;
}

bool Database::update_package_status(const int ship_id, const string status){
  string sql;
  try
  {
    sql = "UPDATE \"amazonSite_order\" SET order_status=" + c->quote(status);
    sql += string("WHERE package_id=") + std::to_string(ship_id) + ";";
    std::cout << "start to execute sql" << std::endl;
    executeSQL(c, sql);
    std::cout << "finished executing sql" << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return false;
  }
  return true;
}

int Database::match_inventory(const int product_id, const int quantity){
  try
  {
   stringstream ss_sql;
  // select warehouse_id from "amazonSite_inventory" where product_id = product_id and quantity >= quantity limit 1;
  ss_sql << "select warehouse_id,quantity from \"amazonSite_inventory\"";
  ss_sql << "where product_id=" << product_id << " and quantity>=" << quantity << " limit 1;";
  std::cout << ss_sql.str() << std::endl;
  work w(*c);
  result r = select_for_update(ss_sql.str(), w);
  if (r.empty()){
    std::cout << "No matched inventory with product_id = " << product_id << " and quantity = " << quantity << std::endl;
    return -100;  
  }
  std::cout << "success select_for_update found" << std::endl;
  result::const_iterator c = r.begin();
  int warehouse_id = c[0].as<int>();
  int stock = c[1].as<int>();
  stock -= quantity;

  stringstream ss_update_sql;
  ss_update_sql << "update \"amazonSite_inventory\" set quantity = " << stock;
  ss_update_sql << " where warehouse_id = " << warehouse_id << " and product_id = " << product_id << ";";
  std::cout << ss_update_sql.str() << std::endl;
  w.exec(ss_update_sql.str());

  update_commit(w);

  return warehouse_id;   
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return -200;
  }
}

void Database::disconnect() {
  if (this->c != NULL) {
    c->disconnect();
    delete this->c;
  }
}

// Database::~Database() {
//   if (this->c != NULL) {
//     c->disconnect();
//     delete this->c;
//   }
// }
