#include "session.h"
#include <math.h>
#include <string.h>

void session_init(PalaverSession *s, const Topic *topic, double threshold, double step_size) {
    memset(s, 0, sizeof(*s));
    s->topic = *topic;
    s->convergence_threshold = threshold;
    s->step_size = step_size;
}

int session_add_participant(PalaverSession *s, const Participant *p) {
    if (s->num_participants >= SESSION_MAX_PARTICIPANTS) return -1;
    s->participants[s->num_participants] = *p;
    return s->num_participants++;
}

int session_add_proposal(PalaverSession *s, const double *position) {
    if (s->num_proposals >= SESSION_MAX_PARTICIPANTS) return -1;
    Proposal *pr = &s->proposals[s->num_proposals];
    memcpy(pr->position, position, sizeof(double) * s->topic.dimensions);
    pr->votes = 0;
    pr->id = s->num_proposals;
    return s->num_proposals++;
}

int session_vote(PalaverSession *s, int participant_idx, int proposal_idx) {
    if (participant_idx < 0 || participant_idx >= s->num_participants) return -1;
    if (proposal_idx < 0 || proposal_idx >= s->num_proposals) return -1;
    s->proposals[proposal_idx].votes++;
    return 0;
}

ConsensusResult compute_consensus(PalaverSession *s) {
    ConsensusResult result;
    memset(&result, 0, sizeof(result));
    result.confidence = 0.0;

    if (s->num_participants == 0) return result;

    int dims = s->topic.dimensions;

    for (int round = 0; round < SESSION_MAX_ROUNDS; round++) {
        s->current_round = round + 1;

        /* compute weighted center */
        double center[PALAVER_MAX_DIMS];
        compute_center(s->participants, s->num_participants, center);

        /* measure current spread */
        double spread = consensus_distance(s->participants, s->num_participants);

        if (spread <= s->convergence_threshold) {
            memcpy(result.position, center, sizeof(double) * dims);
            result.rounds = round + 1;
            /* confidence = 1 - (spread / initial_spread) approx; if converged, high confidence */
            result.confidence = (spread < 1e-12) ? 1.0 : 1.0 - (spread / (spread + 1.0));
            return result;
        }

        /* move each participant toward center by step_size */
        for (int i = 0; i < s->num_participants; i++) {
            for (int d = 0; d < dims; d++) {
                double diff = center[d] - s->participants[i].position[d];
                s->participants[i].position[d] += diff * s->step_size;
            }
        }
    }

    /* didn't fully converge */
    double center[PALAVER_MAX_DIMS];
    compute_center(s->participants, s->num_participants, center);
    memcpy(result.position, center, sizeof(double) * dims);
    result.rounds = SESSION_MAX_ROUNDS;
    double spread = consensus_distance(s->participants, s->num_participants);
    result.confidence = 1.0 - (spread / (spread + 1.0));
    return result;
}
