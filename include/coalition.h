#ifndef COALITION_H
#define COALITION_H

#include "palaver.h"

#define COALITION_MAX_GROUPS 32
#define COALITION_MAX_MEMBERS 64

typedef struct {
    int member_ids[COALITION_MAX_MEMBERS];
    int num_members;
    double center[PALAVER_MAX_DIMS];
    double strength;
} Coalition;

typedef struct {
    Coalition groups[COALITION_MAX_GROUPS];
    int num_groups;
} CoalitionResult;

/* Internal: find participant by id in array. Returns index or -1. */
int find_participant_idx(const Participant *participants, int n, int id);

/* Find coalition groups: participants within threshold distance of each other. */
CoalitionResult find_coalitions(const Participant *participants, int n, double threshold);

/* Strength of a coalition: avg pairwise closeness * avg influence.
   Needs the full participant array to look up positions by id. */
double coalition_strength_ex(const Participant *participants, int np,
                             const int *ids, int n_ids);

/* Merge coalitions whose centers are within merge_threshold. Returns new result. */
CoalitionResult merge_coalitions(const Participant *participants, int np,
                                 const CoalitionResult *cr, double merge_threshold);

#endif /* COALITION_H */
