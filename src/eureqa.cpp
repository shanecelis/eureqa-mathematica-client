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

extern "C" {
#include "mathlink.h"
}
//extern "C" int _connect( char* host);
extern "C" {

void _connect(char const* host);
void _is_connected();
void _disconnect();
void _send_data_set_maybe_labels(bool labels);
void _send_data_set();
void _send_data_set_labels();
void _send_options(char const* model);
void _start_search();
void _pause_search();
void _end_search();
void _query_progress();
void _query_frontier();
void _get_solution_frontier();
void _add_to_solution_frontier_helper();
void _clear_solution_frontier();
}

// We should make these instanceable.  Leaving it non-instanceable for
// the moment.
eureqa::connection conn;
static int next_conn_id = 1;

eureqa::solution_frontier front;
static int next_front_id = 1;

int ensure_connected(const char *s)
{
    if (! conn.is_connected()) {
        char msg[256];
        snprintf(msg, 256, "Message[%s::noconn]",s); 
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
         MLEvaluate(stdlink, (char *) "Message[ConnectTo::conn]"); 
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
        return;
    }

    // It would be nice if we respect abort requests.
    if (conn.connect(host)) {
        // We connected.
        MLPutFunction(stdlink, (char *) "ConnectionInfo", 1); 
          MLPutInteger(stdlink, next_conn_id);
        next_conn_id++;
    } else {
        MLEvaluate(stdlink, (char *) "Message[ConnectTo::err]"); 
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
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
        MLEvaluate(stdlink, (char *) "Message[Disconnect::noconn]"); 
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
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
            MLEvaluate(stdlink, (char *) "Message[SendDataSet::readerr]"); 
            MLNextPacket(stdlink); 
            MLNewPacket(stdlink); 
            MLPutSymbol(stdlink, (char *) "$Failed");
            return;
    }
    eureqa::data_set dataset(dims[0], dims[1]); // holds the data
    for(i=0; i<dims[0]; i++) 
        for(j=0; j<dims[1]; j++)
            dataset(i,j) = data[j + i * dims[1]];

    if (labels) {
        const char *lhead;
        int n;
        MLGetFunction(stdlink, &lhead, &n);
        std::cerr << "got " << lhead << " and " << n << " with dims[1] " << dims[1] << std::endl;
        if (n != dims[1]) {
            // For some reason this block of code is not working correctly.
            // The executable dies quietly when it goes through here.
            
            MLReleaseSymbol(stdlink, lhead);
            MLDisownRealArray(stdlink, data, dims, heads, d);

            MLEvaluate(stdlink, (char *) "Message[SendDataSet::colmis]"); 
            MLNextPacket(stdlink); 
            MLNewPacket(stdlink); 
            MLPutSymbol(stdlink, (char *) "$Failed");
            return;
        }
        for (int i = 0; i < n; i++) {
            const char *label;
            MLGetString(stdlink, &label);
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
        MLEvaluate(stdlink, (char *) "Message[SendDataSet::err]"); 
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
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
        MLEvaluate(stdlink, (char *) "Message[SendOptions::err]"); 
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
    }
}

void _start_search()
{
    if (ensure_connected("StartSearch")) return;
    if (conn.start_search()) {
        MLPutSymbol(stdlink, (char *) "Null");
    } else { 
        MLEvaluate(stdlink, (char *) "Message[StartSearch::err]"); 
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
    }
}

void _pause_search()
{
    if (ensure_connected("PauseSearch")) return;
    if (conn.pause_search()) {
        MLPutSymbol(stdlink, (char *) "Null");
    } else { 
        MLEvaluate(stdlink, (char *) "Message[PauseSearch::err]"); 
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
    }
}

void _end_search()
{
    if (ensure_connected("EndSearch")) return;
    if (conn.end_search()) {
        MLPutSymbol(stdlink, (char *) "Null");
    } else { 
        MLEvaluate(stdlink, (char *) "Message[EndSearch::err]"); 
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
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
        MLEvaluate(stdlink, (char *) "Message[QueryProgress::err]");
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
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
        MLEvaluate(stdlink, (char *) "Message[QueryFrontier::err]");
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
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
    next_front_id++;
}

void _get_solution_frontier() {
    put_solution_frontier(front);
}


void _add_to_solution_frontier_helper() {

    long n;
    eureqa::solution_info sol;
    if (! MLCheckFunction(stdlink, (char *) "List", &n)) {
        MLEvaluate(stdlink, (char *) "Message[AddToSolutionFrontierHelper::err]");
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
        return;
    } 

    if (n != 5) {
        MLEvaluate(stdlink, (char *) "Message[AddToSolutionFrontierHelper::invlen]");
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
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

    if (! MLGetString(stdlink, &text)) {
        MLEvaluate(stdlink, (char *) "Message[AddToSolutionFrontierHelper::geterr, \"text\"]");
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
        return;
    } 

    if (! MLGetDouble(stdlink, &score)) {
        MLEvaluate(stdlink, (char *) "Message[AddToSolutionFrontierHelper::geterr, \"score\"]");
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
        return;
    } 

    if (! MLGetDouble(stdlink, &fitness)) {
        MLEvaluate(stdlink, (char *) "Message[AddToSolutionFrontierHelper::geterr, \"fitness\"]");
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
        return;
    } 

    if (! MLGetDouble(stdlink, &complexity)) {
        MLEvaluate(stdlink, (char *) "Message[AddToSolutionFrontierHelper::geterr, \"complexity\"]");
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
        return;
    } 

    if (! MLGetInteger(stdlink, &age)) {
        MLEvaluate(stdlink, (char *) "Message[AddToSolutionFrontierHelper::geterr, \"age\"]");
        MLNextPacket(stdlink); 
        MLNewPacket(stdlink); 
        MLPutSymbol(stdlink, (char *) "$Failed");
        return;
    } 

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
