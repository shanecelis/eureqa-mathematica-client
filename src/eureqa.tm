
//void _connect P(( char* host));

:Begin:
:Function:       _connect
:Pattern:        ConnectTo[host_String]
:Arguments:      { host }
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
:Pattern:        SendDataSet[data_?MatrixQ]
:Arguments:      {data}
:ArgumentTypes:  {Manual}
:ReturnType:     Manual
:End:

// void _send_data_set_labels P((void));

:Begin:
:Function:       _send_data_set_labels
:Pattern:        SendDataSet[data_?MatrixQ, labels_List]
:Arguments:      {data, labels}
:ArgumentTypes:  {Manual}
:ReturnType:     Manual
:End:


// void _send_options P((char *));

:Begin:
:Function:       _send_options
:Pattern:        SendOptions[model_String]
:Arguments:      {model}
:ArgumentTypes:  {String}
:ReturnType:     Manual
:End:

// void _send_options P((void));

:Begin:
:Function:       _send_options_explicit
:Pattern:        SendOptions[options_SearchOptions]
:Arguments:      {options}
:ArgumentTypes:  {Manual}
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

:Begin:
:Function:       _add_to_solution_frontier_helper
:Pattern:        AddToSolutionFrontierHelper[text_String, score_Real, fitness_Real, complexity_Real, age_Integer]
:Arguments:      {text, score, fitness, complexity, age}
:ArgumentTypes:  {String, Real64, Real64, Real64, Integer }
:ReturnType:     Manual
:End:

