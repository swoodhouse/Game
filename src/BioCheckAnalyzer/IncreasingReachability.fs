// Copyright (c) Microsoft Research 2018
// License: MIT. See LICENSE

module IncreasingReachability

// open Paths // not sure if we can directly use this
// open stepZ3rangelist

// do backwards reachability until convergence to all possible states.. at the level of ranges.. return the seq
let increasingReachability (qn : QN.node list) initialBounds =
    // seems difficult to implement
    // 

    failwith "unimplemented"
// what is the sig for this.. do i do for each submodel or the mega model, and do i do from states or ranges. i guess from a range. then i need to intersect
// can i call find_paths or do i need to reimplement something like that


//let runReachability qn initialRanges =
//    printfn "Calling CAV reachability algorithm to narrow ranges..."
//    let paths = output_paths qn initialRanges |> List.rev
//    let stable = paths |> List.head |> values |> List.forall (fun l -> List.length l = 1)
//
//    if stable then
//        List.head paths, stable
//    else
//        let ranges = collapseRanges initialRanges paths    
//        ranges, stable

// Attractors.collapseRanges
//
//let output_paths (network : QN.node list) bounds =
//    let mutable paths = [bounds]
//    let mutable step = 0
//    let mutable pathLength = 0
//    let nubounds = ref bounds
//    let mutable con = true 
//    while con do
//        nubounds := stepZ3rangelist.find_paths network step !nubounds bounds
//        con <- List.forall (fun elem -> elem <> !nubounds) paths 
//        if con then
//            paths <- paths @ [!nubounds]
//            pathLength <- step
//            step <- step + 1 
//    paths
//
//// SI: rewritten output_paths to remove redundant code and to be more functional. 
//// decreasing reachability calculates the fixed point of reachability wrt bounds. 
//let decreasing_reachability (qn : QN.node list) bounds naive =
//    if naive then
//        [bounds]
//    else 
//        let rec loop step bounds' bounds paths =
//            let bounds'' = stepZ3rangelist.reachability qn step bounds' bounds
//            // If bounds'' is different from each path, then find next bound. 
//            if List.forall (fun p -> p <> bounds'') paths then 
//                // SI: should bounds be bounds'?
//                loop (step+1) bounds'' bounds (paths @ [bounds'']) 
//            else paths
//        loop 0 bounds bounds [bounds]
//
