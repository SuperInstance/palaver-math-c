# palaver-math-c

**Palaver** — West African consensus dialogue, rendered in C99 for edge and embedded devices.

A mathematical library for computing consensus among participants with positions, influence weights, and iterative convergence. Named after the West African tradition of communal dialogue to reach agreement.

## Features

- **Core math**: Weighted centroid computation, pairwise consensus distance, Euclidean distance
- **Sessions**: Iterative convergence with configurable step size and threshold
- **Coalitions**: Union-Find based grouping of nearby participants, with merge support
- **Convergence**: Rate analysis via linear regression, consensus prediction
- **Dialogue trees**: BFS-based shortest path to agreement nodes
- **Arena allocator**: Zero-dependency memory management suitable for constrained devices

## Building

```bash
make          # build libpalaver.a
make test     # build and run tests
make clean    # clean build artifacts
```

## API Overview

```c
#include "palaver_api.h"

// Initialize context with a topic, convergence threshold, and step size
PalaverContext ctx;
Topic topic = {.id = 1, .dimensions = 2, .ideal_point = {0, 0}};
palaver_init(&ctx, &topic, 0.01, 0.3);

// Add participants
double pos[] = {1.0, 2.0};
Participant p = make_participant(1, 2, pos, 1.0);
session_add_participant(&ctx.session, &p);

// Run consensus
ConsensusResult result = palaver_run(&ctx);
// result.position, result.confidence, result.rounds
```

## Module Summary

| Header | Description |
|---|---|
| `palaver.h` | Core types (Participant, Topic) and math functions |
| `session.h` | Iterative convergence sessions |
| `coalition.h` | Coalition finding and merging |
| `convergence.h` | Rate analysis and prediction |
| `dialogue.h` | Dialogue tree construction and BFS consensus path |
| `palaver_api.h` | Unified convenience API |
| `arena.h` | Arena allocator |

## Requirements

- C99 compiler (gcc, clang, etc.)
- No external dependencies
- libm (standard math library)

## License

MIT
