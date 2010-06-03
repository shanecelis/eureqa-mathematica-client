:Evaluate: BeginPackage["EureqaClient`"]
:Evaluate:  eureqaSymbols = {ConnectTo, Disconnect, SendDataSet, SendOptions, StartSearch, PauseSearch, EndSearch, QueryProgress, QueryFrontier, Solution, FormulaText, Fitness, Score, Complexity, Age, Generations, GenerationsPerSec, Evaluations, EvaluationsPerSec, TotalPopulationSize, FormulaTextToExpression, ClearSolutionFrontier, AddToSolutionFrontier, GetSolutionFrontier, SearchProgress, IsConnected}
:Evaluate:  Apply[Unprotect, eureqaSymbols]

:Evaluate:	ConnectTo::usage = "ConnectTo[host] connects to the Eureqa server on host."
:Evaluate:  ConnectTo::conn = "There is already a connection."
:Evaluate:  ConnectTo::err = "Unable to connect to Eureqa server."

:Evaluate:  Disconnect::usage = "Disconnect[] disconnects from a Eureqa server."
:Evaluate:  SendDataSet::usage = "SendDataSet[data] sends the data to the Eureqa server with default labels \"xi\" for each column i.\nSendDataSet[data, {l1, l2, ...}] send the data to the Eureqa server with given labels li for each column i."
:Evaluate:  SendDataSet::readerr = "Error reading data matrix."
:Evaluate:  SendDataSet::colmis = "Invalid number of labels: columns of data do not equal length of list of labels."
:Evaluate:  Map[(#::noconn = "Not connected to a Eureqa server.")&, {SendDataSet,SendOptions, StartSearch, PauseSearch, EndSearch, QueryProgress, Disconnect}]
:Evaluate:  StartSearch::err = "Error starting search."
:Evaluate:  PauseSearch::err = "Error pausing search."
:Evaluate:  EndSearch::err = "Error ending search."
:Evaluate:  QueryProgress::err = "Error querying progress."
:Evaluate:  QueryFrontier::err = "Error querying frontier."

:Evaluate:  FormulaTextToExpression::usage = "Converts a string of the form 'f(x,y,z) = x*sin(y) + z' into an expression: x Sin[y] + z"

:Evaluate:  FormulaTextToExpression::inval = "Invalid formula text given '``'."
:Evaluate:  NewSolutionFrontier::usage = "NewSolutionFrontier[] returns a SolutionFrontier[id, {sol1, sol2}]"
:Evaluate:  AddToSolutionFrontier::usage = "AddToSolutionFrontier[front, sol] adds a solution to the frontier and returns a SolutionFrontier[id, {sol1, sol2}]"
:Evaluate:  SolutionInfo::badform = "Invalid form of SolutionInfo '``'."



:Evaluate: Begin["EureqaClient`Private`"]
:Evaluate:  FormulaTextToExpression[""] := Null
:Evaluate:  FormulaTextToExpression[s_String] := Module[{rhs}, If[StringCount[s, "="] == 1, rhs = StringReplace[StringSplit[s, "="][[2]], "e" -> "(10)^"], Message[FormulaTextToExpression::inval, s]; $Failed]; ToExpression[rhs, TraditionalForm]]

:Evaluate: get::nofield = "No such field '``' in expression '``'."
:Evaluate: get[s_, field_] := Module[{result}, result = field /. s; If[result === field, Message[get::nofield, field, s]; $Failed, result]]

:Evaluate: GetSolutionInfoHelper[sol_SolutionInfo] := Module[{list}, list = Sort[List @@ sol]; Check[ Map[get[list, #] &, {FormulaText, Score, Fitness, Complexity, Age}], Message[SolutionInfo::badform, sol]; $Failed] ]

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


// void _add_to_solution_frontier P(());

:Evaluate: AddToSolutionFrontier[sol_SolutionInfo] := Check[AddToSolutionFrontierHelper[GetSolutionInfoHelper[sol]], Message[AddToSolutionFrontier::err]; $Failed]

:Evaluate: AddToSolutionFrontier[prog_SearchProgress] := AddToSolutionFrontier[Solution /. (List @@ prog)]

:Begin:
:Function:       _add_to_solution_frontier_helper
:Pattern:        AddToSolutionFrontierHelper[members_List]
:Arguments:      {members}
:ArgumentTypes:  { Manual }
:ReturnType:     Manual
:End:

:Evaluate:  Apply[Protect, eureqaSymbols]
:Evaluate:  Remove[eureqaSymbols]
:Evaluate:	End[ ]
:Evaluate:	EndPackage[ ]
