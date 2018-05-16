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

// finish isSubset, Game.fs, GameDLL

let keys = Map.toList >> List.unzip >> fst

let remove x = List.filter ((<>) x)

let isSubset range1 range2 = Set.isSubset (Set.ofList range1) (Set.ofList range2)

let expandRange (qn : QN.node list) v r =
    let x = qn |> List.find (fun n -> n.var = v)
    let min, max = x.range
    let r = if List.min r <> min then (List.min r - 1) :: r else r
    let r = if List.max r <> max then r @ [List.max r + 1] else r
    r

let checkTransition qn fullRanges rangeFrom rangeTo =
    let reachable var range = isSubset range (Map.find var rangeTo) // is this correct?
    // in the case of rangeTo being a fixpoint... then you want to rule out x if the fixpoint is not contained in newRange   
    let newRange = stepZ3rangelist.find_paths qn 1 rangeFrom fullRanges
    newRange |> Map.forall reachable

let rec increasingReachability qn initialRanges =
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

    if expandedRanges = initialRanges then Seq.singleton initialRanges
    else Seq.append (Seq.singleton initialRanges) (increasingReachability qn expandedRanges)

let runIncreasingReachability qn bottomRanges topRanges =
    // lazily run backwards reachability from bottom ranges, return first range that contains topRanges
    let ranges = increasingReachability qn bottomRanges
    Seq.find (fun r -> isSubset (Attractors.values r) (Attractors.values topRanges)) ranges
