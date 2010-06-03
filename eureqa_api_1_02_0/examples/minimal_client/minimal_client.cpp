#include <iostream>
#include <time.h>

#include <eureqa/eureqa.h>

#ifdef WIN32
inline void sleep(int sec) { Sleep(sec*1000); }
#endif

int main(int argc, char *argv[])
{
    std::cout << "The minimal_client attempts to connect to a Eureqa Server ";
    std::cout << "running on the local machine and perform a search with ";
    std::cout << "very little error checking. It is meant to be used as a ";
    std::cout << "source code example." << std::endl;
    std::cout << std::endl;
    
    // initialize data and options
    eureqa::data_set data("../data_sets/default_data.txt"); // holds the data
    eureqa::search_options options("y = f(x)"); // holds the search options

    std::cout << "Data: " << data.summary() << std::endl;
    std::cout << "Options: " << options.summary() << std::endl;
    
    // connect to a eureqa server running on the local machine
    eureqa::connection conn("127.0.0.1"); // interface to a server
    eureqa::server_info server; // receives info about the server
    conn.query_server_info(server);
    conn.send_data_set(data);
    conn.send_options(options);
    conn.start_search();
    
    std::cout << "Connection: " << conn.summary() << std::endl;
    std::cout << "Server: " << server.summary() << std::endl;
    
    // monitor the search
    eureqa::search_progress progress; // recieves the progress and new solutions
    eureqa::solution_frontier best_solutions; // filters out the best solutions
    
    // continue searching (until user hits ctrl-c)
    while (conn.query_progress(progress))
    {
        // print the progress (e.g. number of generations)
        std::cout << progress.summary() << std::endl;
        
        // the eureqa server sends a stream of new solutions in the progress
        // here we filter out and store only the pareto-optimal solutions
        if (best_solutions.add(progress.solution_))
        {
            // print a display of the best solutions
            std::cout << best_solutions.to_string() << std::endl;
        }
        sleep(1);
    }
    
    std::cout << std::endl;
    #ifdef WIN32
    system("pause");
    #endif
    return 0;
}
