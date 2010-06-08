
//void _connect P(( char* host));

:Begin:
:Function:       _connect
:Pattern:        ConnectTo[EureqaClient`Private`host_String]
:Arguments:      { EureqaClient`Private`host }
:ArgumentTypes:  { String }
:ReturnType:     Manual
:End:

// void _is_connected P(());

:Begin:
:Function:       _is_connected
:Pattern:        IsConnected[]
:Arguments:      { }
:ArgumentTypes:  { }
:ReturnType:     Manual
:End:


// void _disconnect P(());

:Begin:
:Function:       _disconnect
:Pattern:        Disconnect[]
:Arguments:      { }
:ArgumentTypes:  { }
:ReturnType:     Manual
:End:

// void _send_data_set P((void));

:Begin:
:Function:       _send_data_set
:Pattern:        SendDataSet[EureqaClient`Private`data_?MatrixQ]
:Arguments:      {EureqaClient`Private`data}
:ArgumentTypes:  {Manual}
:ReturnType:     Manual
:End:

// void _send_data_set_labels P((void));

:Begin:
:Function:       _send_data_set_labels
:Pattern:        SendDataSet[EureqaClient`Private`data_?MatrixQ, EureqaClient`Private`labels_List]
:Arguments:      {EureqaClient`Private`data, EureqaClient`Private`labels}
:ArgumentTypes:  {Manual}
:ReturnType:     Manual
:End:


// void _send_options P((char *));

:Begin:
:Function:       _send_options
:Pattern:        SendOptions[EureqaClient`Private`model_String]
:Arguments:      {EureqaClient`Private`model}
:ArgumentTypes:  {String}
:ReturnType:     Manual
:End:

// void _send_options_explicit P((int));

:Begin:
:Function:       _send_options_explicit
:Pattern:        SendOptions[EureqaClient`Private`options__Rule]
:Arguments:      {Length[List[EureqaClient`Private`options]], EureqaClient`Private`options}
:ArgumentTypes:  {Integer, Manual}
:ReturnType:     Manual
:End:


// void _end_search P((char *));

:Begin:
:Function:       _end_search
:Pattern:        EndSearch[]
:Arguments:      {}
:ArgumentTypes:  {}
:ReturnType:     Manual
:End:

// void _pause_search P((char *));

:Begin:
:Function:       _pause_search
:Pattern:        PauseSearch[]
:Arguments:      {}
:ArgumentTypes:  {}
:ReturnType:     Manual
:End:

// void _start_search P((char *));

:Begin:
:Function:       _start_search
:Pattern:        StartSearch[]
:Arguments:      {}
:ArgumentTypes:  {}
:ReturnType:     Manual
:End:

// void _query_progress P((char *));

:Begin:
:Function:       _query_progress
:Pattern:        QueryProgress[]
:Arguments:      {}
:ArgumentTypes:  {}
:ReturnType:     Manual
:End:

// void _query_frontier P((char *));

:Begin:
:Function:       _query_frontier
:Pattern:        QueryFrontier[]
:Arguments:      {}
:ArgumentTypes:  {}
:ReturnType:     Manual
:End:

// void _clear_solution_frontier P(());

:Begin:
:Function:       _clear_solution_frontier
:Pattern:        ClearSolutionFrontier[]
:Arguments:      {}
:ArgumentTypes:  { Manual }
:ReturnType:     Manual
:End:

// void _get_solution_frontier P(());

:Begin:
:Function:       _get_solution_frontier
:Pattern:        GetSolutionFrontier[]
:Arguments:      {}
:ArgumentTypes:  { Manual }
:ReturnType:     Manual
:End:


// void _add_to_solution_frontier_helper P(());

// I hope there's a better way to avoid symbol pollution than this!
:Begin:
:Function:       _add_to_solution_frontier_helper
:Pattern:        AddToSolutionFrontierHelper[EureqaClient`Private`text_String, 
                 EureqaClient`Private`score_Real, EureqaClient`Private`fitness_Real, EureqaClient`Private`complexity_Real, EureqaClient`Private`age_Integer]
:Arguments:      {EureqaClient`Private`text, EureqaClient`Private`score, EureqaClient`Private`fitness, EureqaClient`Private`complexity, EureqaClient`Private`age}
:ArgumentTypes:  {String, Real64, Real64, Real64, Integer }
:ReturnType:     Manual
:End:

