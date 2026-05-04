#include "password_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

static const char* default_base_words[] = {
    "password", "admin", "user", "snapchat", "love", "hello", "welcome", "sunshine",
    "dragon", "master", "shadow", "princess", "football", "baseball", "iloveyou"
};

static const char* default_suffixes[] = {
    "123", "!", "1", "2024", "2025", "1234", "!@#", "000", "007", "69"
};

static const char* default_prefixes[] = {
    "!", "#", "admin", "super", "my"
};

static const char* default_special_chars[] = {
    "!", "@", "#", "$", "%", "&", "*", "?", "_", "-"
};

char* leet_speak(const char *text) {
    static char result[MAX_PASSWORD_LENGTH];
    int i = 0;
    memset(result, 0, sizeof(result));
    
    while (text[i] && i < MAX_PASSWORD_LENGTH - 1) {
        char c = text[i];
        switch (tolower(c)) {
            case 'a': result[i] = '4'; break;
            case 'e': result[i] = '3'; break;
            case 'i': result[i] = '1'; break;
            case 'o': result[i] = '0'; break;
            case 's': result[i] = '5'; break;
            case 't': result[i] = '7'; break;
            default: result[i] = c; break;
        }
        i++;
    }
    result[i] = '\0';
    return result;
}

AIPasswordGenerator* ai_generator_init(void) {
    AIPasswordGenerator *gen = (AIPasswordGenerator*)malloc(sizeof(AIPasswordGenerator));
    if (!gen) return NULL;
    
    gen->learning_rate = 0.1;
    gen->memory_capacity = MAX_MEMORY_SIZE;
    gen->memory_size = 0;
    gen->password_memory = (char**)malloc(sizeof(char*) * gen->memory_capacity);
    
    gen->patterns.base_words_count = sizeof(default_base_words) / sizeof(default_base_words[0]);
    for (int i = 0; i < gen->patterns.base_words_count; i++) {
        gen->patterns.base_words[i] = strdup(default_base_words[i]);
    }
    
    gen->patterns.suffixes_count = sizeof(default_suffixes) / sizeof(default_suffixes[0]);
    for (int i = 0; i < gen->patterns.suffixes_count; i++) {
        gen->patterns.common_suffixes[i] = strdup(default_suffixes[i]);
    }
    
    gen->patterns.prefixes_count = sizeof(default_prefixes) / sizeof(default_prefixes[0]);
    for (int i = 0; i < gen->patterns.prefixes_count; i++) {
        gen->patterns.common_prefixes[i] = strdup(default_prefixes[i]);
    }
    
    gen->patterns.special_chars_count = sizeof(default_special_chars) / sizeof(default_special_chars[0]);
    for (int i = 0; i < gen->patterns.special_chars_count; i++) {
        gen->patterns.special_chars[i] = strdup(default_special_chars[i]);
    }
    
    srand((unsigned int)time(NULL));
    return gen;
}

void ai_generator_free(AIPasswordGenerator *gen) {
    if (!gen) return;
    
    for (int i = 0; i < gen->patterns.base_words_count; i++) {
        free(gen->patterns.base_words[i]);
    }
    for (int i = 0; i < gen->patterns.suffixes_count; i++) {
        free(gen->patterns.common_suffixes[i]);
    }
    for (int i = 0; i < gen->patterns.prefixes_count; i++) {
        free(gen->patterns.common_prefixes[i]);
    }
    for (int i = 0; i < gen->patterns.special_chars_count; i++) {
        free(gen->patterns.special_chars[i]);
    }
    
    for (int i = 0; i < gen->memory_size; i++) {
        free(gen->password_memory[i]);
    }
    free(gen->password_memory);
    free(gen);
}

static int is_in_memory(AIPasswordGenerator *gen, const char *password) {
    for (int i = 0; i < gen->memory_size; i++) {
        if (strcmp(gen->password_memory[i], password) == 0) {
            return 1;
        }
    }
    return 0;
}

static void add_to_memory(AIPasswordGenerator *gen, const char *password) {
    if (gen->memory_size >= gen->memory_capacity) {
        free(gen->password_memory[0]);
        for (int i = 1; i < gen->memory_size; i++) {
            gen->password_memory[i - 1] = gen->password_memory[i];
        }
        gen->memory_size--;
    }
    gen->password_memory[gen->memory_size++] = strdup(password);
}

char* generate_context_aware_password(AIPasswordGenerator *gen, const char *username, int attempt_number) {
    static char password[MAX_PASSWORD_LENGTH];
    char username_lower[128];
    
    strncpy(username_lower, username, sizeof(username_lower) - 1);
    username_lower[sizeof(username_lower) - 1] = '\0';
    for (int i = 0; username_lower[i]; i++) {
        username_lower[i] = tolower(username_lower[i]);
    }
    
    int pattern_choice = rand() % 5;
    
    switch (pattern_choice) {
        case 0:
            if (strlen(username) <= 8) {
                snprintf(password, sizeof(password), "%s123", username_lower);
            } else {
                snprintf(password, sizeof(password), "%s123", username_lower);
            }
            break;
        case 1:
            snprintf(password, sizeof(password), "%s!", username_lower);
            break;
        case 2:
            snprintf(password, sizeof(password), "%s", leet_speak(username_lower));
            break;
        case 3: {
            int base_idx = rand() % gen->patterns.base_words_count;
            int suffix_idx = rand() % gen->patterns.suffixes_count;
            snprintf(password, sizeof(password), "%s%s",
                     gen->patterns.base_words[base_idx],
                     gen->patterns.common_suffixes[suffix_idx]);
            break;
        }
        case 4: {
            int base_idx = rand() % gen->patterns.base_words_count;
            int suffix_idx = rand() % gen->patterns.suffixes_count;
            char base[64];
            strncpy(base, gen->patterns.base_words[base_idx], sizeof(base) - 1);
            base[0] = toupper(base[0]);
            base[sizeof(base) - 1] = '\0';
            snprintf(password, sizeof(password), "%s%s", base,
                     gen->patterns.common_suffixes[suffix_idx]);
            break;
        }
    }
    
    if (is_in_memory(gen, password)) {
        int suffix = rand() % 1000;
        snprintf(password + strlen(password), sizeof(password) - strlen(password), "%d", suffix);
    }
    
    add_to_memory(gen, password);
    return password;
}

char* generate_advanced_ai_password(AIPasswordGenerator *gen, const char *username,
                                     char **previous_attempts, int prev_count) {
    if (prev_count == 0) {
        return generate_context_aware_password(gen, username, 0);
    }
    
    for (int i = 0; i < 10; i++) {
        char *new_pass = generate_context_aware_password(gen, username, prev_count + i);
        int found = 0;
        for (int j = 0; j < prev_count; j++) {
            if (strcmp(new_pass, previous_attempts[j]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            return new_pass;
        }
    }
    
    static char password[MAX_PASSWORD_LENGTH];
    snprintf(password, sizeof(password), "%s%d", previous_attempts[prev_count - 1], rand() % 999 + 1);
    return password;
}

NeuralWeights predict_next_password_type(char **failed_attempts, int fail_count) {
    NeuralWeights weights;
    weights.length_weights[0] = 0.8;
    weights.length_weights[1] = 0.9;
    weights.length_weights[2] = 0.7;
    weights.with_special_char = 0.6;
    weights.with_numbers = 0.95;
    weights.mixed_case = 0.5;
    
    if (fail_count == 0) {
        return weights;
    }
    
    int avg_length = 0;
    int has_special = 0;
    int has_numbers = 0;
    
    for (int i = 0; i < fail_count; i++) {
        int len = strlen(failed_attempts[i]);
        avg_length += len;
        for (int j = 0; j < len; j++) {
            if (strchr("!@#$%", failed_attempts[i][j])) has_special++;
            if (isdigit(failed_attempts[i][j])) has_numbers++;
        }
    }
    
    avg_length /= fail_count;
    
    if (avg_length < 7) {
        weights.length_weights[1] += 0.2;
    }
    if (has_special < fail_count * 0.3) {
        weights.with_special_char += 0.3;
    }
    if (has_numbers < fail_count * 0.8) {
        weights.with_numbers += 0.2;
    }
    
    return weights;
}

char* generate_neural_password(NeuralWeights *weights, AIPasswordGenerator *gen, const char *username) {
    static char password[MAX_PASSWORD_LENGTH];
    (void)username;
    
    int base_idx = rand() % gen->patterns.base_words_count;
    strncpy(password, gen->patterns.base_words[base_idx], sizeof(password) - 1);
    password[sizeof(password) - 1] = '\0';
    
    if ((double)rand() / RAND_MAX < weights->with_numbers) {
        const char *nums[] = {"123", "1234", "12345", "2024", "2025", "2026"};
        int num_idx = rand() % 6;
        strncat(password, nums[num_idx], sizeof(password) - strlen(password) - 1);
    }
    
    if ((double)rand() / RAND_MAX < weights->with_special_char) {
        int spec_idx = rand() % gen->patterns.special_chars_count;
        strncat(password, gen->patterns.special_chars[spec_idx],
                sizeof(password) - strlen(password) - 1);
    }
    
    if ((double)rand() / RAND_MAX < weights->mixed_case) {
        password[0] = toupper(password[0]);
    }
    
    return password;
}

double calculate_ai_delay(int attempt, double base_delay) {
    if (attempt < 50) {
        return base_delay + 0.5 + ((double)rand() / RAND_MAX) * 1.0;
    } else if (attempt < 200) {
        return base_delay + 0.2 + ((double)rand() / RAND_MAX) * 0.8;
    } else {
        return base_delay + 0.1 + ((double)rand() / RAND_MAX) * 0.4;
    }
}
