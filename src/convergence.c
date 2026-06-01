#include "convergence.h"
#include <math.h>
#include <string.h>

double convergence_rate(const SessionHistory *history) {
    if (history->num_rounds < 2) return 0.0;

    /* Linear regression on round index vs spread */
    int n = history->num_rounds;
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
    for (int i = 0; i < n; i++) {
        double x = (double)i;
        double y = history->rounds[i].spread;
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_xx += x * x;
    }
    double denom = n * sum_xx - sum_x * sum_x;
    if (fabs(denom) < 1e-15) return 0.0;
    double slope = (n * sum_xy - sum_x * sum_y) / denom;
    return -slope; /* positive rate means convergence */
}

ConsensusResult predict_consensus(const PalaverSession *session, int max_rounds) {
    ConsensusResult result;
    memset(&result, 0, sizeof(result));

    if (session->num_participants == 0) return result;

    /* Simulate convergence */
    int dims = session->topic.dimensions;
    int np = session->num_participants;

    /* Work on copies */
    Participant copies[SESSION_MAX_PARTICIPANTS];
    memcpy(copies, session->participants, sizeof(Participant) * np);

    /* Record history for rate estimation */
    SessionHistory hist;
    hist.num_rounds = 0;

    for (int round = 0; round < max_rounds && round < SESSION_MAX_ROUNDS; round++) {
        double center[PALAVER_MAX_DIMS];
        compute_center(copies, np, center);
        double spread = consensus_distance(copies, np);

        hist.rounds[hist.num_rounds].spread = spread;
        memcpy(hist.rounds[hist.num_rounds].center, center, sizeof(double) * dims);
        hist.num_rounds++;

        if (spread <= session->convergence_threshold) {
            memcpy(result.position, center, sizeof(double) * dims);
            result.rounds = round + 1;
            result.confidence = 1.0 - (spread / (spread + 1.0));
            return result;
        }

        /* Move toward center */
        for (int i = 0; i < np; i++) {
            for (int d = 0; d < dims; d++) {
                double diff = center[d] - copies[i].position[d];
                copies[i].position[d] += diff * session->step_size;
            }
        }
    }

    /* Return best estimate */
    double center[PALAVER_MAX_DIMS];
    compute_center(copies, np, center);
    memcpy(result.position, center, sizeof(double) * dims);
    result.rounds = max_rounds;
    double spread = consensus_distance(copies, np);
    result.confidence = 1.0 - (spread / (spread + 1.0));
    return result;
}
