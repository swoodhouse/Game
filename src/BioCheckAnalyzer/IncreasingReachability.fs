// Copyright (c) Microsoft Research 2018
// License: MIT. See LICENSE

module IncreasingReachability

(*
initial ranges, per sub-model. or attractor per sub-model [per unmutated sub-model]

1. -1 +1 each side of range of each var
2. For each possible concrete val of each var, and each abstract range of other vars, can i rule out some element of range in successor.
   do as a list not min/max bound. order of choice of variable does not matter. left to right
3. stopping condition is that ranges stop changing
4. one level back. global stopping condition is that ranges converge
if SAT then ...

not so straightforward to implement this...

*)

let keys = Map.toList >> List.unzip >> fst

let expandRange (qn : QN.node list) v r =
    let x = qn |> List.find (fun n -> n.var = v)
    let min, max = x.range
    let r = if List.min r <> min then (List.min r - 1) :: r else r
    let r = if List.max r <> max then r @ [List.max r + 1] else r
    r
    
let checkTransition _ _ _ = // temp
    failwith "unimplemented"

let remove x range = List.filter ((<>) x) range

// write imperative to begin with. i think a mix of loop and recursion would work..
// need a z3 context somewhere too..
// check with nir that this seems correct
let rec increasingReachability (qn : QN.node list) (initialRanges : Map<QN.var, int list>) =
  let mutable expandedRanges = Map.empty
  let mutable newRanges = Map.map (expandRange qn) initialRanges

  // wait.. do i really need a loop and recursion.. yeah.. because next round you re-expand.. except expand is wrong if i do like this, with potential gaps in the range.. right?
  // while loop. expandedNew != expanded // can we do with a generator expression instead?
  while expandedRanges <> newRanges do
    expandedRanges <- newRanges
    for var in keys newRanges do
      let range = Map.find var newRanges
      for x in range do
        if not (checkTransition qn x newRanges) then
          newRanges <- Map.add var (remove x range) newRanges

  if expandedRanges = initialRanges then [initialRanges] // or []?
  else initialRanges :: increasingReachability qn expandedRanges // or expandedRanges :: ?


// move this to Game.fs
// let runIncreasingReachability qn initialRanges =
//     increasingReachability qn initialRanges |> Attractors.collapseRanges // not sure if this is right..
  
// stepZ3rangelist.find_paths : qn -> int -> Map<QN.var, int list> -> Map<QN.var, int list> -> Map<QN.var, int list>
// why two range parameters?

// assert_query : qn.node -> value:int -> time:int -> z -> s -> c
// assert_target_function : ...
// BioCheckZ3.assert_bound : qn.node -> lower:int -> upper:int -> time:int -> z -> s -> c

// open Paths // not sure if we can directly use this
// open stepZ3rangelist

// do backwards reachability until convergence to all possible states.. at the level of ranges.. return the seq
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
