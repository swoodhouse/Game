// Copyright (c) Microsoft Research 2018
// License: MIT. See LICENSE

module IncreasingReachability

(*
initial ranges, per sub-model. or attractor per sub-model [per unmutated sub-model]

1. -1 +1 each side of range of each var
2. For each possible concrete val of each var, and each abstract range of other vars, can i rule out some element of range in successor.
   do as a list not min/max bound. order of choice of variable does not matter. left to right
3. stopping condition is that ranges stop changing
4. one level back. global stopping condition is that ranges converg

// then collapse everything
*)
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

let keys = Map.toList >> List.unzip >> fst

let expandRange (qn : QN.node list) v r =
    let x = qn |> List.find (fun n -> n.var = v)
    let min, max = x.range
    let r = if List.min r <> min then (List.min r - 1) :: r else r
    let r = if List.max r <> max then r @ [List.max r + 1] else r
    r

let remove x range = List.filter ((<>) x) range

let checkTransition qn fullRanges rangeFrom rangeTo =
    let isSubset range1 range2 = Set.isSubset (Set.ofList range1) (Set.ofList range2)
    let reachable var range = isSubset range (Map.find var rangeTo) // is this correct?
    // in the case of rangeTo being a fixpoint... then you want to rule out x if the fixpoint is not contained in newRange   
    let newRange = stepZ3rangelist.find_paths qn 1 rangeFrom fullRanges
    newRange |> Map.forall reachable

// check with nir that this seems correct
let rec increasingReachability (qn : QN.node list) (initialRanges : Map<QN.var, int list>) =
    let fullRanges = Rangelist.nuRangel qn
    let mutable expandedRanges = Map.empty
    let mutable newRanges = Map.map (expandRange qn) initialRanges

    while expandedRanges <> newRanges do
        expandedRanges <- newRanges
        for var in keys newRanges do
            let range = Map.find var newRanges
            for x in range do
                let singletonRange = Map.add var [x] newRanges
                if not (checkTransition qn fullRanges singletonRange initialRanges) then
                    newRanges <- Map.add var (remove x range) newRanges

    if expandedRanges = initialRanges then [initialRanges]
    else initialRanges :: increasingReachability qn expandedRanges
