(* init.m

  This contains the Mathematica code for the Eureqa Client.

  Written by Shane Celis

*)
BeginPackage["EureqaClient`"];

(* Let's handle our symbol uniformly.  Unprotect so we can modify them during
  (re)loading, and protect when closing.  *)

  Unprotect[SearchOptions];
    SearchOptions = {SearchRelationship,
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

    eureqaSymbols = Join[SearchOptions,
                {
                (* Tags or objects *)
                 SolutionFrontier, 
                 (* Tags for SolutionInfo *)
                 SolutionInfo, 
                  FormulaText, 
                  Fitness, 
                  Score, 
                  SearchOptions,
                  Complexity, 
                  Age, 
                  Expression,
                 (* Tags for SearchProgress *)
                 SearchProgress, 
                  Solution, 
                  Generations, 
                  GenerationsPerSec, 
                  Evaluations, 
                  EvaluationsPerSec, 
                  TotalPopulationSize, 
                (* Functions *)
                 ConnectTo, 
                 Disconnect, 
                 SendDataSet, 
                 StartSearch, 
                 PauseSearch, 
                 EndSearch, 
                 QueryProgress, 
                 QueryFrontier, 
                 FormulaTextToExpression, 
                 ClearSolutionFrontier, 
                 AddToSolutionFrontier,                  
                 AddToSolutionFrontierHelper, 
                 SolutionFrontierGrid,
                 GetSolutionFrontier, 
                 SearchProgressGrid, 
                 IsConnected,
                 (* Values *)
                 DefaultBuildingBlocks,
                 (* Arguments to SendOptions *)
                 SendOptions, 
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
                  Count,
                 EureqaSearch,
                 (* Arguments to EureqaSearch *)
                  Host,
                  VariableLabels,
                  MaxGenerations,
                  TerminateCondition,
                  UpdatesPerSecond
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
    QueryFrontier::err = "Error querying frontier.";
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
    (*AllBuildingBlocks = Where are these defined?*)

    Begin["EureqaClient`Private`"]; 
    SendDataSet[data_, Automatic] := SendDataSet[data];
    reload::usage = "Reloads the mathlink executable.";
    (*Set EureqaClient`Private`linkName = "XXX" so that you can run
    the mathlink executable in a debugger or see its output.  It will
    ask for a name at startup, give it your choice for "XXX".  *)
    If[Length[Names["EureqaClient`Private`linkName"]] === 0,  
        EureqaClient`Private`linkName = None];
    load[] := (*If[Length[Names["EureqaClient`Private`linkName"]] === 0 || linkName === None, *)
                  (* Never seen the linkName symbol, load the default link. *)
                  mathlink = Install[$UserBaseDirectory <> "/Applications/EureqaClient/eureqaml"](*, 
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
        fields = { Complexity, Fitness, Expression, Score};
        header = {Map[Style[SymbolName[#], Bold]&, fields]};
        If[Length[front] > 0,
            infos = front[[1]] /. SolutionInfo -> List;
            gridItems = Join[header, SortBy[Map[getAll[#, Apply[Sequence, fields]]&, infos], 
                                            -Part[#,4]&]],
            gridItems = {header}];
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
    Options[EureqaSearch] = { 
      Host -> "localhost", VariableLabels -> Automatic, StepMonitor -> (Null &), 
      TerminateCondition -> (False &), BuildingBlocks ->  DefaultBuildingBlocks, 
      SolutionPopulationSize -> Automatic, FitnessMetric -> Automatic, 
      NormalizeFitnessBy -> Automatic, SolutionPopulationSize -> Automatic, 
      PredictorPopulationSize -> Automatic, TrainerPopulationSize -> Automatic, 
      SolutionCrossoverProbability -> Automatic, SolutionMutationProbability -> Automatic, 
      PredictorCrossoverProbability -> Automatic, PredictorMutationProbability -> Automatic, 
      MaxGenerations -> Infinity, UpdatesPerSecond -> 1};

    EureqaSearch[data_?MatrixQ, searchRelationship_String, 
      opts : OptionsPattern[]] := 
     Module[{host = OptionValue[Host], result = "", status = "", progress = {}, 
       generations, progressGrid = "", 
       maxGenerations = OptionValue[MaxGenerations]},
      CellGroup[{
        CellPrint[
         TextCell["Abort Evaluation to stop search.", "Output"]],
        CellPrint[ExpressionCell[Framed[Dynamic[result]], "Output"]],
        CellPrint[ExpressionCell[Framed[Dynamic[progressGrid]], "Output"]],
        CellPrint[TextCell[Dynamic[status], "Output"]]
        }];
      status = "Connecting to '" <> host <> "'...";
      Check[ConnectTo[host], Return[]];
      status = "Sending data set...";
      Check[SendDataSet[data, OptionValue[VariableLabels]], Return[]];
      status = "Sending options...";
      Check[SendOptions[SearchRelationship -> searchRelationship, 
        Apply[Sequence, FilterRules[{opts}, SearchOptions]]], Return[]];
      status = "Starting search...";
      Check[StartSearch[], Return[]];
      status = "Searching...";
      ClearSolutionFrontier[];
      CheckAbort[While[IsConnected[],
        If[OptionValue[TerminateCondition][], 
         status = "Search stopped by terminate condition.";
         Break[]];
        OptionValue[StepMonitor][];
        progressGrid = SearchProgressGrid[progress = QueryProgress[]];
        If[(generations = Generations /. List @@ progress) > 
          maxGenerations, 
         status = 
          "Stopped search: generation " <> ToString[generations] <> 
           " is greater than the max generation " <> 
           ToString[maxGenerations] <> " specified."; Break[]];
        result = 
         SolutionFrontierGrid[AddToSolutionFrontier[QueryProgress[]]];
        Pause[1/OptionValue[UpdatesPerSecond]];
        ], 
       status = "Search stopped."];
      EndSearch[];
      Disconnect[]];

    End[];

    Apply[Protect, eureqaSymbols];
    Remove[eureqaSymbols];
    EndPackage[];
