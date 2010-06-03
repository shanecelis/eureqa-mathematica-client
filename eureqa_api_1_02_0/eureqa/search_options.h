#ifndef EUREQA_SEARCH_INFO_H
#define EUREQA_SEARCH_INFO_H

#include <string>
#include <vector>
#include <boost/serialization/vector.hpp>

namespace eureqa
{
// identifiers for the fitness metrics
namespace fitness_types
{
const static int absolute_error       =  0;
const static int squared_error        =  1;
const static int root_squared_error   =  2;
const static int logarithmic_error    =  3;
const static int explog_error         =  4;
const static int correlation          =  5;
const static int minimize_difference  =  6;
const static int akaike_information   =  7;
const static int bayesian_information =  8;
const static int maximum_error        =  9;
const static int median_error         = 10;
const static int implicit_error       = 11;
const static int slope_error          = 12;
const static int count                = 13;
std::string str(int type);
}

// structure for sending search options to the server
class search_options
{
public:
    std::string search_relationship_;
    std::vector<std::string> building_blocks_;
    float normalize_fitness_by_;
    int    fitness_metric_;
    int solution_population_size_;
    int predictor_population_size_;
    int trainer_population_size_;
    float solution_crossover_probability_;
    float solution_mutation_probability_;
    float predictor_crossover_probability_;
    float predictor_mutation_probability_;
    std::string implicit_derivative_dependencies_;
    
public:
    // default constructor sets default options
    search_options(std::string search_relationship = "0=f(0)");
    
    // test if the options are entered and in range
    bool is_valid() const;
    
    // sets default search options
    void set_default_options() { (*this) = search_options(); }
    void set_default_building_blocks();
    
    // returns a short text summary of the search options
    std::string summary() const;
    
protected:
    // boost serialization, used for sending over it the network
    // can also be used for saving/loading the data set
    template<class TArchive> void serialize(TArchive& ar, const unsigned int version);
    friend class boost::serialization::access;
};

/*---------------------------------------------------------
    Implementation:
*--------------------------------------------------------*/
namespace fitness_types
{
inline 
std::string str(int type)
{
    switch(type)
    {
    case absolute_error:       return "Absolute Error";
    case squared_error:        return "Squared Error";
    case root_squared_error:   return "Root Squared Error";
    case logarithmic_error:    return "Logarithmic Error";
    case explog_error:         return "Exponetial Logarithmic Error";
    case correlation:          return "Correlation Coefficient";
    case minimize_difference:  return "Minimize the Difference";
    case akaike_information:   return "Akaike Information Criterion";
    case bayesian_information: return "Bayesian Information Criterion";
    case maximum_error:        return "Maximum Error";
    case median_error:         return "Median Error";
    case implicit_error:       return "Implicit Derivative Error";
	case slope_error:          return "Slope Error";
    default:                   return "Unknown?";
    }
}
}

inline
search_options::search_options(std::string search_relationship) :
    search_relationship_(search_relationship),
    normalize_fitness_by_(1),
    fitness_metric_(fitness_types::absolute_error),
    solution_population_size_(64),
    predictor_population_size_(8),
    trainer_population_size_(8),
    solution_crossover_probability_(0.7f),
    solution_mutation_probability_(0.03f),
    predictor_crossover_probability_(0.5f),
    predictor_mutation_probability_(0.06f),
    implicit_derivative_dependencies_("")
{
    set_default_building_blocks();
}

inline
bool search_options::is_valid() const
{
    return (search_relationship_.length() > 0)
        && (fitness_metric_ >= 0 && fitness_metric_ < fitness_types::count)
        && (building_blocks_.size() > 0)
        && (solution_population_size_ >= 5) 
        && (predictor_population_size_ >= 5)
        && (trainer_population_size_ >= 1)
        && (solution_crossover_probability_ >= 0 && solution_crossover_probability_ <= 1)
        && (solution_mutation_probability_ >= 0 && solution_mutation_probability_ <= 1)
        && (predictor_crossover_probability_ >= 0 && predictor_crossover_probability_ <= 1)
        && (predictor_mutation_probability_ >= 0 && predictor_mutation_probability_ <= 1)
        && (fitness_metric_ != fitness_types::implicit_error || implicit_derivative_dependencies_.length() > 0)
        ;
}

inline
void search_options::set_default_building_blocks()
{
    building_blocks_.clear();
    building_blocks_.push_back("1.23");    // constants
    building_blocks_.push_back("a"); // variables
    building_blocks_.push_back("a+b"); // adds
    building_blocks_.push_back("a-b"); // subtracts
    building_blocks_.push_back("a*b"); // multiplies
    building_blocks_.push_back("a/b"); // divides
    building_blocks_.push_back("sin(a)"); // sines
    building_blocks_.push_back("cos(a)"); // cosines
}

inline 
std::string search_options::summary() const
{
    std::ostringstream os;
    if (!is_valid()) { os << "Invalid! "; }
    os << "\"" << search_relationship_ << "\"";
    os << ", " << building_blocks_.size() << " building-block types";
    os << ", " << fitness_types::str(fitness_metric_) << " fitness";
    return os.str();
}

template<typename TArchive>
inline
void search_options::serialize(TArchive& ar, const unsigned int /*version*/) 
{
    ar & BOOST_SERIALIZATION_NVP( search_relationship_ );
    ar & BOOST_SERIALIZATION_NVP( building_blocks_ );
    ar & BOOST_SERIALIZATION_NVP( normalize_fitness_by_ );
    ar & BOOST_SERIALIZATION_NVP( fitness_metric_ );
    ar & BOOST_SERIALIZATION_NVP( solution_population_size_ );
    ar & BOOST_SERIALIZATION_NVP( predictor_population_size_ );
    ar & BOOST_SERIALIZATION_NVP( trainer_population_size_ );
    ar & BOOST_SERIALIZATION_NVP( solution_crossover_probability_ );
    ar & BOOST_SERIALIZATION_NVP( solution_mutation_probability_ );
    ar & BOOST_SERIALIZATION_NVP( predictor_crossover_probability_ );
    ar & BOOST_SERIALIZATION_NVP( predictor_mutation_probability_ );
    ar & BOOST_SERIALIZATION_NVP( implicit_derivative_dependencies_ );
}

} // namespace eureqa

#endif // EUREQA_SEARCH_INFO_H
