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
*)

let keys = Map.toList >> List.unzip >> fst

let expandRange (qn : QN.node list) v r =
    let x = qn |> List.find (fun n -> n.var = v)
    let min, max = x.range
    let r = if List.min r <> min then (List.min r - 1) :: r else r
    let r = if List.max r <> max then r @ [List.max r + 1] else r
    r

let remove x range = List.filter ((<>) x) range

// HERE:
// 3 remaining things.. not sure what the 2 range parameters are for. not sure what step size should really be. need to change the rangeFrom to have singleton for x
// step = 1 is correct.
// 1 remaining thing. the 2 range parameters

let checkTransition qn _ rangeFrom rangeTo =
    let isSubset range1 range2 = Set.isSubset (Set.ofList range1) (Set.ofList range2)
    let reachable var range = isSubset range (Map.find var rangeTo)
      
    let newRange = stepZ3rangelist.find_paths qn 1 rangeList rangeFrom // not sure what these to range parameters are for..
    newRange |> Map.forall reachable // pull this out to a function

// //            step <- step + 1  in find_paths....
// stepZ3rangelist.find_paths : qn -> int -> Map<QN.var, int list> -> Map<QN.var, int list> -> Map<QN.var, int list>
// why two range parameters?

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
                let thingyRanges = Map.add var [x] newRanges // temp name
                if not (checkTransition qn x (failwith "unimplemented") thingyRanges) then // 
                    newRanges <- Map.add var (remove x range) newRanges

    if expandedRanges = initialRanges then [initialRanges] // or []?
    else initialRanges :: increasingReachability qn expandedRanges // or expandedRanges :: ?


// move this to Game.fs
// let runIncreasingReachability qn initialRanges =
//     increasingReachability qn initialRanges |> Attractors.collapseRanges // not sure if this is right..


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
