#ifndef PALAVER_H
#define PALAVER_H

#include <stddef.h>

#define PALAVER_MAX_DIMS 32

typedef struct {
    int id;
    double position[PALAVER_MAX_DIMS];
    double influence;
    int dims; /* number of active dimensions */
} Participant;

typedef struct {
    int id;
    int dimensions;
    double ideal_point[PALAVER_MAX_DIMS];
} Topic;

/* Compute centroid of all participants (weighted by influence). */
void compute_center(const Participant *participants, int n, double *out_center);

/* Average pairwise distance between participants — measures spread. */
double consensus_distance(const Participant *participants, int n);

/* Euclidean distance between two points of given dimensionality. */
double euclidean_distance(const double *a, const double *b, int dims);

#endif /* PALAVER_H */
