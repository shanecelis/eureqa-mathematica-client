BeginPackage["EureqaClient`"]

eureqaSymbols = {ConnectTo, Disconnect, SendDataSet, SendOptions, StartSearch, PauseSearch, EndSearch, QueryProgress, QueryFrontier, Solution, FormulaText, Fitness, Score, Complexity, Age, Generations, GenerationsPerSec, Evaluations, EvaluationsPerSec, TotalPopulationSize, FormulaTextToExpression, ClearSolutionFrontier, AddToSolutionFrontier, GetSolutionFrontier, SearchProgress, IsConnected, Link}

    Apply[Unprotect, eureqaSymbols]

    Link = Install[$UserBaseDirectory <> "/Applications/EureqaClient/eureqa"]

    ConnectTo::usage = "ConnectTo[host] connects to the Eureqa server on host."
    ConnectTo::conn = "There is already a connection."
    ConnectTo::err = "Unable to connect to Eureqa server."

    Disconnect::usage = "Disconnect[] disconnects from a Eureqa server."
    SendDataSet::usage = "SendDataSet[data] sends the data to the Eureqa server with default labels \"xi\" for each column i.\nSendDataSet[data, {l1, l2, ...}] send the data to the Eureqa server with given labels li for each column i."
    SendDataSet::readerr = "Error reading data matrix."
    SendDataSet::colmis = "Invalid number of labels: columns of data do not equal length of list of labels."
    Map[(#::noconn = "Not connected to a Eureqa server.")&, {SendDataSet,SendOptions, StartSearch, PauseSearch, EndSearch, QueryProgress, Disconnect}]
    StartSearch::err = "Error starting search."
    PauseSearch::err = "Error pausing search."
    EndSearch::err = "Error ending search."
    QueryProgress::err = "Error querying progress."
    QueryFrontier::err = "Error querying frontier."

    FormulaTextToExpression::usage = "Converts a string of the form 'f(x,y,z) = x*sin(y) + z' into an expression: x Sin[y] + z"

    FormulaTextToExpression::inval = "Invalid formula text given '``'."
    NewSolutionFrontier::usage = "NewSolutionFrontier[] returns a SolutionFrontier[id, {sol1, sol2}]"
    AddToSolutionFrontier::usage = "AddToSolutionFrontier[sol] adds a solution to the frontier and returns a SolutionFrontier[id, {sol1, sol2}]"
    SolutionInfo::badform = "Invalid form of SolutionInfo '``'."



    Begin["EureqaClient`Private`"]
    FormulaTextToExpression[""] := Null
    FormulaTextToExpression[s_String] := Module[{rhs}, If[StringCount[s, "="] == 1, rhs = StringReplace[StringSplit[s, "="][[2]], "e" -> "(10)^"], Message[FormulaTextToExpression::inval, s]; $Failed]; ToExpression[rhs, TraditionalForm]]

    get::nofield = "No such field '``' in expression '``'."
    get[s_, field_] := Module[{result}, result = field /. s; If[result === field, Message[get::nofield, field, s]; $Failed, result]]

    GetSolutionInfoHelper[sol_SolutionInfo] := Module[{list}, list = Sort[List @@ sol]; Check[ Map[get[list, #] &, {FormulaText, Score, Fitness, Complexity, Age}], Message[SolutionInfo::badform, sol]; $Failed] ]

    Apply[Protect, eureqaSymbols]
    Remove[eureqaSymbols]
    End[ ]
    EndPackage[ ]
