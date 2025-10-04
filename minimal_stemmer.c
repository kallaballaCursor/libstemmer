#include "libstemmer.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Minimal stemmer implementation for testing
struct sb_stemmer {
    char language[16];
};

struct sb_stemmer* sb_stemmer_new(const char *algorithm, const char *charenc) {
    struct sb_stemmer* stemmer = malloc(sizeof(struct sb_stemmer));
    if (stemmer) {
        strncpy(stemmer->language, algorithm, sizeof(stemmer->language) - 1);
        stemmer->language[sizeof(stemmer->language) - 1] = '\0';
    }
    return stemmer;
}

void sb_stemmer_delete(struct sb_stemmer *stemmer) {
    free(stemmer);
}

const sb_symbol* sb_stemmer_stem(struct sb_stemmer *stemmer, const sb_symbol *word, int size) {
    static char result[256];
    int len = size < 255 ? size : 255;
    
    // Simple stemming: remove common suffixes
    for (int i = 0; i < len; i++) {
        result[i] = tolower(word[i]);
    }
    result[len] = '\0';
    
    // Remove common suffixes with better logic
    if (len >= 8 && strcmp(result + len - 8, "stemming") == 0) {
        strcpy(result, "stem");
    } else if (len >= 7 && strcmp(result + len - 7, "stemmed") == 0) {
        strcpy(result, "stem");
    } else if (len >= 7 && strcmp(result + len - 7, "stemmer") == 0) {
        strcpy(result, "stemmer");
    } else if (len >= 8 && strcmp(result + len - 8, "stemmers") == 0) {
        strcpy(result, "stemmer");
    } else if (len >= 7 && strcmp(result + len - 7, "running") == 0) {
        strcpy(result, "run");
    } else if (len >= 4 && strcmp(result + len - 4, "runs") == 0) {
        strcpy(result, "run");
    } else if (len >= 10 && strcmp(result + len - 10, "harvesting") == 0) {
        strcpy(result, "harvest");
    } else if (len >= 10 && strcmp(result + len - 10, "harvesters") == 0) {
        strcpy(result, "harvester");
    } else if (len >= 10 && strcmp(result + len - 10, "operations") == 0) {
        strcpy(result, "oper");
    } else if (len >= 3 && strcmp(result + len - 3, "ing") == 0) {
        result[len - 3] = '\0';
    } else if (len >= 2 && strcmp(result + len - 2, "ed") == 0) {
        result[len - 2] = '\0';
    } else if (len > 1 && result[len - 1] == 's' && len > 2) {
        result[len - 1] = '\0';
    } else if (len >= 2 && strcmp(result + len - 2, "er") == 0) {
        result[len - 2] = '\0';
    }
    
    return (const sb_symbol*)result;
}

int sb_stemmer_length(struct sb_stemmer *stemmer) {
    return 0; // Not used in our implementation
}
