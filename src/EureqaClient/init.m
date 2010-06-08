(* init.m

  This file contains the Mathematica portion of the code for the
  Eureqa Client.

  Written by Shane Celis.

*)
BeginPackage["EureqaClient`"];

(* Let's handle our symbols uniformly.  Unprotect so we can modify them during
  (re)loading, and protect when closing.  *)

  symbolGroups = Hold[SendOptionsOptions, SolutionInfoOptions,
                  SearchProgressOptions, EureqaSearchOptions,
                  FitnessMetrics]; (* Hold is like quote in Lisp. *)
  (* Unprotect for cases of reloading a package.*)
  Unprotect@@symbolGroups;
    (* Maybe I should put these in their own packages to split them up finer. *)
    (* Arguments to SendOptions *)
    SendOptionsOptions = {SearchRelationship,
                     BuildingBlocks,
                     FitnessMetric,
                     NormalizeFitnessBy,
                     SolutionPopulationSize,
                     PredictorPopulationSize,
                     TrainerPopulationSize,
                     SolutionCrossoverProbability,
                     SolutionMutationProbability,
                     PredictorCrossoverProbability,
                     PredictorMutationProbability};
                 (* Tags for SolutionInfo *)
    SolutionInfoOptions = {
                     FormulaText, 
                     Fitness, 
                     Score, 
                     Complexity, 
                     Age, 
                     Expression};
                 (* Tags for SearchProgress *)
    SearchProgressOptions = {
                  Solution, 
                  Generations, 
                  GenerationsPerSec, 
                  Evaluations, 
                  EvaluationsPerSec, 
                  TotalPopulationSize};
    EureqaSearchOptions = {
                 (* Arguments to EureqaSearch *)
                  Host,
                  VariableLabels,
                  MaxGenerations,
                  TerminateCondition,
                  UpdatesPerSecond,
                  DisplaySolutionFrontier,
                  DisplaySearchProgress};
    FitnessMetrics = {
                 (* Fitness Metrics *)
                  AbsoluteError,
                  SquaredError,
                  RootSquaredError,
                  LogarithmicError,
                  ExplogError,
                  Correlation,
                  MinimizeDifference,
                  AkaikeInformation,
                  BayesianInformation,
                  MaximumError,
                  MedianError,
                  ImplicitError,
                  SlopeError,
                  Count};

    eureqaSymbols = Join[
                (* Make sure we handle all groups of symbols and the
                symbols they contain uniformly. *)
                ReleaseHold[symbolGroups],
                {
                 (* Ugh, all I'm trying to do is have the names of my
                 symbol groups taken care of too, so it's not only the
                 contents of the symbolGroups, but also the
                 symbolGroups themselves. *)
                 ReleaseHold[SymbolName/@Unevaluated/@symbolGroups],
                (* MathLink Functions *)
                 ConnectTo, 
                 Disconnect, 
                 SendOptions, 
                 SendDataSet, 
                 StartSearch, 
                 PauseSearch, 
                 EndSearch, 
                 QueryProgress, 
                 QueryFrontier, 
                 ClearSolutionFrontier, 
                 AddToSolutionFrontierHelper, 
                 GetSolutionFrontier, 
                 SearchProgressGrid, 
                 IsConnected,
                (* Tags or objects *)
                 SolutionFrontier, 
                 SolutionInfo, 
                 ConnectionInfo,
                 SearchProgress, 
                 (* Mathematica Functions *)
                 AddToSolutionFrontier,                  
                 FormulaTextToExpression, 
                 SolutionFrontierGrid,
                 EureqaSearch,
                 GetField,
                 GetFields,
                 PutField,
                 HasField,
                 Fields,
                 FieldValues,
                 SolutionFrontierToMatrix,
                 (*SolutionFrontierToMatrix Options *)
                  IncludeFieldNames,
                 (* Values *)
                 DefaultBuildingBlocks
                 }];

    Apply[Unprotect, eureqaSymbols];

    ConnectTo::usage = "ConnectTo[host] connects to the Eureqa server on host.";
    ConnectTo::conn = "There is already a connection.";
    ConnectTo::err = "Unable to connect to Eureqa server.";

    Disconnect::usage = "Disconnect[] disconnects from a Eureqa server.";
    SendDataSet::usage = "SendDataSet[data] sends the data to the Eureqa server with default labels \"xi\" for each column i.\nSendDataSet[data, {l1, l2, ...}] send the data to the Eureqa server with given labels li for each column i.";
    SendDataSet::readerr = "Error reading data matrix.";
    SendDataSet::colmis = "Invalid number of labels: columns of data do not equal length of list of labels.";
    SendDataSet::invarg = "Invalid argument.";
    SendDataSet::err = "Generic error.";
    Map[(#::noconn = "Not connected to a Eureqa server.")&, {SendDataSet,SendOptions, StartSearch, PauseSearch, EndSearch, QueryProgress, Disconnect}];
    StartSearch::err = "Error starting search.";
    PauseSearch::err = "Error pausing search.";
    EndSearch::err = "Error ending search.";
    QueryProgress::err = "Error querying progress.";
    QueryProgress::arcerr = "Internal error: got an archive exception, probably due to an error parsing the search relationship equation.";
    QueryFrontier::err = "Error querying frontier.";
    SendOptions::usage = "SendOptions[model] sets the search relationship with the other options left at their default.\nSendOptions[SearchRelationship -> model, SolutionPopulationSize -> 100, ...] allows you to specify many options.  See Options[SendOptions] for a full list.";
    SendOptions::inv = "Invalid SearchOptions provided.";
    SendOptions::expstr = "Expected a string.";
    SendOptions::expstr1 = "Expected a string for option '``'.";
    SendOptions::expstr2 = "Expected a string for option '``' instead got '``'.";
    SendOptions::failstr = "Failed to retrieve string.";
    SendOptions::expint = "Expected an integer.";
    SendOptions::expint1 = "Expected an integer for option '``'.";
    SendOptions::failint = "Failed to retrieve integer.";
    SendOptions::expreal = "Expected a real number.";
    SendOptions::expreal1 = "Expected a real number for option '``'.";
    SendOptions::failreal = "Failed to retrieve real number.";
    SendOptions::failmsym1 = "Failed to see if there was an 'Automatic' symbol for option '``'.";
    SendOptions::explist = "Expected a list.";
    SendOptions::explist1 = "Expected a list for option '``'.";
    SendOptions::expsym = "Expected a symbol.";
    SendOptions::expsym1 = "Expected a symbol for option '``'.";
    SendOptions::invsym = "Invalid symbol given.";
    SendOptions::invsym1 = "Invalid symbol given for option '``'.";
    SendOptions::invopt = "Invalid option symbol given.";
    SendOptions::invopt1 = "Invalid option symbol given '``'.";
    SendOptions::failsym = "Failed to retrieve symbol.";
    SendOptions::failsym1 = "Failed to retrieve symbol for option '``'.";
    SendOptions::exprule = "Expected a options specified  with Rule funtions, e.g. SearchOptions[SearchRelationship -> \"D(x,t) = f(x,y,z,t)\".";
    SendOptions::expruletwo = "Expected each Rule to have two arguments.  Note: a -> b == Rule[a,b].";
    SendOptions::exprsym = "Expected each Rule's first argument to be a symbol.";
    SendOptions::senderr = "Error sending options.";

    FormulaTextToExpression::usage = "Converts a string of the form 'f(x,y,z) = x*sin(y) + z' into an expression: x Sin[y] + z";

    FormulaTextToExpression::inval = "Invalid formula text given '``'.";
    NewSolutionFrontier::usage = "NewSolutionFrontier[] returns a SolutionFrontier[id, {sol1, sol2}]";
    AddToSolutionFrontier::usage = "AddToSolutionFrontier[sol] adds a solution to the frontier and returns a SolutionFrontier[id, {sol1, sol2}]";
    SolutionInfo::badform = "Invalid form of SolutionInfo '``'.";
    DefaultBuildingBlocks = {"1.23", "a", "a+b", "a-b", "a*b", "a/b", "sin(a)", "cos(a)"};
    GetField::nofield = "No such field '``' in expression '``'.";
    GetField::usage = "GetField[Structure[A -> a, B -> b], A] returns a";
    Fields::usage = "Fields[{A -> a, B -> b}] returns {A, B}";
    HasField::usage = "HasField[{A -> a, B -> b}, A] returns True";
    PutField::usage = "PutField[{A -> a, B -> b}, C, c] returns {A -> a, B-> b, C-> c}";
    FieldValues::usage = "FieldValues[{A -> a, B -> b}] returns {a, b}";

    SolutionFrontierToMatrix::usage = "Converts a solution frontier from a set of solution info field objects into a matrix of values including the names of the fields.";
    TerminateCondition::usage = "Option used with EureqaSearch to specify a function f[progress, solutionFrontier] that returns True to terminate a search.";
    StepMonitor::usage = "Option used with EureqaSearch to specify a function that is run on every update interval.  It is not provided with any arguments, but it can query the running server with QueryProgress[] and GetSolutionFrontier[] functions.";
    EureqaSearch::usage = "EureqaSearch[data, model] searches for the kind of relationship specified by the model in the data.  Some important options are Host, VariableLabels, BuildingBlocks.  See all the options this function accepts by evaluating Options[EureqaSearch].";
    FitnessMetric::usage = "Option used to specify how fitness should be evaluated.  Evaluate FitnessMetrics to see the values available.";
    Host::usage = "Option used to specify what host to connect to.";
    VariableLabels::usage = "Option used to specify the labels of each column of data.  If not specified the labels used by default are x1, x2, ....";

    (*AllBuildingBlocks = Where are these defined?*)

    Begin["EureqaClient`Private`"]; 
    (* The actual function implementations after this point. *)

    SendDataSet[data_, Automatic] := SendDataSet[data];
    reload::usage = "Reloads the mathlink executable.";
    (*Set EureqaClient`Private`linkName = "XXX" so that you can run
    the mathlink executable in a debugger or see its output.  It will
    ask for a name at startup, give it your choice for "XXX".  *)
    If[Length[Names["EureqaClient`Private`linkName"]] === 0,  
        EureqaClient`Private`linkName = None];
    load[] :=  (*mathlink = Install["4", LinkMode -> Connect]; *)
               mathlink = Install[$UserBaseDirectory <> "/Applications/EureqaClient/eureqaml"];
    (*If[Length[Names["EureqaClient`Private`linkName"]] === 0 || linkName === None, *)
                  (* Never seen the linkName symbol, load the default link. *)
(*                  mathlink = Install[$UserBaseDirectory <> "/Applications/EureqaClient/eureqaml"]*)(*, 
                  mathlink = Install[linkName, LinkMode -> Connect]]; *)
    unload[] := Uninstall[mathlink]; 
    reload[] := Module[{}, unload[]; load[]];
    If[Length[Names["EureqaClient`Private`mathlink"]] === 0,
      (* We've never seen the mathlink symbol before, so load it fresh. *)
      load[],
      (* We've seen the mathlink symbol before, so unload the last one
      before loading the new one. *)
      unload[]; load[]];


    FormulaTextToExpression[""] := Null;
    FormulaTextToExpression[s_String] := Module[{rhs}, 
              If[StringCount[s, "="] == 1, 
                 rhs = StringReplace[StringSplit[s, "="][[2]], "e" -> "(10)^"], 
                 Message[FormulaTextToExpression::inval, s]; 
                 $Failed]; 
                 ToExpression[rhs, TraditionalForm]];
    Fields[s_List] := s /. Rule[f_Symbol, _] -> f;
    Fields[s_] := Fields[List@@s];
    FieldValues[s_List] := s /. Rule[_, v_] -> v;
    FieldValues[s_] := FieldValues[List@@s];
    HasField[s_, field_Symbol] := MemberQ[Fields[s], field]
    PutField[s_, field_Symbol, value_] := If[HasField[s, field],
                                      s /. Rule[field, _] -> Rule[field, value],
                                      Append[s, Rule[field, value]]]

    GetField[s_List, field_Symbol] := Module[{result}, 
                                   result = field /. s; 
                                   If[result === field, 
                                       Message[GetField::nofield, field, s]; 
                                   $Failed, result]];
    GetField[s_, field_Symbol] := GetField[Apply[List, s], field];

    GetFields[s_, fields__Symbol] := Map[GetField[s, #]&, List[fields]];
    GetSolutionInfoHelper[sol_SolutionInfo] := Module[{list}, 
           list = Sort[List @@ sol]; 
           Check[ Map[GetField[list, #] &, {FormulaText, Score, Fitness, 
                                       Complexity, Age}], 
                                     Message[SolutionInfo::badform, sol]; 
                  $Failed] ];
    AddToSolutionFrontier[sol_SolutionInfo] := 
        Check[Apply[AddToSolutionFrontierHelper, GetSolutionInfoHelper[sol]], 
              Message[AddToSolutionFrontier::err]; $Failed];
    AddToSolutionFrontier[prog_SearchProgress] := 
        AddToSolutionFrontier[Solution /. (List @@ prog)];

    Options[SolutionFrontierToMatrix] = {IncludeFieldNames -> True};
    
    SolutionFrontierToMatrix[front_SolutionFrontier, opts : OptionsPattern[]] := 
     Module[{fields, values, part},
      If[Length[front] == 0,
       	  Return[{}]];
      fields = Sort[Fields[front[[1]]]];
      (* We want to sort by the score. *)
      If[FreeQ[fields, Score],
            Message[SolutionFrontierToMatrix::noscore];
            Return[$Failed]];
      part = Position[fields, Score][[1,1]];
      values = SortBy[Map[GetFields[#, Apply[Sequence, fields]] &, 
               List @@ front], -Part[#, part] &];
      If[OptionValue[IncludeFieldNames],
         Join[{fields}, values],
         values]]
    (*SolutionFrontierToMatrix[front_SolutionFrontier] := *)
    SolutionFrontierGrid[front_SolutionFrontier] := Module[{fields, gridItems, 
                                                            infos, header},
        gridItems = SolutionFrontierToMatrix[front];
        fields = gridItems[[1]];
        header = {Map[Style[SymbolName[#], Bold]&, fields]};
        If[Length[front] > 0,
            gridItems = Join[header, Rest[gridItems]],
            gridItems = {header}];
        Framed[Grid[gridItems, 
             ItemSize -> Scaled[1/Length[header[[1]]]], Alignment -> Left
             ]]
        ]

    SearchProgressGrid[progress_SearchProgress] := Module[{fields, header},
        fields = { Generations, GenerationsPerSec, Evaluations, 
                   EvaluationsPerSec, TotalPopulationSize};
        header = {Map[Style[SymbolName[#], Bold]&, fields]};
        Framed[Grid[
             Transpose[Join[header, {Map[GetField[progress,#]&, fields]}]], 
             ItemSize -> Scaled[1/4], Alignment -> Left
            ]]
        ]

    (* This is kind of weird because I don't use the OptionValue in
    SendOptions.  In fact SendOptions is implemented in C, but I
    wanted to make sure that Option like modules could be accessed by
    the Option function for documentation purposes. *)
    Options[SendOptions] = Map[Rule[#, Automatic]&, SendOptionsOptions];

    Options[EureqaSearch] = { 
      Host -> "localhost", 
      VariableLabels -> Automatic, 
      StepMonitor -> (Null &), 
      TerminateCondition -> (False &), 
      BuildingBlocks ->  DefaultBuildingBlocks, 
      SolutionPopulationSize -> Automatic, 
      FitnessMetric -> Automatic, 
      NormalizeFitnessBy -> Automatic, 
      SolutionPopulationSize -> Automatic, 
      PredictorPopulationSize -> Automatic, 
      TrainerPopulationSize -> Automatic, 
      SolutionCrossoverProbability -> Automatic, 
      SolutionMutationProbability -> Automatic, 
      PredictorCrossoverProbability -> Automatic, 
      PredictorMutationProbability -> Automatic, 
      MaxGenerations -> Infinity, 
      UpdatesPerSecond -> 1,
      DisplaySearchProgress -> SearchProgressGrid,
      DisplaySolutionFrontier -> SolutionFrontierGrid
      };

    EureqaSearch[data_?MatrixQ, searchRelationship_String, 
      opts : OptionsPattern[]] := 
     Module[{host = OptionValue[Host], frontier, frontierGrid = "", status = "", 
       progress = {}, generations, progressGrid = "", loop = True,
       maxGenerations = OptionValue[MaxGenerations]},
      CellGroup[{
        CellPrint[
         TextCell["Abort Evaluation to stop search.", "Output"]],
        CellPrint[ExpressionCell[Dynamic[frontierGrid], "Output"]],
        CellPrint[ExpressionCell[Dynamic[progressGrid], "Output"]],
        CellPrint[TextCell[Dynamic[status], "Output"]]
        }];
      status = "Connecting to '" <> host <> "'...";
      Check[ConnectTo[host], Return[]];
      status = "Sending data set...";
      Check[SendDataSet[data, OptionValue[VariableLabels]], Return[]];
      status = "Sending options...";
      Check[SendOptions[SearchRelationship -> searchRelationship, 
        Apply[Sequence, FilterRules[{opts}, SendOptionsOptions]]], Return[]];
      status = "Starting search...";
      Check[StartSearch[], Return[]];
      status = "Searching...";
      ClearSolutionFrontier[];
      Check[CheckAbort[
      While[IsConnected[] && loop,
        OptionValue[StepMonitor][];
        Check[progress = QueryProgress[], 
        status = "Error querying the server for progress."; Break[]];
        progressGrid = OptionValue[DisplaySearchProgress][progress];
        generations = GetField[progress, Generations];
        If[generations > maxGenerations,
         status = 
          "Stopped search: generation " <> ToString[generations] <> 
           " is greater than the max generation " <> 
           ToString[maxGenerations] <> " specified."; Break[]];
        frontier = AddToSolutionFrontier[progress];
        frontierGrid = OptionValue[DisplaySolutionFrontier][frontier];
        If[OptionValue[TerminateCondition][progress, frontier], 
         status = "Search stopped by terminate condition.";
         Break[]];
        Pause[1/OptionValue[UpdatesPerSecond]];
        ], 
       status = "Search stopped."; loop = False;],
       (* error *)
       status = "An error halted the search.";
       loop = False;
       ];
      EndSearch[];
      Disconnect[]];

    End[];

    Apply[Protect, eureqaSymbols];
    Remove[eureqaSymbols];
    Remove["symbolGroups"];
    EndPackage[];
