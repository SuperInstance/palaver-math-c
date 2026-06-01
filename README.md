# palaver-math-c

> Dialogue and consensus mathematics in C — iterative convergence, coalitions, sessions, and dialogue trees.

## What This Does

`palaver-math-c` implements West African palaver-inspired consensus mathematics in C. Participants hold position vectors, vote on proposals, and converge toward agreement through iterative position updates. It provides consensus distance, convergence rate prediction, coalition detection, session management, and dialogue tree construction. Use it for multi-agent consensus in embedded systems, robotics, or IoT.

## The Cultural Root

See `palaver-math` (PyPI) for the full cultural background. A palaver is community dialogue — iteration until consensus.

## Install

```bash
git clone https://github.com/SuperInstance/palaver-math-c.git
cd palaver-math-c
make
```

## Quick Start

```c
#include "palaver_api.h"

int main() {
    Topic topic = {.name = "policy", .dimensions = 2};

    PalaverContext ctx;
    palaver_init(&ctx, &topic, /*threshold=*/0.5, /*step_size=*/0.1);

    Participant p1 = {.id = 0, .position = {1.0, 0.0}, .influence = 1.0};
    Participant p2 = {.id = 1, .position = {0.0, 1.0}, .influence = 1.0};
    Participant p3 = {.id = 2, .position = {0.5, 0.5}, .influence = 1.0};

    palaver_add_participant(&ctx, &p1);
    palaver_add_participant(&ctx, &p2);
    palaver_add_participant(&ctx, &p3);

    // Run consensus
    ConsensusResult result = palaver_compute_consensus(&ctx);
    printf("Consensus: %s, rounds: %d\n",
           result.consensus_reached ? "yes" : "no", result.rounds);

    // Convergence rate
    double rate = palaver_convergence_rate(&ctx);
    printf("Convergence rate: %.4f\n", rate);

    // Dialogue tree
    DialogueTree tree;
    dialogue_tree_init(&tree);
    dialogue_add_statement(&tree, "Build the road", -1);
    dialogue_add_statement(&tree, "What about the river?", 0);
    dialogue_add_statement(&tree, "Build a bridge", 1);

    int path[16];
    int path_len;
    find_consensus_path(&tree, path, &path_len);

    return 0;
}
```

## API Reference

### Core Consensus (`palaver.h`)
- `void compute_center(const Participant *p, int n, double *out_center)`
- `double consensus_distance(const Participant *p, int n)`
- `double euclidean_distance(const double *a, const double *b, int dims)`
- `void influence_weighted_center(const Participant *p, int n, const double *weights, double *out)`

### Session (`session.h`)
- `void session_init(PalaverSession *s, const Topic *topic, double threshold, double step_size)`
- `int session_add_participant(PalaverSession *s, const Participant *p)`
- `int session_add_proposal(PalaverSession *s, const double *position)`
- `int session_vote(PalaverSession *s, int participant_idx, int proposal_idx)`
- `ConsensusResult session_compute_consensus(PalaverSession *s)`

### Convergence (`convergence.h`)
- `double convergence_rate(const SessionHistory *history)`
- `int is_converging(const SessionHistory *history)`

### Coalition (`coalition.h`)
- `int find_coalitions(const Participant *p, int n, double threshold, int *out_groups, int *out_count)`
- `double coalition_strength_ex(const Participant *p, int np, const int *indices, int ni)`

### Dialogue (`dialogue.h`)
- `void dialogue_tree_init(DialogueTree *tree)`
- `int dialogue_add_statement(DialogueTree *tree, const char *text, int parent_id)`
- `int find_consensus_path(DialogueTree *tree, int *path_out, int *path_len)`

### Unified API (`palaver_api.h`)
- `void palaver_init(PalaverContext *ctx, const Topic *topic, double threshold, double step_size)`
- `void palaver_add_participant(PalaverContext *ctx, const Participant *p)`
- `ConsensusResult palaver_compute_consensus(PalaverContext *ctx)`
- `double palaver_convergence_rate(PalaverContext *ctx)`

## License

MIT
