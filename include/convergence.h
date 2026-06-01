#ifndef CONVERGENCE_H
#define CONVERGENCE_H

#include "palaver.h"
#include "session.h"

typedef struct {
    double spread;       /* consensus_distance at that round */
    double center[PALAVER_MAX_DIMS];
} RoundSnapshot;

typedef struct {
    RoundSnapshot rounds[SESSION_MAX_ROUNDS];
    int num_rounds;
} SessionHistory;

/* Rate of convergence: negative slope of spread over rounds. */
double convergence_rate(const SessionHistory *history);

/* Predict where consensus will land after max_rounds, using exponential decay fit. */
ConsensusResult predict_consensus(const PalaverSession *session, int max_rounds);

#endif /* CONVERGENCE_H */
