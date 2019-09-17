#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

#include <emscripten.h>

typedef struct rgba {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} rgba_t;

static int euclidean_distance(const rgba_t *p1, const rgba_t *p2) {
    int dr = (p1->r - p2->r);
    int dg = (p1->g - p2->g);
    int db = (p1->b - p2->b);

    return dr*dr + dg*dg + db*db;
}

static void pick_centroid(rgba_t **objs, int *clusters, size_t num_objs, int cluster, rgba_t *centroid) {
    int num_cluster = 0;
    
    uint32_t r = 0, g = 0, b = 0;

    for (int i = 0; i < num_objs; i++) {
        if (clusters[i] != cluster) continue;

        r += objs[i]->r;
        g += objs[i]->g;
        b += objs[i]->b;
        num_cluster++;
    }
    if (num_cluster) {
        r /= num_cluster;
        g /= num_cluster;
        b /= num_cluster;
        
        centroid->r = r;
        centroid->g = g;
        centroid->b = b;
    }
    return;
}


EMSCRIPTEN_KEEPALIVE
int kmeans(rgba_t *pixels, int width, int height, int k, int max_iterations) {
    size_t num_objs = width * height;

    int *clusters = (int*)calloc(num_objs, sizeof(int));
    rgba_t **objs = (rgba_t**)calloc(num_objs, sizeof(rgba_t*));
    for (int i = 0; i < num_objs; i++) {
        objs[i] = &(pixels[i]);
        clusters[i] = i % k;
    }

    srand(time(0));
    rgba_t **centers = (rgba_t**)calloc(k, sizeof(rgba_t*));
    rgba_t *initial_centroids = (rgba_t*)calloc(k, sizeof(rgba_t));
    for (int i = 0; i < k; i++) {
        initial_centroids[i] = pixels[rand() % num_objs];
        centers[i] = &(initial_centroids[i]);
    }

    size_t clusters_sz = sizeof(int) * num_objs;
    int* clusters_last = (int*)malloc(clusters_sz);
    memset(clusters_last, 0, clusters_sz);

    int distance, current_distance;
    int cluster, current_cluster;
    int iterations = 0;

    while(memcmp(clusters_last, clusters, clusters_sz) != 0 && iterations++ < max_iterations) {
        memcpy(clusters_last, clusters, clusters_sz);

        // Update clusters
        for (int i = 0; i < num_objs; i++) {
            current_distance = euclidean_distance(objs[i], centers[0]);
            current_cluster = 0;

            for (int cluster = 1; cluster < k; cluster++) {
                distance = euclidean_distance(objs[i], centers[cluster]);
                if (distance < current_distance) {
                    current_distance = distance;
                    current_cluster = cluster;
                }
            }

            clusters[i] = current_cluster;
        }

        // Update the means
        for (int i = 0; i < k; i++) {
            pick_centroid(objs, clusters, num_objs, i, centers[i]);
        }
    }

    rgba_t* temp = (rgba_t*)calloc(num_objs, sizeof(rgba_t));
    memcpy(temp, pixels, num_objs * sizeof(rgba_t));
    int row = 0;
    int col = 0;

    // Changes horizontal bars to vertical cause it looks cool
    for (int cluster = 0; cluster < k; cluster++) {
        for(int p = 0; p < num_objs; p++) {
            if (clusters[p] == cluster) {
                pixels[row * width + col] = temp[p];
                if (++row >= height) {
                    row = 0;
                    col += 1;
                }
            }
        }
    }

    free(temp);
    free(clusters_last);
    free(clusters);
    free(objs);
    free(centers);
    free(initial_centroids);
    return (iterations < max_iterations) ? iterations : -1;
}
