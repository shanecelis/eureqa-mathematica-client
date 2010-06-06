(* init.m

  This contains the Mathematica code for the Eureqa Client.

  Written by Shane Celis

*)
BeginPackage["EureqaClient`"];

(* Let's handle our symbol uniformly.  Unprotect so we can modify them during
  (re)loading, and protect when closing.  *)

    eureqaSymbols = {ConnectTo, 
                 Disconnect, 
                 SendDataSet, 
                 SendOptions, 
                 StartSearch, 
                 PauseSearch, 
                 EndSearch, 
                 QueryProgress, 
                 QueryFrontier, 
                 Solution, 
                 SolutionFrontier, 
                 FormulaText, 
                 Fitness, 
                 Score, 
                 Complexity, 
                 Age, 
                 Generations, 
                 GenerationsPerSec, 
                 Evaluations, 
                 EvaluationsPerSec, 
                 TotalPopulationSize, 
                 FormulaTextToExpression, 
                 ClearSolutionFrontier, 
                 AddToSolutionFrontier,                  
                 AddToSolutionFrontierHelper, 
                 SolutionFrontierGrid,
                 GetSolutionFrontier, 
                 SearchProgress, 
                 SearchProgressGrid, 
                 IsConnected}

    Apply[Unprotect, eureqaSymbols];

    ConnectTo::usage = "ConnectTo[host] connects to the Eureqa server on host.";
    ConnectTo::conn = "There is already a connection.";
    ConnectTo::err = "Unable to connect to Eureqa server.";

    Disconnect::usage = "Disconnect[] disconnects from a Eureqa server.";
    SendDataSet::usage = "SendDataSet[data] sends the data to the Eureqa server with default labels \"xi\" for each column i.\nSendDataSet[data, {l1, l2, ...}] send the data to the Eureqa server with given labels li for each column i.";
    SendDataSet::readerr = "Error reading data matrix.";
    SendDataSet::colmis = "Invalid number of labels: columns of data do not equal length of list of labels.";
    Map[(#::noconn = "Not connected to a Eureqa server.")&, {SendDataSet,SendOptions, StartSearch, PauseSearch, EndSearch, QueryProgress, Disconnect}];
    StartSearch::err = "Error starting search.";
    PauseSearch::err = "Error pausing search.";
    EndSearch::err = "Error ending search.";
    QueryProgress::err = "Error querying progress.";
    QueryFrontier::err = "Error querying frontier.";

    FormulaTextToExpression::usage = "Converts a string of the form 'f(x,y,z) = x*sin(y) + z' into an expression: x Sin[y] + z";

    FormulaTextToExpression::inval = "Invalid formula text given '``'.";
    NewSolutionFrontier::usage = "NewSolutionFrontier[] returns a SolutionFrontier[id, {sol1, sol2}]";
    AddToSolutionFrontier::usage = "AddToSolutionFrontier[sol] adds a solution to the frontier and returns a SolutionFrontier[id, {sol1, sol2}]";
    SolutionInfo::badform = "Invalid form of SolutionInfo '``'.";

    Begin["EureqaClient`Private`"]; 
    reload::usage = "Reloads the mathlink executable.";
    AddToSolutionFrontierHelper::usage = "Blah";
    load[] := Module[{}, mathlink = Install[$UserBaseDirectory <> "/Applications/EureqaClient/eureqa"]];
    unload[] := Uninstall[mathlink];
    reload[] := Module[{}, unload[]; load[]];
    If[Length[Names["EureqaClient`Private`mathlink"]] == 0,
      (* We've never seen the mathlink symbol before, so load it fresh. *)
      load[],
      (* We've seen the mathlink symbol before, so unload the last one
      before loading the new one. *)
      unload[]; load[]];
    (*mathlink = Install["57", LinkMode -> Connect];*)
    FormulaTextToExpression[""] := Null;
    FormulaTextToExpression[s_String] := Module[{rhs}, 
              If[StringCount[s, "="] == 1, 
                 rhs = StringReplace[StringSplit[s, "="][[2]], "e" -> "(10)^"], 
                 Message[FormulaTextToExpression::inval, s]; 
                 $Failed]; 
                 ToExpression[rhs, TraditionalForm]];

    get::nofield = "No such field '``' in expression '``'.";
    get[s_, field_] := Module[{result}, 
                       result = field /. s; 
                       If[result === field, 
                               Message[get::nofield, field, s]; 
                       $Failed, result]];
    getAll[s_, fields__] := Map[get[s, #]&, List[fields]];
    GetSolutionInfoHelper[sol_SolutionInfo] := Module[{list}, 
           list = Sort[List @@ sol]; 
           Check[ Map[get[list, #] &, {FormulaText, Score, Fitness, 
                                       Complexity, Age}], 
                                     Message[SolutionInfo::badform, sol]; 
                  $Failed] ];

    AddToSolutionFrontier[sol_SolutionInfo] := 
        Check[Apply[AddToSolutionFrontierHelper, GetSolutionInfoHelper[sol]], 
              Message[AddToSolutionFrontier::err]; $Failed];
    AddToSolutionFrontier[prog_SearchProgress] := 
        AddToSolutionFrontier[Solution /. (List @@ prog)];

    SolutionFrontierGrid[front_SolutionFrontier] := Module[{fields, gridItems, 
                                                            infos, header},
        fields = { Complexity, Fitness, Expression, FormulaText};
        header = {Map[Style[SymbolName[#], Bold]&, fields]};
        If[Length[front] > 0,
            infos = front[[1]] /. SolutionInfo -> List;
            gridItems = Join[header, Map[getAll[#, Apply[Sequence, fields]]&, infos]],
            gridItems = {{}}];
        Grid[gridItems, 
             ItemSize -> Scaled[1/Length[header[[1]]]], Alignment -> Left
                  (*TableSpacing -> {Automatic, 5}, TableHeadings -> header*)]]

    SearchProgressGrid[progress_SearchProgress] := Module[{fields, header},
        fields = { Generations, GenerationsPerSec, Evaluations, 
                   EvaluationsPerSec, TotalPopulationSize};
        header = {Map[Style[SymbolName[#], Bold]&, fields]};
        Grid[Join[header, {Map[get[Apply[List, progress],#]&, fields]}], 
             ItemSize -> Scaled[1/Length[fields]], Alignment -> Left
                  (*TableSpacing -> {Automatic, 5}, TableHeadings -> header*)]]


    End[];

    Apply[Protect, eureqaSymbols];
    Remove[eureqaSymbols];
    EndPackage[];
