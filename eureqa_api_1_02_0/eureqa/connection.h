#ifndef EUREQA_CONNECTION_H
#define EUREQA_CONNECTION_H

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <eureqa/data_set.h>
#include <eureqa/server_info.h>
#include <eureqa/search_progress.h>
#include <eureqa/search_options.h>
#include <eureqa/solution_frontier.h>

#define EUREQA_USE_XML

namespace eureqa
{
// default connection options
static const int default_port_tcp = 22112;
static const int default_port_multicast = 30002;
static boost::asio::io_service default_io_service;

// response codes
static const int result_success = 0;
static const int result_error = 1;

// command codes
namespace commands
{
static const int send_data_set      = 101;
static const int send_data_location = 102;
static const int send_options       = 103;
static const int send_individuals   = 104;
static const int query_progress     = 201;
static const int query_server_info  = 202;
static const int query_individuals  = 203;
static const int query_frontier     = 204;
static const int start_search       = 301;
static const int pause_search       = 302;
static const int end_search         = 303;
static const int calc_solution_info = 401;
}

// info sent back from the server after non-query commands
struct command_result
{
public:
    int value_;
    std::string message_;
public:
    command_result() : value_(result_success) { }
    command_result(int value, std::string message) : value_(value), message_(message) { }
    int value() const { return value_; }
    std::string message() const { return message_; }
    operator const void*() const { return ((value_ == result_success)?this:0); }
};
inline
std::ostream& operator <<(std::ostream& os, const command_result& r) { return os << r.message(); }

// synchronous/blocking network interface with a eureqa server
class connection
{
protected:
    boost::asio::ip::tcp::socket socket_;
    command_result last_result_;
    
public:
    // default constructor
    connection();
    connection(std::string hostname, int port = default_port_tcp);
    connection(boost::asio::io_service& io_service);
    virtual ~connection() { disconnect(); }
    
    // basic connection information
    bool is_connected() const { return socket_.is_open(); }
    command_result last_result() const { return last_result_; }

    // opens a network connection to a eureqa server
    bool connect(std::string hostname, int port = default_port_tcp);
    void disconnect() { socket_.close(); }

    // send server the data set over the network
    // or tell it to load it from a network file
    bool send_data_set(const eureqa::data_set& data);
    bool send_data_location(std::string path);
    
    // send server the search options
    bool send_options(const eureqa::search_options& options);
    
    // send server individuals to insert into its population
    bool send_individuals(std::string text);
    bool send_individuals(eureqa::solution_info soln);
    bool send_individuals(const std::vector<eureqa::solution_info>& individuals);

    // query server for information on the search progress
    bool query_progress(eureqa::search_progress& progress);
    
    // query server for its system information
    bool query_server_info(eureqa::server_info& info);
    
    // query server for random individuals from its population
    bool query_individuals(eureqa::solution_info& soln);
    bool query_individuals(std::vector<eureqa::solution_info>& individuals, int count);
    
    // query the servers local solution frontier
    bool query_frontier(eureqa::solution_frontier& front);
    
    // tell server to start/pause/end searching
    bool start_search();
    bool pause_search();
    bool end_search();
    
    // calculate the solution info on the server
    bool calc_solution_info(eureqa::solution_info& soln);
    bool calc_solution_info(std::vector<eureqa::solution_info>& individuals);
    
    // returns are a short description of the connection
    std::string summary() const;
    std::string remote_address() const;
    int remote_port() const;
    
protected:
    bool connect_socket(std::string hostname, int port);

    template<typename T> bool write_fixed(const T& val);
    template<typename T> bool write_command_fixed(int cmd, const T& val);
    bool write_command(int cmd);
    bool write_command_packet(int cmd, const void* buf, int num_bytes);
    bool write_command_packet(int cmd, const std::string& s);
    
    template<typename T> bool read_fixed(T& val);
    bool read_packet(std::vector<char>& buf);
    bool read_packet(std::string& s);
    bool read_response();
    
};

/*---------------------------------------------------------------------------
    implementions:
*--------------------------------------------------------------------------*/

inline 
connection::connection() : 
    socket_(default_io_service)
{ }

inline
connection::connection(std::string hostname, int port) :
    socket_(default_io_service)
{
    connect(hostname, port);
}

inline 
connection::connection(boost::asio::io_service& io_service) : 
    socket_(io_service)
{ }

inline
bool connection::connect(std::string hostname, int port)
{
    if (!connect_socket(hostname, port)) { return false; }
    if (!read_response()) { return false; }
    return true;
}

inline
bool connection::send_data_set(const eureqa::data_set& data)
{
    // serialize the data set
    #ifdef EUREQA_USE_XML
    std::ostringstream ss;
    boost::archive::xml_oarchive ar(ss);
    #else
    std::ostringstream ss(std::ios_base::out|std::ios_base::binary);
    boost::archive::binary_oarchive ar(ss);
    #endif
    ar & boost::serialization::make_nvp("data_set", data );
    
    // send a command-code, packet size, and data packet
    if (!write_command_packet(commands::send_data_set, ss.str())) { return false; }
    if (!read_response()) { return false; }
    return true;
}

inline
bool connection::send_data_location(std::string path)
{
    if (!write_command_packet(commands::send_data_location, path)) { return false; }
    if (!read_response()) { return false; }
    return true;
}

inline
bool connection::send_options(const eureqa::search_options& options)
{
    // serialize the data set
    #ifdef EUREQA_USE_XML
    std::ostringstream ss;
    boost::archive::xml_oarchive ar(ss);
    #else
    std::ostringstream ss(std::ios_base::out|std::ios_base::binary);
    boost::archive::binary_oarchive ar(ss);
    #endif
    ar << boost::serialization::make_nvp("search_options", options );
    
    // send a command-code, packet size, and data packet
    if (!write_command_packet(commands::send_options, ss.str())) { return false; }
    if (!read_response()) { return false; }
    return true;
}

inline
bool connection::send_individuals(std::string text)
{
    return send_individuals(eureqa::solution_info(text));
}

inline
bool connection::send_individuals(eureqa::solution_info soln)
{
    std::vector<eureqa::solution_info> individuals(1, soln);
    return send_individuals(individuals);
}

inline
bool connection::send_individuals(const std::vector<solution_info>& individuals)
{
    // serialize the data set
    std::ostringstream ss;
    boost::archive::xml_oarchive ar(ss);
    ar << boost::serialization::make_nvp("vector_solution_info", individuals );
    
    // send a command-code, packet size, and data packet
    if (!write_command_packet(commands::send_individuals, ss.str())) { return false; }
    if (!read_response()) { return false; }
    return true;
}

inline
bool connection::query_progress(eureqa::search_progress& progress)
{
    // request progress
    if (!write_command(commands::query_progress)) { return false; }
    
    // read packet
    std::string s;
    if (!read_packet(s)) { return false; }
    
    // serialize store
    std::istringstream ss(s);
    boost::archive::xml_iarchive ar(ss);
    ar >> boost::serialization::make_nvp("search_progress", progress );
    return true;
}

inline
bool connection::query_server_info(eureqa::server_info& info)
{
    // request progress
    if (!write_command(commands::query_server_info)) { return false; }
    
    // read packet
    std::string s;
    if (!read_packet(s)) { return false; }
    
    // serialize store
    std::istringstream ss(s);
    boost::archive::xml_iarchive ar(ss);
    ar >> boost::serialization::make_nvp("server_info", info );
    return true;
}

inline
bool connection::query_individuals(eureqa::solution_info& soln)
{
    std::vector<eureqa::solution_info> individuals;
    if (!query_individuals(individuals, 1)) { return false; }
    if (individuals.size() != 1) { return false; }
    soln = individuals[0];
    return true;
}

inline
bool connection::query_individuals(std::vector<solution_info>& individuals, int count)
{
    // request individuals
    if (!write_command_fixed(commands::query_individuals, count)) { return false; }
    
    // read packet
    std::string s;
    if (!read_packet(s)) { return false; }
    
    // serialize
    std::istringstream ss(s);
    boost::archive::xml_iarchive ar(ss);
    ar >> boost::serialization::make_nvp("vector_solution_info", individuals );
    return true;
}

inline
bool connection::query_frontier(eureqa::solution_frontier& frontier)
{
    // request frontier
    if (!write_command(commands::query_frontier)) { return false; }
    
    // read packet
    std::string packet;
    if (!read_packet(packet)) { return false; }
    
    // serialize store
    std::istringstream is(packet);
    boost::archive::xml_iarchive ar(is);
    ar >> boost::serialization::make_nvp("solution_frontier", frontier);
    return true;
}

inline
bool connection::start_search()
{
    if (!write_command(commands::start_search)) { return false; }
    if (!read_response()) { return false; }
    return true;
}

inline
bool connection::pause_search()
{
    if (!write_command(commands::pause_search)) { return false; }
    if (!read_response()) { return false; }
    return true;
}

inline
bool connection::end_search()
{
    if (!write_command(commands::end_search)) { return false; }
    if (!read_response()) { return false; }
    return true;
}

inline
bool connection::calc_solution_info(eureqa::solution_info& ind)
{
    std::vector<eureqa::solution_info> individuals(1, ind);
    if (!calc_solution_info(individuals)) { return false; }
    if (individuals.size() != 1) { return false; }
    ind = individuals[0];
    return true;
}

inline
bool connection::calc_solution_info(std::vector<eureqa::solution_info>& individuals)
{
    // serialize the data set
    std::ostringstream os;
    boost::archive::xml_oarchive ar(os);
    ar << boost::serialization::make_nvp("vector_solution_info", individuals);
    
    // send a command-code, packet size, and data packet
    if (!write_command_packet(commands::calc_solution_info, os.str())) { return false; }
    
    // read packet
    std::string packet;
    if (!read_packet(packet)) { return false; }
    
    // serialize
    std::istringstream is(packet);
    boost::archive::xml_iarchive ar2(is);
    ar2 >> boost::serialization::make_nvp("vector_solution_info", individuals);
    return true;
}

inline
std::string connection::remote_address() const
{
    boost::system::error_code error;
    return socket_.remote_endpoint(error).address().to_string();
}
inline 
int connection::remote_port() const
{
    boost::system::error_code error;
    return socket_.remote_endpoint(error).port();
}

inline
std::string connection::summary() const
{
    std::ostringstream os;
    if (!is_connected())
    {
        os << "Disconnected";
    }
    else
    {
        os << "Connected to " << remote_address();
    }
    return os.str();
}

inline
bool connection::connect_socket(std::string hostname, int port)
{
    try
    {
        // resolve hostname into a tcp end point
        boost::asio::ip::tcp::resolver resolver(socket_.get_io_service());
        boost::asio::ip::tcp::resolver::query query(hostname, boost::lexical_cast<std::string>(port));
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        boost::asio::ip::tcp::resolver::iterator end;
    
        // create and connect a socket
        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end)
        {
            socket_.close();
            socket_.connect(*endpoint_iterator++, error);
        }
        
        if (error) { socket_.close(); }
        return !error;
    }
    catch (...) { return false; }
}

template<typename T>
inline
bool connection::write_fixed(const T& val)
{
    // write a primitive or fixed-sized object
    boost::system::error_code error;
    boost::asio::write(socket_, boost::asio::buffer(&val,sizeof(T)), boost::asio::transfer_all(), error);
    if (error) { disconnect(); }
    return !error;
}

template<typename T> 
inline
bool connection::write_command_fixed(int cmd, const T& val)
{
    // write a packet: a size/data pair
    boost::system::error_code error;
    boost::array<boost::asio::const_buffer, 2> packet = {{ boost::asio::buffer(&cmd,sizeof(int)), boost::asio::buffer(&val,sizeof(T)) }};
    boost::asio::write(socket_, packet, boost::asio::transfer_all(), error);
    if (error) { disconnect(); }
    return !error;
}

inline
bool connection::write_command(int cmd)
{
    return write_fixed(cmd);
}

inline
bool connection::write_command_packet(int cmd, const void* buf, int num_bytes)
{
    // write a packet: a size/data pair
    boost::system::error_code error;
    boost::array<boost::asio::const_buffer, 3> packet = {{ boost::asio::buffer(&cmd,sizeof(int)), boost::asio::buffer(&num_bytes,sizeof(int)), boost::asio::buffer(buf,num_bytes) }};
    boost::asio::write(socket_, packet, boost::asio::transfer_all(), error);
    if (error) { disconnect(); }
    return !error;
}

inline
bool connection::write_command_packet(int cmd, const std::string& s)
{
    // wraps the std::vector<char> version
    return write_command_packet(cmd, s.c_str(), s.length());
}

inline
bool connection::read_packet(std::vector<char>& buf)
{
    // read size of packet
    int num_bytes = 0;
    boost::system::error_code error_header;
    boost::asio::read(socket_, boost::asio::buffer(&num_bytes,sizeof(int)), boost::asio::transfer_all(), error_header);
    if (error_header) { disconnect(); }
    if (error_header || num_bytes < 0) { return false; }
    
    // read data
    buf.resize(num_bytes);
    boost::system::error_code error_data;
    if (num_bytes > 0) { boost::asio::read(socket_, boost::asio::buffer(&buf[0],num_bytes), boost::asio::transfer_all(), error_data); }
    if (error_data) { disconnect(); }
    return !error_header && !error_data;
}

inline
bool connection::read_packet(std::string& s)
{
    // wraps the std::vector<char> version
    std::vector<char> buf;
    if (!read_packet(buf)) { return false; }
    s.assign(&buf[0], buf.size());
    return true;
}

template<typename T>
inline
bool connection::read_fixed(T& val)
{
    // read a primitive or fixed-sized object
    boost::system::error_code error;
    boost::asio::read(socket_, boost::asio::buffer(&val,sizeof(T)), boost::asio::transfer_all(), error);
    if (error) { disconnect(); }
    return !error;
}

inline
bool connection::read_response()
{
    if (!read_fixed(last_result_.value_)) { return false; }
    if (!read_packet(last_result_.message_)) { return false; }
    return true;
}

} // namespace eureqa

#endif //EUREQA_CONNECTION_H
