#ifndef EUREQA_SERVER_INFO_H
#define EUREQA_SERVER_INFO_H

#include <string>

namespace eureqa
{
// structure for receiving system information about the server
class server_info
{
public:
    std::string hostname_;
    std::string operating_system_;
    double eureqa_version_;
    int cpu_cores_;
    
public:
    // default constructor
    server_info();
    
    // test if info is entered and in range
    bool is_valid() const;
    
    // returns a short text summary of the server info
    std::string summary() const;
    
protected:
    // boost serialization, used for sending over it the network
    // can also be used for saving/loading the data set
    friend class boost::serialization::access;
    template<class TArchive> void serialize(TArchive& ar, const unsigned int /*version*/);
};

/*---------------------------------------------------------
    Implementation:
*--------------------------------------------------------*/
inline
server_info::server_info() :
    hostname_(""),
    operating_system_(""),
    eureqa_version_(0),
    cpu_cores_(0)
{ }

inline
bool server_info::is_valid() const
{
    return (hostname_.length() > 0)
        && (operating_system_.length() > 0)
        && (eureqa_version_ > 0)
        && (cpu_cores_ > 0)
        ;
}


inline
std::string server_info::summary() const
{
    std::ostringstream os;
    if (!is_valid()) { os << "Invalid! "; }
    os << hostname_;
    os << ", Eureqa " << eureqa_version_;
    os << " (" << operating_system_ << ")";
    os << ", " << cpu_cores_ << " CPU core" << (cpu_cores_==1?"":"s");
    return os.str();
}

template<typename TArchive>
inline
void server_info::serialize(TArchive& ar, const unsigned int /*version*/) 
{
    ar & BOOST_SERIALIZATION_NVP( hostname_ );
    ar & BOOST_SERIALIZATION_NVP( operating_system_ );
    ar & BOOST_SERIALIZATION_NVP( eureqa_version_ );
    ar & BOOST_SERIALIZATION_NVP( cpu_cores_ );
}

} // namespace eureqa

#endif // EUREQA_SERVER_INFO_H
