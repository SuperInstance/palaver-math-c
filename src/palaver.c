#include "palaver.h"
#include <math.h>

void compute_center(const Participant *participants, int n, double *out_center) {
    if (n <= 0) return;
    int dims = participants[0].dims;
    double total_influence = 0.0;

    for (int d = 0; d < dims; d++) {
        out_center[d] = 0.0;
    }

    for (int i = 0; i < n; i++) {
        double inf = participants[i].influence;
        total_influence += inf;
        for (int d = 0; d < dims; d++) {
            out_center[d] += participants[i].position[d] * inf;
        }
    }

    if (total_influence > 0.0) {
        for (int d = 0; d < dims; d++) {
            out_center[d] /= total_influence;
        }
    }
}

double euclidean_distance(const double *a, const double *b, int dims) {
    double sum = 0.0;
    for (int i = 0; i < dims; i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

double consensus_distance(const Participant *participants, int n) {
    if (n <= 1) return 0.0;
    double total = 0.0;
    int count = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            int dims = participants[i].dims;
            total += euclidean_distance(participants[i].position,
                                        participants[j].position, dims);
            count++;
        }
    }
    return total / count;
}
