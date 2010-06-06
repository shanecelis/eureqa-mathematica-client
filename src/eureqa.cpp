/* To launch this program from within Mathematica use:
 *   In[1]:= link = Install["addtwo"]
 *
 * Or, launch this program from a shell and establish a
 * peer-to-peer connection.  When given the prompt Create Link:
 * type a port name. ( On Unix platforms, a port name is a
 * number less than 65536.  On Mac or Windows platforms,
 * it's an arbitrary word.)
 * Then, from within Mathematica use:
 *   In[1]:= link = Install["portname", LinkMode->Connect]
 */

#include <iostream>
#include <eureqa/eureqa.h>
#include <boost/unordered_map.hpp>
#include <cstring>

#define FAILED_WITH_MESSAGE(msg) \
        MLClearError(stdlink); \
        MLNewPacket(stdlink); \
        MLEvaluate(stdlink, (char *) "Message[" msg "]"); \
        MLNextPacket(stdlink); \
        MLNewPacket(stdlink); \
        MLPutSymbol(stdlink, (char *) "$Failed")

extern "C" {
#include "mathlink.h"
}

extern "C" {
void _connect(char const* host);
void _is_connected();
void _disconnect();
void _send_data_set_maybe_labels(bool labels);
void _send_data_set();
void _send_data_set_labels();
void _send_options(char const* model);
void _send_options_explicit();
void _start_search();
void _pause_search();
void _end_search();
void _query_progress();
void _query_frontier();
void _get_solution_frontier();
void _add_to_solution_frontier_helper();
void _add_to_solution_frontier_helper2(const char* text, double score, 
                                       double fitness, double complexity,
                                       int    age);
void _clear_solution_frontier();
}

eureqa::connection conn;
static int next_conn_id = 1;
eureqa::solution_frontier front;
eureqa::search_options options; // holds the search options

/*
  Let's use a generic set of classes to handle getting and setting
  integer, real, and string properties on the Eureqa members.  It'll
  make the maintenance easier.
 */
class GetSet;
boost::unordered_map<std::string, GetSet*> option_properties;

class GetSet {
    public:
    virtual int update_data(const char* sym) = 0;
};

class StringGetSet : public GetSet {
    std::string& data;
    public:
    StringGetSet(std::string& astr) : data(astr) { }
    virtual int update_data(const char* sym) {
        const char *str;
        if (MLGetNext(stdlink) != MLTKSTR) {
            FAILED_WITH_MESSAGE("SendOptions::expstr");
            return 1;
        }
        if (! MLGetString(stdlink, &str)) {
            FAILED_WITH_MESSAGE("SendOptions::failstr");
            return 2;
        }
        printf("StringGetSet got '%s' for option '%s'\n", str, sym);
        data = str;
        MLReleaseString(stdlink, str);
        return 0;
    }
};

class IntegerGetSet : public GetSet {
    int &data;
    public:
    IntegerGetSet(int &i) : data(i) { }
    virtual int update_data(const char* sym) {
        int i;
        if (MLGetNext(stdlink) != MLTKINT) {
            FAILED_WITH_MESSAGE("SendOptions::expint");
            return 1;
        }
        if (! MLGetInteger(stdlink, &i)) {
            FAILED_WITH_MESSAGE("SendOptions::failint");
            return 2;
        }
        data = i;
        return 0;
    }
};

class RealGetSet : public GetSet {
    float &data;
    public:
    RealGetSet(float &f) : data(f) { }
    virtual int update_data(const char* sym) {
        float f;
        if (MLGetNext(stdlink) != MLTKREAL && MLGetNext(stdlink) != MLTKINT) {
            FAILED_WITH_MESSAGE("SendOptions::expreal");
            return 1;
        }

        if (! MLGetFloat(stdlink, &f)) {
            FAILED_WITH_MESSAGE("SendOptions::failreal");
            return 2;
        }
        data = f;
        return 0;
    }
};

class StringListGetSet : public GetSet {
    std::vector<std::string> &data;
    public:
    StringListGetSet(std::vector<std::string> &l) : data(l) { }
    virtual int update_data(const char* sym) {
        long n;
        data.clear();
        if (! MLCheckFunction(stdlink, (char *) "List", &n)) {
            FAILED_WITH_MESSAGE("SendOptions::explist");
            return 1;
        } 
        for (int i = 0; i < n; i++) {
            long m;
            const char *str;
            if (MLGetNext(stdlink) != MLTKSTR) {
                FAILED_WITH_MESSAGE("SendOptions::expstr");
                return 2;
            }
            if (! MLGetString(stdlink, &str)) {
                FAILED_WITH_MESSAGE("SendOptions::failstr");
                return 3;
            }
            data.push_back(str);
            MLReleaseString(stdlink, str);
        }
    }
};

class MetricGetSet : public GetSet {
    int &data;
    boost::unordered_map<std::string, int> enum_values;
    public:
    MetricGetSet(int &i) : data(i) { 
        enum_values["AbsoluteError"] = eureqa::fitness_types::absolute_error;
        enum_values["SquaredError"] = eureqa::fitness_types::squared_error;
        enum_values["RootSquaredError"] = eureqa::fitness_types::root_squared_error;
        enum_values["LogarithmicError"] = eureqa::fitness_types::logarithmic_error;
        enum_values["ExplogError"] = eureqa::fitness_types::explog_error;
        enum_values["Correlation"] = eureqa::fitness_types::correlation;
        enum_values["MinimizeDifference"] = eureqa::fitness_types::minimize_difference;
        enum_values["AkaikeInformation"] = eureqa::fitness_types::akaike_information;
        enum_values["BayesianInformation"] = eureqa::fitness_types::bayesian_information;
        enum_values["MaximumError"] = eureqa::fitness_types::maximum_error;
        enum_values["MedianError"] = eureqa::fitness_types::median_error;
        enum_values["ImplicitError"] = eureqa::fitness_types::implicit_error;
        enum_values["SlopeError"] = eureqa::fitness_types::slope_error;
        enum_values["Count"] = eureqa::fitness_types::count;
    }
    virtual int update_data(const char* sym) {
        const char *symbol;
        if (! MLGetSymbol(stdlink, &symbol)) {
            FAILED_WITH_MESSAGE("SendOptions::failsym");
            return 1;
        }
        if (enum_values.find(symbol) == enum_values.end()) {
            FAILED_WITH_MESSAGE("SendOptions::expsym");
            return 2;
        }
        data = enum_values[symbol];
        return 0;
    }
};


int initialize_option_properties()
{
    if (option_properties.size() != 0)
        return 0;
    option_properties["SearchRelationship"] = new StringGetSet(options.search_relationship_);
    option_properties["BuildingBlocks"] = new StringListGetSet(options.building_blocks_);
    option_properties["NormalizeFitnessBy"] = new RealGetSet(options.normalize_fitness_by_);
    option_properties["FitnessMetric"] = new MetricGetSet(options.fitness_metric_);
    option_properties["SolutionPopulationSize"] = new IntegerGetSet(options.solution_population_size_);
    option_properties["PredictorPopulationSize"] = new IntegerGetSet(options.predictor_population_size_);
    option_properties["TrainerPopulationSize"] = new IntegerGetSet(options.trainer_population_size_);
    option_properties["SolutionCrossoverProbability"] = new RealGetSet(options.solution_crossover_probability_);
    option_properties["SolutionMutationProbability"] = new RealGetSet(options.solution_mutation_probability_);
    option_properties["PredictorCrossoverProbability"] = new RealGetSet(options.predictor_crossover_probability_);
    option_properties["PredictorMutationProbability"] = new RealGetSet(options.predictor_mutation_probability_);
}

int update_option(const char* sym) {
    if (option_properties.find(sym) == option_properties.end()) {
        // No such property
        return 1;
    }
    return option_properties[sym]->update_data(sym);
}

int ensure_connected(const char *s)
{
    if (! conn.is_connected()) {
        char msg[256];
        snprintf(msg, 256, "Message[%s::noconn]",s); 
        MLClearError(stdlink); 
        MLNewPacket(stdlink);
        MLEvaluate(stdlink, msg);
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, "$Failed");
        return 1; // error
    }
    return 0; // no error; connected
    
}

void _connect(char const* host)
{
    if (conn.is_connected()) {
        FAILED_WITH_MESSAGE("ConnectTo::conn");
        return;
    }

    // It would be nice if we respect abort requests.
    if (conn.connect(host)) {
        // We connected.
        MLPutFunction(stdlink, (char *) "ConnectionInfo", 1); 
          MLPutInteger(stdlink, next_conn_id);
        next_conn_id++;
    } else {
        FAILED_WITH_MESSAGE("ConnectTo::err");
    }
}

void _is_connected()
{
    if (conn.is_connected()) {
        MLPutSymbol(stdlink, (char *) "True");
    } else {
        MLPutSymbol(stdlink, (char *) "False");
    }
}

void _disconnect()
{
    if (! conn.is_connected()) {
        FAILED_WITH_MESSAGE("Disconnect::noconn");
        return;
    }

    // It would be nice if we respect request for an abort.
    conn.disconnect();
    MLPutSymbol(stdlink, (char *) "Null");
}

void _send_data_set_maybe_labels(bool labels) {
    if (ensure_connected((char *) "SendDataSet")) return;
    double *data;
    long *dims;                 // dimensions
    char **heads;
    long d; /* stores the rank of the array */
    long i,j;

    if(! MLGetRealArray(stdlink, &data, &dims, &heads, &d)) {
        FAILED_WITH_MESSAGE("SendDataSet::readerr");
        return;
    }
    eureqa::data_set dataset(dims[0], dims[1]); // holds the data
    for(i=0; i<dims[0]; i++) 
        for(j=0; j<dims[1]; j++)
            dataset(i,j) = data[j + i * dims[1]];

    if (labels) {
        const char *lhead;
        int n;
        if (! MLGetFunction(stdlink, &lhead, &n)) {
            FAILED_WITH_MESSAGE("SendDataSet::invarg");
            return;
        }
        if (n != dims[1]) {
            // For some reason this block of code is not working correctly.
            // The executable dies quietly when it goes through here.
            
            MLReleaseSymbol(stdlink, lhead);
            MLDisownRealArray(stdlink, data, dims, heads, d);
            FAILED_WITH_MESSAGE("SendDataSet::colmis");
            return;
        }
        for (int i = 0; i < n; i++) {
            const char *label;
            if (!MLGetString(stdlink, &label)) {
                FAILED_WITH_MESSAGE("SendDataSet::invarg");
                return;
            }
            dataset.X_symbols_[i] = label;
            MLReleaseString(stdlink, label);
        }

        MLReleaseSymbol(stdlink, lhead);
    }

    std::cerr << dataset.summary() << std::endl;
    if (conn.send_data_set(dataset)) {
        // Everything went well.  Send through the data we received.
        MLPutDoubleArray(stdlink, data, dims, heads, d);
        MLDisownRealArray(stdlink, data, dims, heads, d);
    } else {
        MLDisownRealArray(stdlink, data, dims, heads, d);
        FAILED_WITH_MESSAGE("SendDataSet::err");
    }
}

void _send_data_set()
{
    _send_data_set_maybe_labels(false);
}

void _send_data_set_labels() 
{
    _send_data_set_maybe_labels(true);
}

void _send_options(char const* model)
{
    if (ensure_connected("SendOptions")) return;
    eureqa::search_options options(model); // holds the search options
    std::cerr << options.summary() << std::endl;
    if (conn.send_options(options)) {
        MLPutSymbol(stdlink, (char *) "Null");        
    } else {
        FAILED_WITH_MESSAGE("SendOptions::err");
    }
}

void _send_options_explicit()
{
    printf("BEGIN _send_options_explicit\n");
    if (ensure_connected("SendOptions")) return;
    
    //if (...)
    initialize_option_properties();
    options.set_default_options();
    options.set_default_building_blocks();
    //std::cerr << options.summary() << std::endl;
    long n;
    if (! MLCheckFunction(stdlink, (char *) "SearchOptions", &n)) {
        FAILED_WITH_MESSAGE("SendOptions::invarg");
        return;
    } 
    printf("n = %ld\n", n);
    for (int i = 0; i < n; i++) {
        long m;
        const char *head;
        if (! MLCheckFunction(stdlink, "Rule", &m)) {
            FAILED_WITH_MESSAGE("SendOptions::invarg2");
            return;
        } 
        printf("m = %ld\n", m);

        // if (! MLCheckFunction(stdlink, (char *) "Rule", &m)) {
        //     FAILED_WITH_MESSAGE("SendOptions::invarg2");
        //     return;
        // } 
        if (m != 2) {
            FAILED_WITH_MESSAGE("SendOptions::invarg3");
            return;
        }

        int o;
        const char *sym;
        if (! MLGetSymbol(stdlink, &sym)) {
            FAILED_WITH_MESSAGE("SendOptions::invarg4");
            return;
        } 
        printf("Got options for '%s'\n", sym);
        int err = update_option(sym);
        printf("err =  '%d'\n", err);
        // const char *formula;
        // if (! MLGetString(stdlink, &formula)) {
        //     FAILED_WITH_MESSAGE("SendOptions::invarg5");
        //     return;
        // }
        // options.search_relationship_ = formula;
    }
    
    if (conn.send_options(options)) {
        MLPutSymbol(stdlink, (char *) "Null");        
    } else {
        FAILED_WITH_MESSAGE("SendOptions::err");
    }
}


void _start_search()
{
    if (ensure_connected("StartSearch")) return;
    if (conn.start_search()) {
        MLPutSymbol(stdlink, (char *) "Null");
    } else { 
        FAILED_WITH_MESSAGE("StartSearch::err");
    }
}

void _pause_search()
{
    if (ensure_connected("PauseSearch")) return;
    if (conn.pause_search()) {
        MLPutSymbol(stdlink, (char *) "Null");
    } else { 
        FAILED_WITH_MESSAGE("PauseSearch::err");
    }
}

void _end_search()
{
    if (ensure_connected("EndSearch")) return;
    if (conn.end_search()) {
        MLPutSymbol(stdlink, (char *) "Null");
    } else { 
        FAILED_WITH_MESSAGE("EndSearch::err");
    }
}

void put_solution_info(eureqa::solution_info solution)
{

    // SolutionInfo[FormulaText -> "", Score -> .1, Fitness -> .1, Complexity -> .1, Age -> 1]
    int error;
    MLINK loopback = MLLoopbackOpen(stdenv, &error);
    if(loopback == (MLINK)0 || error != MLEOK) { 
        /* unable to create loopbacklink */ 
        loopback = 0;
    } else {
        long n;
        char buf[256];
        snprintf(buf, 256, "FormulaTextToExpression[\"%s\"]", solution.text_.c_str());
        MLEvaluate(loopback, buf);
        // MLPutFunction(loopback, "EvaluatePacket", 1);
        //   MLPutFunction(loopback, "FormulaTextToExpression", 1);
        //   MLPutString(loopback, solution.text_.c_str());
        // MLEndPacket(loopback);
        //MLNextPacket(loopback);
        if (! MLCheckFunction(loopback, "EvaluatePacket", &n)) {
            MLClose(loopback);
            // Should put a message out there.
            
            loopback = 0;
        } 

    }

        MLPutFunction(stdlink, (char *) "SolutionInfo", loopback ? 6 : 5); 
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "FormulaText");
            MLPutString(stdlink, solution.text_.c_str());
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "Score");
            MLPutDouble(stdlink, solution.score_);

          if (loopback) {
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "Expression");
            MLTransferExpression(stdlink, loopback);
            MLClose(loopback);
            loopback = 0;
          }            

          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "Fitness");
            MLPutDouble(stdlink, solution.fitness_);
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "Complexity");
            MLPutDouble(stdlink, solution.complexity_);
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "Age");
            MLPutInteger(stdlink, solution.age_);
}

void get_solution_info(eureqa::solution_info& solution)
{
    long n;
    if (! MLCheckFunction(stdlink, (char *) "SolutionInfo", &n)) {    
        std::cerr << " not a solution info input\n";
    }
    // NYI 

}

void _query_progress()
{
    if (ensure_connected("QueryProgress")) return;
    eureqa::search_progress progress; // recieves the progress and new solutions
    if (conn.query_progress(progress)) {
        // SearchProgress[Solution -> soln, Generations -> g, GenerationsPerSec -> gps, Evaluations -> e, EvaluationsPerSec -> eps, TotalPopulationSize -> s]
        MLPutFunction(stdlink, (char *) "SearchProgress", 6); 
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "Solution");
            put_solution_info(progress.solution_);
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "Generations");
            MLPutDouble(stdlink, progress.generations_);
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "GenerationsPerSec");
            MLPutDouble(stdlink, progress.generations_per_sec_);
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "Evaluations");
            MLPutDouble(stdlink, progress.evaluations_);
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "EvaluationsPerSec");
            MLPutDouble(stdlink, progress.evaluations_per_sec_);
          MLPutFunction(stdlink, (char *) "Rule", 2);
            MLPutSymbol(stdlink, (char *) "TotalPopulationSize");
            MLPutDouble(stdlink, progress.total_population_size_);
    } else {
        FAILED_WITH_MESSAGE("QueryProgress::err");
    }
}

void _query_frontier() {
    eureqa::solution_frontier front;
    if (conn.query_frontier(front)) {
        MLPutFunction(stdlink, (char *) "SolutionFrontier", front.size()); 
        for (int i = 0; i < front.size(); i++) {
            put_solution_info(front[i]);
        }
    } else {
        FAILED_WITH_MESSAGE("QueryFrontier::err");
    }
}

void put_solution_frontier(eureqa::solution_frontier &front) {
    MLPutFunction(stdlink, (char *) "SolutionFrontier", 1); 
      MLPutFunction(stdlink, (char *) "List", front.size());
        //MLPutInteger(stdlink, next_front_id);
        for (int i = 0; i < front.size(); i++) {
          put_solution_info(front[i]);
        } 
}

void _clear_solution_frontier() {
    front.clear();
    put_solution_frontier(front);
}

void _get_solution_frontier() {
    put_solution_frontier(front);
}


void _add_to_solution_frontier_helper() {

    printf("BEGIN add_to_solution_frontier_helper\n");
    long n;
    eureqa::solution_info sol;
    printf("1 add_to_solution_frontier_helper\n");

    if (! MLCheckFunction(stdlink, (char *) "List", &n)) {
        FAILED_WITH_MESSAGE("AddToSolutionFrontierHelper::err");
        return;
    } 
    printf("2 add_to_solution_frontier_helper\n");
    if (n != 5) {
        FAILED_WITH_MESSAGE("AddToSolutionFrontierHelper::invlen");
        return;
    }
    // I'm going to pull the members the same way they're place onto the list
    // by GetSolutionInfoHelper:
    // Map[get[list, #] &, {FormulaText, Score, Fitness, Complexity, Age}],
    const char* text;
    double score;
    double fitness;
    double complexity;
    int    age;

    printf("3 add_to_solution_frontier_helper\n");
    if (! MLGetString(stdlink, &text)) {
        FAILED_WITH_MESSAGE("AddToSolutionFrontierHelper::geterr");
        return;
    } 

    printf("4 add_to_solution_frontier_helper\n");
    if (! MLGetDouble(stdlink, &score)) {
        FAILED_WITH_MESSAGE("AddToSolutionFrontierHelper::geterr");
        return;
    } 

    printf("5 add_to_solution_frontier_helper\n");
    if (! MLGetDouble(stdlink, &fitness)) {
        FAILED_WITH_MESSAGE("AddToSolutionFrontierHelper::geterr");
        return;
    } 

    printf("6 add_to_solution_frontier_helper\n");
    if (! MLGetDouble(stdlink, &complexity)) {
        FAILED_WITH_MESSAGE("AddToSolutionFrontierHelper::geterr");
        return;
    } 

    printf("7 add_to_solution_frontier_helper\n");
    if (! MLGetInteger(stdlink, &age)) {
        FAILED_WITH_MESSAGE("AddToSolutionFrontierHelper::geterr");
        return;
    } 

    printf("8 add_to_solution_frontier_helper\n");
    // XXX I might leak this string if anything fails above.
    sol.text_ = text;
    MLReleaseString(stdlink, text);
    sol.score_ = score;
    sol.fitness_ = fitness;
    sol.complexity_ = complexity;
    sol.age_ = age;
    front.add(sol);
    put_solution_frontier(front);
}

void _add_to_solution_frontier_helper2(const char* text, double score, 
                                       double fitness, double complexity,
                                       int    age)
{
    eureqa::solution_info sol;
    sol.text_ = text;
    sol.score_ = score;
    sol.fitness_ = fitness;
    sol.complexity_ = complexity;
    sol.age_ = age;
    front.add(sol);
    put_solution_frontier(front);
}



#if WINDOWS_MATHLINK

#if __BORLANDC__
#pragma argsused
#endif

int PASCAL WinMain( HINSTANCE hinstCurrent, HINSTANCE hinstPrevious, LPSTR lpszCmdLine, int nCmdShow)
{
	char  buff[512];
	char FAR * buff_start = buff;
	char FAR * argv[32];
	char FAR * FAR * argv_end = argv + 32;

	hinstPrevious = hinstPrevious; /* suppress warning */

	if( !MLInitializeIcon( hinstCurrent, nCmdShow)) return 1;
	MLScanString( argv, &argv_end, &lpszCmdLine, &buff_start);
	return MLMain( (int)(argv_end - argv), argv);
}

#else

int main(int argc, char* argv[])
{
	return MLMain(argc, argv);
}

#endif
