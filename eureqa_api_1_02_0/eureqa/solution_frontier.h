#ifndef EUREQA_BEST_SOLUTIONS_H
#define EUREQA_BEST_SOLUTIONS_H

namespace eureqa
{
// information about a solution
class solution_info
{
public:
    std::string text_; // text representation of the solution
    float score_; // score, related to fitness, for ranking solutions
    float fitness_; // fitness value of the solution
    float complexity_; // complexity of the solution
    unsigned int age_; // genotypic age of the solution
    
public:
    // constructor
    solution_info(std::string text = "") : text_(text), score_(-1e30f), fitness_(-1e30f), age_(0) { }
    
    // tests if the solution dominates another solution in fitness and complexity
    bool dominates(const solution_info& s) const;
    bool matches(const solution_info& s) const;
    
protected:
    // boost serialization, used for sending over it the network
    // can also be used for saving/loading the data set
    friend class boost::serialization::access;
    template<class TArchive> void serialize(TArchive& ar, const unsigned int /*version*/);
};
inline
std::ostream& operator <<(std::ostream& os, const solution_info& r) { return os << r.text_; }


// predicate for sorting
struct by_descending_fitness;
struct by_descending_score;

// container for tracking the best solutions
class solution_frontier
{
protected:
    // set of the highest fit solutions for differnt complexities
    // ordered from most complex to least
    std::vector<solution_info> front_;
    
public:
    // adds solution to the pareto front if non-dominated
    // and removes any existing dominated by the solution
    bool add(const solution_info& soln);
    
    // tests if a solution is non-dominated and not already on the current frontier
    bool test(solution_info soln) const;
    
    // returns a text display of the frontier
    std::string to_string() const;
    
    // basic container functions
    int size() const { return (int)front_.size(); }
    solution_info& operator [](int i) { return front_[i]; }
    const solution_info& operator [](int i) const { return front_[i]; }
    void clear() { front_.clear(); }
    void remove(int i) { front_.erase(front_.begin()+i); }
    
protected:
    // boost serialization, used for sending over it the network
    // can also be used for saving/loading the data set
    friend class boost::serialization::access;
    template<class TArchive> void serialize(TArchive& ar, const unsigned int /*version*/);
};


/*---------------------------------------------------------
    Implementation:
*--------------------------------------------------------*/
struct by_descending_fitness
{
    bool operator ()(const solution_info& a, const solution_info& b) const
    {
        return (a.fitness_ > b.fitness_) 
            || (a.fitness_ == b.fitness_ && a.complexity_ < b.complexity_);
    }
};

struct by_descending_score
{
    bool operator ()(const solution_info& a, const solution_info& b) const
    {
        return (a.score_ > b.score_) 
            || (a.score_ == b.score_ && a.complexity_ < b.complexity_);
    }
};

inline
bool solution_info::dominates(const solution_info& s) const
{
    return (fitness_ >= s.fitness_ && complexity_ < s.complexity_)
        || (fitness_ > s.fitness_ && complexity_ <= s.complexity_);
}

inline
bool solution_info::matches(const solution_info& s) const
{
    return (fitness_ == s.fitness_ && complexity_ == s.complexity_);
}

inline
bool solution_frontier::add(const solution_info& soln)
{
    // test if it belongs on the frontier
    if (!test(soln)) { return false; }

    // remove dominated
    for (int i=size()-1; i>=0; --i)
    {
        if (soln.dominates(front_[i])) { remove(i); }
    }
    
    // add and sort
    front_.push_back(soln);
    std::sort(front_.begin(), front_.end(), by_descending_score());
    return true;
}

inline
bool solution_frontier::test(solution_info soln) const
{
    for (int i=0; i<size(); ++i)
    {
        if (front_[i].dominates(soln) || front_[i].matches(soln)) { return false; }
    }
    return true;
}

inline
std::string solution_frontier::to_string() const
{
    std::ostringstream os;
    os << "Size:\tFitness:\tEquation:" << std::endl;
    os << "-----\t--------\t---------" << std::endl;
    for (int i=0; i<size(); ++i)
    {
        os << front_[i].complexity_ << '\t';
        os << -front_[i].fitness_ << '\t';
        os << front_[i].text_ << std::endl;
    }
    return os.str();
}

template<class TArchive> 
inline 
void solution_info::serialize(TArchive& ar, const unsigned int /*version*/)
{
    ar & BOOST_SERIALIZATION_NVP( text_ );
    ar & BOOST_SERIALIZATION_NVP( score_ );
    ar & BOOST_SERIALIZATION_NVP( fitness_ );
    ar & BOOST_SERIALIZATION_NVP( complexity_ );
    ar & BOOST_SERIALIZATION_NVP( age_ );
}

template<class TArchive> 
inline 
void solution_frontier::serialize(TArchive& ar, const unsigned int /*version*/)
{
    ar & BOOST_SERIALIZATION_NVP( front_ );
}

} // namespace eureqa

#endif // EUREQA_BEST_SOLUTIONS_H
