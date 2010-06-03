#ifndef EUREQA_DATA_SET_H
#define EUREQA_DATA_SET_H

#include <string>
#include <fstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/binary_object.hpp>

namespace eureqa
{
// parsing helper functions for importing text data
template<typename T> bool is_convertable_to(const std::string& s);
template<typename T> T convert_to(const std::string& s, T default_value = 0);
std::string read_word(std::istream& is, const std::string& white = " \t\r\n");

// main structure for holding data used by eureqa
class data_set
{
public:
    std::vector<int> r_; // series identifier (optional)
    std::vector<float> t_; // time ordering values (optional)
    std::vector<float> w_; // weight values (optional)
    boost::numeric::ublas::matrix<float> X_; // data values
    boost::numeric::ublas::matrix<float> Y_; // special values (reserved)
    std::vector<std::string> X_symbols_; // symbols for data values
    std::vector<std::string> Y_symbols_; // symbols for special values (reserved)
    
public:
    // constructors
    data_set() { }
    data_set(std::string path) { import_ascii(path); }
    data_set(int rows, int cols) : X_(rows,cols) { set_default_symbols(); }
    
    // test if the data set is sized and filled in correctly
    bool is_valid() const;
    
    // sets symbols as { x1, x2, x3, ... }
    void set_default_symbols();
    
    // basic container member functions
    int size()     const { return (int)X_.size1(); }
    int num_vars() const { return (int)X_.size2(); }
    int special_vars() const { return (int)Y_.size2(); }
          float& operator ()(int i, int j)       { return X_(i,j); }
    const float& operator ()(int i, int j) const { return X_(i,j); }
    void clear() { (*this) = data_set(); }
    void swap(data_set& d);
    void resize(int rows, int cols);
    bool empty() const { return (size() == 0); }
    
    // imports plain ascii text files with single line header
    // can read a eureqa header, plain column header, or missing header
    // data can be delimited by whitespace or commas
    // lines can be commented using the '%' character
    bool import_ascii(std::string path);
    bool import_ascii(std::string path, std::string& error_msg);
    bool import_ascii(std::istream& is);
    bool import_ascii(std::istream& is, std::string& error_msg);
    void export_ascii(std::string path) { std::ofstream file(path.c_str()); export_ascii(file); }
    void export_ascii(std::ostream& os);
    
    // returns a short text summary of the data set
    std::string summary() const;
    
protected:
    // boost serialization, used for sending over it the network
    // can also be used for saving/loading the data set
    friend class boost::serialization::access;
    template<class TArchive> void serialize(TArchive& ar, const unsigned int version);
};

/*---------------------------------------------------------
    Implementation:
*--------------------------------------------------------*/
inline
std::string read_word(std::istream& is, const std::string& white)
{
    std::string s;
    char c;
    while (is.get(c))
    {
        if (white.find(c) == std::string::npos) { s += c; }
        else if (!s.empty()) { is.unget(); break; }
    }
    return s;
}

template<typename T>
inline
bool is_convertable_to(const std::string& s)
{
    try { boost::lexical_cast<T>(s); return true; }
    catch (boost::bad_lexical_cast) { return false; }
}

template<typename T> 
inline
T convert_to(const std::string& s, T default_value)
{
    try { return boost::lexical_cast<T>(s); }
    catch (boost::bad_lexical_cast) { return default_value; }
}

inline
bool data_set::is_valid() const
{
    return (size() > 0 && num_vars() > 0) // not empty
        && ((int)r_.size() == 0 || (int)r_.size() == size()) // no IDs or correct size
        && ((int)t_.size() == 0 || (int)t_.size() == size()) // no ordering or correct size
        && ((int)w_.size() == 0 || (int)w_.size() == size()) // no weigths or correct size
        && ((int)Y_.size1() == 0 || (int)Y_.size1() == size()) // no special values or correct size
        && ((int)X_.size1() == size()) // X correct size
        && ((int)X_.size2() == num_vars()) // X correct number of vars
        && ((int)X_symbols_.size() == (int)X_.size2()) // correct number of X symbols
        && ((int)Y_symbols_.size() == (int)Y_.size2()) // correct number of Y symbols
        ;
}

inline
void data_set::set_default_symbols()
{
    X_symbols_.resize(X_.size2());
    Y_symbols_.resize(Y_.size2());
    for (int j=0; j<(int)X_.size2(); ++j) { X_symbols_[j] = boost::str(boost::format("x%i")%j); }
    for (int j=0; j<(int)Y_.size2(); ++j) { Y_symbols_[j] = boost::str(boost::format("y%i")%j); }
}

inline
void data_set::swap(data_set& d)
{
    r_.swap(d.r_);
    t_.swap(d.t_);
    w_.swap(d.w_);
    X_.swap(d.X_);
    Y_.swap(d.Y_);
    X_symbols_.swap(d.X_symbols_);
    Y_symbols_.swap(d.Y_symbols_);
}

inline
void data_set::resize(int rows, int cols)
{
    int cols0 = num_vars();
    
    if (r_.size() > 0) { r_.resize(rows); }
    if (t_.size() > 0) { w_.resize(rows); }
    if (w_.size() > 0) { t_.resize(rows); }
    X_.resize(rows, cols);
    if ((int)Y_.size1() > 0 && (int)Y_.size2() > 0) { Y_.resize(rows, Y_.size2()); }
    
    X_symbols_.resize(cols);
    if (cols > cols0) { set_default_symbols(); }
}

inline
bool data_set::import_ascii(std::string filename) 
{
    std::string ignored; 
    return import_ascii(filename, ignored); 
} 
inline
bool data_set::import_ascii(std::istream& is)
{
    std::string ignored;
    return import_ascii(is, ignored);
}

inline 
bool data_set::import_ascii(std::string filename, std::string& error_msg)
{
    std::ifstream ifs(filename.c_str());
    if (!ifs) { error_msg = "Unable to open file \'" + filename + "\'"; return false; }
    return import_ascii(ifs, error_msg);
}

inline
bool data_set::import_ascii(std::istream& is, std::string& error_msg)
{
    if (!is) { error_msg = "Bad file stream"; return false; }
    clear();

    // read header
    std::string header_s;
    std::getline(is, header_s);
    std::vector<std::string> header;
    boost::trim_if(header_s, boost::is_any_of("% \t\r\n"));
    boost::split(header, header_s, boost::is_any_of("% \t\r\n"), boost::token_compress_on);
    
    int data_cols = 0;
    for (int j=0; j<(int)header.size(); ++j)
        if (header[j] != "|")
            ++data_cols;
    
    // determine data format
    bool eureqa_header = (std::count(header.begin(),header.end(),"|") == 2);
    bool no_header = is_convertable_to<double>(header[0]);
    bool plain_header = (!eureqa_header && !no_header);
    
    // default column locations
    int r_col = -1;
    int t_col = -1;
    int w_col = -1;
    int x_col = 0;
    int x_count = data_cols;
    int y_col = -1;
    int y_count = 0;
    
    // interpret header
    if (eureqa_header)
    {
        int sep1 = (int)(std::find(header.begin(), header.end(), "|") - header.begin());
        int sep2 = (int)(std::find(header.begin()+sep1+1, header.end(), "|") - header.begin());
        for (int j=0; j<sep1; ++j)
        {
            if (header[j] == "r") { r_col = j; }
            if (header[j] == "t") { t_col = j; }
            if (header[j] == "w") { w_col = j; }
        }
        x_col = sep1;
        x_count = sep2 - sep1 - 1;
        y_col = sep2 - 1;
        y_count = header.size() - sep2 - 1;
        X_symbols_.assign(header.begin()+sep1+1, header.begin()+sep2);
        Y_symbols_.assign(header.begin()+sep2+1, header.end());
    }
    else if (no_header) { set_default_symbols(); }
    else if (plain_header) { X_symbols_ = header; }
    else { error_msg = "First line is misformed or an unrecognized header"; return false; }
    
    // read data values
    const std::string delims = ", \t\r\n";
    std::string word = read_word(is, delims);
    int data_points = 0;
    for (int i=0; is; ++i)
    {
        // skip commented lines
        if (word.length() > 0 && word[0] == '%') { std::getline(is, word); word = read_word(is, delims); --i; continue; }
        
        // grow the container sizes
        ++data_points;
        if (r_col >= 0) { r_.push_back(0); }
        if (t_col >= 0) { t_.push_back(0); }
        if (w_col >= 0) { w_.push_back(0); }
        if (x_col >= 0 && x_count >= 0 && i >= (int)X_.size1()) { X_.resize(data_points*2, x_count, true); }
        if (y_col >= 0 && y_count >= 0 && i >= (int)Y_.size1()) { Y_.resize(data_points*2, y_count, true); }
        
        // fill the new row
        for (int j=0; j<data_cols; ++j)
        {
            if (!is_convertable_to<float>(word)) 
            { 
                error_msg = str(boost::format("Missing or non-numeric value at row %1%, column %2%: \'%3%\'")%(i+1)%(j+1)%word); 
                return false; 
            }
            if (r_col >= 0 && j == r_col) { r_[i] = convert_to<int>(word); }
            if (t_col >= 0 && j == t_col) { t_[i] = convert_to<float>(word); }
            if (w_col >= 0 && j == w_col) { w_[i] = convert_to<float>(word); }
            if (x_col >= 0 && j >= x_col && j < x_col+x_count) { X_(i,j-x_col) = convert_to<float>(word); }
            if (y_col >= 0 && j >= y_col && j < y_col+y_count) { Y_(i,j-y_col) = convert_to<float>(word); }
            word = read_word(is, delims);
        }
    }

    // trim off the extra rows
    if (x_col >= 0) { X_.resize(data_points, x_count, true); }
    if (y_col >= 0) { Y_.resize(data_points, y_count, true); }
    
    // final check
    if (!is_valid()) { error_msg = "Final data set is incomplete or invalid"; return false; }
    error_msg.clear();
    return true;
}

inline
void data_set::export_ascii(std::ostream& os)
{
    // header symbols
    os << "% ";
    if (r_.size() > 0) { os << "r\t"; }
    if (t_.size() > 0) { os << "t\t"; }
    if (w_.size() > 0) { os << "w\t"; }
    os << "| ";
    for (int j=0; j<(int)X_.size2(); ++j) { os << X_symbols_[j] << '\t'; }
    os << "| ";
    for (int j=0; j<(int)Y_.size2(); ++j) { os << Y_symbols_[j] << '\t'; }
    os << std::endl;
    
    // data rows
    for (int i=0; i<size(); ++i)
    {
        if (r_.size() > 0) { os << r_[i] << '\t'; }
        if (t_.size() > 0) { os << t_[i] << '\t'; }
        if (w_.size() > 0) { os << w_[i] << '\t'; }
        for (int j=0; j<(int)X_.size2(); ++j) { os << X_(i,j) << '\t'; }
        for (int j=0; j<(int)Y_.size2(); ++j) { os << Y_(i,j) << '\t'; }
        os << std::endl;
    }
}

inline
std::string data_set::summary() const
{
    std::ostringstream os;
    if (!is_valid()) { os << "Invalid! "; }
    os << size() << " data points";
    os << ", " << num_vars() << " variables";
    if (Y_.size2() > 0) { os << ", " << Y_.size1() << " special variables"; }
    if (r_.size()  > 0) { os << ", series identifiers"; }
    if (t_.size()  > 0) { os << ", ordering values"; }
    if (w_.size()  > 0) { os << ", weight values"; }
    return os.str();
}

template<class TArchive, typename T>
inline
void binary_serialize_vector(TArchive& ar, std::string name, std::vector<T>& vec, const unsigned int /*version*/)
{
    int vec_size = vec.size();
    ar & boost::serialization::make_nvp((name + "_size__").c_str(), vec_size );
    if (TArchive::is_loading::value) { vec.resize(vec_size); }
    if (vec.size() > 0)
    {
        ar & boost::serialization::make_nvp(name.c_str(), boost::serialization::make_binary_object(&(vec[0]), sizeof(T)*vec.size()));
    }
}

template<class TArchive, typename T>
inline
void binary_serialize_matrix(TArchive& ar, std::string name, boost::numeric::ublas::matrix<T>& mat, const unsigned int /*version*/)
{
    int mat_rows = mat.size1();
    int mat_cols = mat.size2();
    ar & boost::serialization::make_nvp((name + "_rows__").c_str(), mat_rows);
    ar & boost::serialization::make_nvp((name + "_cols__").c_str(), mat_cols);
    if (TArchive::is_loading::value) { mat.resize(mat_rows, mat_cols); }
    if (mat.data().size() > 0)
    {
        ar & boost::serialization::make_nvp(name.c_str(), boost::serialization::make_binary_object(&(mat.data()[0]), sizeof(T)*mat.data().size()));
    }
}

template<class TArchive>
inline
void data_set::serialize(TArchive& ar, const unsigned int version) 
{
    bool binary_format = true;
    ar & boost::serialization::make_nvp("binary_format", binary_format);
    
    if (binary_format)
    {
        binary_serialize_vector(ar, "r_", r_, version);
        binary_serialize_vector(ar, "t_", t_, version);
        binary_serialize_vector(ar, "w_", w_, version);
        binary_serialize_matrix(ar, "X_", X_, version);
        binary_serialize_matrix(ar, "Y_", Y_, version);
    }
    else
    {
        ar & BOOST_SERIALIZATION_NVP( r_ );
        ar & BOOST_SERIALIZATION_NVP( t_ );
        ar & BOOST_SERIALIZATION_NVP( w_ );
        ar & BOOST_SERIALIZATION_NVP( X_ );
        ar & BOOST_SERIALIZATION_NVP( Y_ );
    }
    
    ar & BOOST_SERIALIZATION_NVP( X_symbols_ );
    ar & BOOST_SERIALIZATION_NVP( Y_symbols_ );
}

} // namespace eureqa

namespace std 
{
    // specialize the swap function
    template<>
    inline void swap(eureqa::data_set& a, eureqa::data_set& b) { a.swap(b); }
}

#endif // EUREQA_DATA_SET_H
