﻿module Game

open System.Runtime.InteropServices
open FSharp.NativeInterop
open IncreasingReachability
open BioCheckPlusZ3

// 1. modify GameDLL project with the below commented out code - convert to 0/1 ADDs. This is step one, then run with the attractors entry point and ensure same as AttractorsDLL. Then, Game.h/cpp
//     I think we can treat Attactors.h/cpp as done, and write Game.h/cpp
//     I guess Game.h now needs two entry points too? One to generate attractors, one to run minimax/valueiteration
//     implement the attractors entry point to begin with brute force rather than mega model, and test that. computeAttractorsExplict()
//     after that you could try scoring them - attractor.Add() * scoreRelation
//     I should check how efficent it is to call Add() and convert BDDs to ADDs versus building them as 0/1 ADDs in the first place. First is cleaner, second may be more efficent, but maybe not

// 2. compute all attractors, check reproduces order/gives same results as pure BDDs. Benchmark running once on megamodel vs per each submodel
// 3. implement increasingReachability, going backwards from loaded attractors
// 4. implement minimax - scores need to be shifted by +1 (0 represents unreached states), paper notes
// the below dll signatures need to change to support mega models, i.e many sub-tables
// also need to add corresponding entry points to GameDLL.cpp
// add new Main entry point that calls playGame
// implement valueIteration

[<DllImport("Game.dll", CallingConvention=CallingConvention.Cdecl)>]
extern int attractors(int numVars, int[] ranges, int[] numInputs, int[] inputVars, int[] numUpdates, int[] inputValues, int[] outputValues,
                      string proofOutput, int proofOutputLength, string csvHeader, int headerLength)

[<DllImport("Game.dll", CallingConvention=CallingConvention.Cdecl)>]
extern int minimax(int numVars, int[] ranges, int[] minValues, int[] numInputs, int[] inputVars, int[] numUpdates, int[] inputValues, int[] outputValues,
                   int numKoVars, int[] koVars, int numOeVars, int[] oeVars) // need who goes last, and depth/height

[<DllImport("Game.dll", CallingConvention=CallingConvention.Cdecl)>]
extern int valueIteration(int numVars, int[] ranges, int[] minValues, int[] numInputs, int[] inputVars, int[] numUpdates, int[] inputValues, int[] outputValues,
                          int numKoVars, int[] koVars, int numOeVars, int[] oeVars)

// stolen from stackoverflow
// sub-lists are in the reverse order to c++ here..
let rec combinations acc size set = seq {
  match size, set with
  | n, x::xs ->
      if n > 0 then yield! combinations (x::acc) (n - 1) xs
      if n >= 0 then yield! combinations acc n xs
  | 0, [] -> yield acc
  | _, [] -> () }

//combinations [] 3 [1 .. 4]

// i think maybe combinations then cross product
// no, just combinations.. [for x in combinations [] 2 [1; 2; 3] do for y in combinations [] 2 [-1;-2;-3;-4] do yield x, y] |> List.length;;
// so now i need combinations in c++. and in same order

//// stolen from rosetta code. important to ensure c++ ordering and f# ordering match
//// ok, cP2 is ordered nicely. the c++ should be rewritten to be only for pairs, too
//let crossProduct a b = List.map (fun (a,b)->[a;b]) (List.allPairs a b)
//  Microsoft.FSharp.Primitives.Basics.List.allPairs
// SW: currently has duplicated code from BioCheckPlusZ3.fs

let rec allPairs a b =
    match a, b with
    | [], _ | _, [] -> []
    | h1::t1, h2::t2 -> (h1,h2)::(allPairs [h1] t2)@(allPairs t1 b)

let crossProduct a b = List.map (fun (a,b)->[a;b]) (allPairs a b)

let generateQNTable' (qn:QN.node list) (ranges : Map<QN.var,int list>) (node : QN.node) =
    let inputnodes =
        node :: List.concat
                     [ for var in node.inputs do
                           yield (List.filter (fun (x:QN.node) -> ((x.var = var) && not (x.var = node.var))) qn) ]
    let list_of_ranges = List.fold (fun acc (node : QN.node) -> (Map.find node.var ranges)::acc) [] inputnodes

    let list_of_possible_combinations = 
        List.rev list_of_ranges |> Seq.fold (fun acc xs -> [for x in xs do for ac in acc -> List.rev(x::(List.rev ac))]) [[]]  
    let create_map_from_var_to_values list_of_nodes (list_of_values : int list) = 
        let node_to_var (node : QN.node) = node.var
        let convert_node_list_to_var_list node_list = List.map node_to_var node_list
        List.zip (convert_node_list_to_var_list list_of_nodes) list_of_values |> Map.ofList
    let list_of_targets = 
        List.map (fun elem -> expr_to_real qn node node.f (create_map_from_var_to_values inputnodes elem)) list_of_possible_combinations 
    let list_of_int_targets = 
        List.map my_round list_of_targets    
    let list_of_actual_next_vals =
        let compute_actual_next_val target_val inputs_vals =
            let range = Map.find node.var ranges
            let min, max = List.head range, List.rev range |> List.head
            apply_target_function (List.head inputs_vals) target_val min max

        List.map2 compute_actual_next_val list_of_int_targets list_of_possible_combinations

    list_of_possible_combinations, list_of_actual_next_vals

let playGame (*mode*) proof_output qn (mutations : (QN.var * int) list) (treatments : (QN.var * int) list) height =
    // build a table per submodel. lots shared between these so maybe don't duplicate that work
    // call DLL minimax/back()
    // split this up in sub-functions

    let qn = qn |> List.sortBy (fun (n : QN.node) -> n.var) // important to sort to match ranges map ordering
    
    // call vmcai on each sub-model
    //printfn "Building QN tables to find attractors..."
    //let conditions = crossProduct mutations treatments
    //let inputValues, outputValues =
    //    [| for c in conditions do
    //           let submodel = List.fold (fun current_qn (var,c) -> QN.ko current_qn var c) qn c
    //           let ranges, _ = Attractors.runVMCAI qn
    //           yield qn |> List.map (generateQNTable' qn ranges) |> List.unzip |] |> Array.unzip

    //let inputValues' = inputValues |> Array.map (List.concat >> List.concat >> Array.ofList) |> Array.concat
    //let outputValues' = outputValues |> Array.map (List.concat >> Array.ofList) |> Array.concat

    //// call DLL attractors. Test: reproduces ORDER? Compare efficency of calling once for each submodel vs once total using megamodel
    //printfn "Calling DLL to find attractors..."
    //let ranges = failwith "unimplemented" // let ranges' = Map.toArray ranges |> Array.map (fun (_, x) -> List.length x - 1) means we don't allow vars to go from non-zero now.?.
    //                                      // i think what you need to do is have a minValue still, but have it be a global minValue, not vmcai based
    printfn "Building QN tables to find attractors..."
    // NOT CROSS PRODUCT BUT ... as a hack could do cross product and filter for under a certain length..
    let conditions = crossProduct mutations treatments // [for x in combinations [] 2 [1; 2; 3] do for y in combinations [] 2 [-1;-2;-3;-4] do yield x, y];; for height..
    let inputValues, outputValues =
        [| for c in conditions do
               let submodel = List.fold (fun current_qn (var,c) -> QN.ko current_qn var c) qn c
               let ranges, _ = Attractors.runVMCAI qn
               yield qn |> List.map (generateQNTable' qn ranges) |> List.unzip |] |> Array.unzip

    let inputValues' = inputValues |> Array.map (List.concat >> List.concat >> Array.ofList) |> Array.concat
    let outputValues' = outputValues |> Array.map (List.concat >> Array.ofList) |> Array.concat

    // call DLL attractors. Test: reproduces ORDER? Compare efficency of calling once for each submodel vs once total using megamodel
    printfn "Calling DLL to find attractors..."
    let ranges = failwith "unimplemented" // let ranges' = Map.toArray ranges |> Array.map (fun (_, x) -> List.length x - 1) means we don't allow vars to go from non-zero now.?.
                                          // i think what you need to do is have a minValue still, but have it be a global minValue, not vmcai based
    let qnVars = qn |> List.map (fun n -> n.var)
    //let inputVars = qn |> List.map (fun n -> n.var :: List.filter (fun x -> not (x = n.var)) n.inputs)
    //                   |> List.map (List.map (fun x -> List.findIndex ((=) x) qnVars)) // convert BMA index to 0-based index
    //let numInputs = List.map List.length inputVars |> Array.ofList
    //let inputVars' = List.concat inputVars |> Array.ofList
    //let numUpdates = Array.map (List.map List.length >> Array.ofList) outputValues |> Array.concat
    //let variables = qn |> List.map (fun n -> n.name)
    //let header = List.reduce (fun x y -> x + "," + y) variables
    //attractors(List.length qn, ranges, numInputs, inputVars', numUpdates, inputValues', outputValues', proof_output, String.length proof_output, header, String.length header) |> ignore

    // load attractors from files, and call increasingReachability, per submodel(?), from only the relevant attractors (easy to select out those with a particular choice var after loading)
    // good point - its after unmutate() that i should do increasingReachability
    // already have code to load ranges from files. and to collapseranges
    // how do i do this based on unmutate?????????
    // well.. you are going to have to do many runs anyway. kind of horrible to explode here though. can i remove the mut vars, ....
    printfn "Building QN tables to play game..."
    let tables =
      [ for i in 0 .. height - 1 do
          // need to treat fixpoints and attractor files separately
          let topRanges = System.IO.Directory.GetFiles(proof_output, sprintf "Attractor_%i_*.csv" i) // does order matter? i.e. do we want to load 0 before 1?
                       |> Array.map (Attractors.loadRangesFromCsv qnVars)
          
          let bottomRanges = System.IO.Directory.GetFiles(proof_output, sprintf "Attractor_%i_*.csv" (i + 1)) // refactor out to a function
                          |> Array.map (Attractors.loadRangesFromCsv qnVars)

          let topRanges = Attractors.collapseRanges topRanges.[0] topRanges.[1..] // this is not ideal..
          let bottomRanges = Attractors.collapseRanges bottomRanges.[0] bottomRanges.[1..] // this is not ideal..
          // then you have the choice of collapsing them and running increasingreach, or.. running increasingreach on each separately and then collapsing all of the results.. implement both here and comment one out
          // can i use collapseRanges? why does it have a special initial parameter
        // well i've implemented one of the two possible ways......
        // well no i haven't.. i've done it per height. you can also do it per sub-model per height, and per attractor per sub-model per height
          let constrainedBounds = IncreasingReachability.runIncreasingReachability qn bottomRanges topRanges // this needs to be another for loop over List.zip bottomRanges topRanges
        // then generateQNTable'...

    // let inputValues, outputValues =
    //     [| for c in conditions do
        // would have to filter 
    //            let submodel = List.fold (fun current_qn (var,c) -> QN.ko current_qn var c) qn c
    //            let ranges, _ = Attractors.runVMCAI qn // here do increasing reach
    //            yield qn |> List.map (generateQNTable' qn ranges) |> List.unzip |] |> Array.unzip

        // to do it per submodel.. call cross product on the mutations.. also need to do unmutate
          () ]
    //printfn "Calling DLL to play game..."
    ()
    //minimax(List.length qn, ) |> ignore
