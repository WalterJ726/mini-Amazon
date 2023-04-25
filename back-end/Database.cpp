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
}

bool Database::initialize_inventory(const int wh_id, const int p_id, const int quantity){
  string sql;
  try
  {
    sql = "INSERT INTO \"amazonSite_inventory\" (quantity, warehouse_id, product_id) ";
    sql += string("VALUES (");
    sql += std::to_string(quantity) + ",";
    sql += std::to_string(wh_id) + ",";
    sql += std::to_string(p_id);
    sql += string(") ON CONFLICT (warehouse_id,product_id) DO UPDATE SET quantity = EXCLUDED.quantity; ");
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

bool Database::insert_and_update_inventory(const int wh_id, const int p_id, const int quantity){
  string sql;
  try
  {
    sql = "INSERT INTO \"amazonSite_inventory\" (quantity, warehouse_id, product_id) ";
    sql += string("VALUES (");
    sql += std::to_string(quantity) + ",";
    sql += std::to_string(wh_id) + ",";
    sql += std::to_string(p_id);
    sql += string(") ON CONFLICT (warehouse_id,product_id) DO UPDATE SET quantity += EXCLUDED.quantity; ");
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
    sql = "UPDATE \"amazonSite_order\" SET order_status=" + status + ",";
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
