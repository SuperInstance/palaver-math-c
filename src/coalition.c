#include "coalition.h"
#include <math.h>
#include <string.h>

int find_participant_idx(const Participant *participants, int n, int id) {
    for (int i = 0; i < n; i++) {
        if (participants[i].id == id) return i;
    }
    return -1;
}

double coalition_strength_ex(const Participant *participants, int np,
                             const int *ids, int n_ids) {
    if (n_ids <= 0) return 0.0;
    if (n_ids == 1) return 1.0;

    double total_dist = 0.0;
    double total_inf = 0.0;
    int dist_count = 0;

    for (int m = 0; m < n_ids; m++) {
        int idx = find_participant_idx(participants, np, ids[m]);
        if (idx < 0) continue;
        total_inf += participants[idx].influence;
    }
    double avg_inf = total_inf / n_ids;

    for (int i = 0; i < n_ids; i++) {
        int ai = find_participant_idx(participants, np, ids[i]);
        if (ai < 0) continue;
        for (int j = i + 1; j < n_ids; j++) {
            int aj = find_participant_idx(participants, np, ids[j]);
            if (aj < 0) continue;
            total_dist += euclidean_distance(participants[ai].position,
                                             participants[aj].position,
                                             participants[ai].dims);
            dist_count++;
        }
    }

    if (dist_count == 0) return avg_inf;
    double avg_dist = total_dist / dist_count;
    /* strength: high influence + low distance = high strength */
    return avg_inf / (1.0 + avg_dist);
}

CoalitionResult find_coalitions(const Participant *participants, int n, double threshold) {
    CoalitionResult result;
    memset(&result, 0, sizeof(result));
    if (n == 0) return result;

    int parent[COALITION_MAX_MEMBERS];
    for (int i = 0; i < n && i < COALITION_MAX_MEMBERS; i++) parent[i] = i;

    /* Union-Find: compress */
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double dist = euclidean_distance(participants[i].position,
                                             participants[j].position,
                                             participants[i].dims);
            if (dist <= threshold) {
                /* find roots */
                int ri = i, rj = j;
                while (parent[ri] != ri) { parent[ri] = parent[parent[ri]]; ri = parent[ri]; }
                while (parent[rj] != rj) { parent[rj] = parent[parent[rj]]; rj = parent[rj]; }
                if (ri != rj) parent[ri] = rj;
            }
        }
    }

    /* Compress all paths */
    for (int i = 0; i < n; i++) {
        int root = i;
        while (parent[root] != root) root = parent[root];
        parent[i] = root;
    }

    /* Map roots to group indices */
    int root_map[COALITION_MAX_MEMBERS];
    int num_roots = 0;
    memset(root_map, -1, sizeof(root_map));

    for (int i = 0; i < n; i++) {
        int root = parent[i];
        int found = -1;
        for (int k = 0; k < num_roots; k++) {
            if (root_map[k] == root) { found = k; break; }
        }
        if (found == -1) {
            found = num_roots;
            root_map[num_roots++] = root;
            memset(&result.groups[found], 0, sizeof(Coalition));
        }
        result.groups[found].member_ids[result.groups[found].num_members++] = participants[i].id;
    }
    result.num_groups = num_roots;

    /* Compute center and strength for each group */
    for (int g = 0; g < result.num_groups; g++) {
        Coalition *c = &result.groups[g];
        int dims = participants[0].dims;
        double total_inf = 0.0;
        for (int d = 0; d < dims; d++) c->center[d] = 0.0;

        for (int m = 0; m < c->num_members; m++) {
            int idx = find_participant_idx(participants, n, c->member_ids[m]);
            if (idx < 0) continue;
            double inf = participants[idx].influence;
            total_inf += inf;
            for (int d = 0; d < dims; d++) {
                c->center[d] += participants[idx].position[d] * inf;
            }
        }
        if (total_inf > 0.0) {
            for (int d = 0; d < dims; d++) c->center[d] /= total_inf;
        }
        c->strength = coalition_strength_ex(participants, n, c->member_ids, c->num_members);
    }

    return result;
}

CoalitionResult merge_coalitions(const Participant *participants, int np,
                                 const CoalitionResult *cr, double merge_threshold) {
    CoalitionResult merged;
    merged.num_groups = 0;

    if (cr->num_groups == 0) return merged;

    /* Copy initial groups */
    for (int i = 0; i < cr->num_groups; i++) {
        merged.groups[i] = cr->groups[i];
    }
    merged.num_groups = cr->num_groups;

    /* Greedy merge pass */
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < merged.num_groups && !changed; i++) {
            for (int j = i + 1; j < merged.num_groups && !changed; j++) {
                int dims = participants[0].dims;
                double dist = euclidean_distance(merged.groups[i].center,
                                                 merged.groups[j].center, dims);
                if (dist <= merge_threshold) {
                    /* Merge j into i */
                    Coalition *gi = &merged.groups[i];
                    Coalition *gj = &merged.groups[j];
                    for (int m = 0; m < gj->num_members && gi->num_members < COALITION_MAX_MEMBERS; m++) {
                        gi->member_ids[gi->num_members++] = gj->member_ids[m];
                    }
                    /* Recompute center */
                    double total_inf = 0.0;
                    for (int d = 0; d < dims; d++) gi->center[d] = 0.0;
                    for (int m = 0; m < gi->num_members; m++) {
                        int idx = find_participant_idx(participants, np, gi->member_ids[m]);
                        if (idx < 0) continue;
                        double inf = participants[idx].influence;
                        total_inf += inf;
                        for (int d = 0; d < dims; d++)
                            gi->center[d] += participants[idx].position[d] * inf;
                    }
                    if (total_inf > 0.0)
                        for (int d = 0; d < dims; d++) gi->center[d] /= total_inf;

                    gi->strength = coalition_strength_ex(participants, np,
                                                         gi->member_ids, gi->num_members);

                    /* Remove j by shifting */
                    for (int k = j; k < merged.num_groups - 1; k++) {
                        merged.groups[k] = merged.groups[k + 1];
                    }
                    merged.num_groups--;
                    changed = 1;
                }
            }
        }
    }

    return merged;
}
